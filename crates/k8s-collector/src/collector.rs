//! Orchestrates the layered pipeline: kube watchers → tombstone adapters →
//! matcher → encoder → writer.
//!
//! The function [`run`] connects to the Kubernetes API, wires the streams, and
//! drives the end-to-end loop including reducer connection management.

use futures_util::{pin_mut, stream, StreamExt};
use tokio::time::{sleep, Duration};

use crate::config::Config;
use crate::convert_to_meta::{job_to_owner, pod_to_meta, rs_to_owner};
use crate::encode;
use crate::matcher::Matcher;
use crate::types::{OwnerMeta, PodMeta};
use kube::runtime::watcher::Event;
// use crate::output::RenderEvent;
use crate::tombstone_adapter::TombstoneAdapter;
use crate::writer::Writer;

use kube::{
    runtime::watcher::{self, Event as KEvent},
    Api, Client,
};

use k8s_openapi::api::apps::v1::ReplicaSet;
use k8s_openapi::api::batch::v1::Job;
use k8s_openapi::api::core::v1::Pod;

/// Run the collector to completion.
///
/// This function:
/// - Starts kube watchers for Pods, ReplicaSets, and Jobs
/// - Applies tombstone adapters to both owner and pod streams
/// - Matches and encodes events, writing them to the reducer
/// - Reconnects aggressively on socket errors and starts a new epoch per connect
pub async fn run(cfg: Config) -> Result<(), crate::Error> {
    let client = Client::try_default()
        .await
        .map_err(|_| crate::Error::Stopped)?;

    let pods_api: Api<Pod> = Api::all(client.clone());
    let rs_api: Api<ReplicaSet> = Api::all(client.clone());
    let job_api: Api<Job> = Api::all(client.clone());

    let wc = watcher::Config::default();

    // Watcher streams mapped to our Event<T> protocol
    let pod_stream = watcher::watcher(pods_api, wc.clone()).map(|e| map_kube_event(e, pod_to_meta));
    let rs_stream = watcher::watcher(rs_api, wc.clone()).map(|e| map_kube_event(e, rs_to_owner));
    let job_stream = watcher::watcher(job_api, wc).map(|e| map_kube_event(e, job_to_owner));

    let owner_stream = stream::select(rs_stream, job_stream);
    pin_mut!(pod_stream);
    pin_mut!(owner_stream);

    let addr = std::net::SocketAddr::new(
        cfg.intake_host
            .parse()
            .unwrap_or(std::net::Ipv4Addr::LOCALHOST.into()),
        cfg.intake_port,
    );
    use tokio::net::TcpStream;

    let (owner_store, mut owner_writer) = kube::runtime::reflector::store::store::<OwnerMeta>();
    let (pod_store, mut pod_writer) = kube::runtime::reflector::store::store::<PodMeta>();
    let mut matcher = Matcher::new(owner_store, pod_store);
    let mut owner_tomb = TombstoneAdapter::new(cfg.delete_ttl, cfg.delete_capacity);
    let mut pod_tomb = TombstoneAdapter::new(cfg.delete_ttl, cfg.delete_capacity);

    loop {
        // Connect
        let stream = match TcpStream::connect(addr).await {
            Ok(s) => s,
            Err(_) => {
                sleep(Duration::from_secs(1)).await;
                continue;
            }
        };
        let mut writer = Writer::new(stream);

        // New epoch on connect
        for ev in matcher.start_new_epoch() {
            let buf = encode::encode(&ev, timestamp());
            if let Err(_e) = writer.send(&buf).await {
                break;
            }
        }
        let _ = writer.flush().await;

        // Inner loop: forward events until connection drops
        let mut connection_ok = true;
        while connection_ok {
            tokio::select! {
                Some(ev) = pod_stream.next() => {
                    if let Ok(events) = ev {
                        for e in events {
                            for fwd in pod_tomb.handle(e) {
                                pod_writer.apply_watcher_event(&fwd);
                                for out in matcher.handle_pod(fwd) {
                                    let buf = encode::encode(&out, timestamp());
                                    if let Err(_e) = writer.send(&buf).await { connection_ok = false; break; }
                                }
                            }
                        }
                    }
                }
                Some(ev) = owner_stream.next() => {
                    if let Ok(events) = ev {
                        for e in events {
                            for fwd in owner_tomb.handle(e) {
                                owner_writer.apply_watcher_event(&fwd);
                                for out in matcher.handle_owner(fwd) {
                                    let buf = encode::encode(&out, timestamp());
                                    if let Err(_e) = writer.send(&buf).await { connection_ok = false; break; }
                                }
                            }
                        }
                    }
                }
                else => { connection_ok = false; }
            }
            let _ = writer.flush().await;
        }

        // Drop connection, wait and reconnect
        sleep(Duration::from_secs(1)).await;
    }
}

/// Map a kube watcher event to one or more internal events by applying a
/// converter function.
fn map_kube_event<K, T>(
    ev: Result<KEvent<K>, kube::runtime::watcher::Error>,
    f: fn(K) -> T,
) -> Result<Vec<Event<T>>, kube::runtime::watcher::Error> {
    Ok(match ev? {
        KEvent::Apply(obj) => vec![Event::Apply(f(obj))],
        KEvent::Delete(obj) => vec![Event::Delete(f(obj))],
        KEvent::Init => vec![Event::Init],
        KEvent::InitApply(obj) => vec![Event::InitApply(f(obj))],
        KEvent::InitDone => vec![Event::InitDone],
    })
}

/// Current UNIX time in nanoseconds.
fn timestamp() -> u64 {
    use std::time::{SystemTime, UNIX_EPOCH};
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_nanos() as u64)
        .unwrap_or(0)
}
