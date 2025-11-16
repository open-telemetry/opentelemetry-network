//! Orchestrates the layered pipeline: kube watchers → tombstone adapters →
//! matcher → encoder → writer.
//!
//! The function [`run`] connects to the Kubernetes API, wires the streams, and
//! drives the end-to-end loop including reducer connection management.

use std::pin::Pin;

use futures_util::{stream, Stream, StreamExt};
use log::{error, warn};
use tokio::time::{sleep, Duration};

use crate::config::Config;
use crate::convert_to_meta::{job_to_owner, pod_to_meta, rs_to_owner};
use crate::encode;
use crate::matcher::Matcher;
use crate::output::RenderEvent;
use crate::tombstone_adapter::TombstoneAdapter;
use crate::types::{OwnerMeta, PodMeta};
use crate::writer::Writer;
use kube::runtime::watcher::Event;

use kube::{
    runtime::{
        reflector::{self},
        watcher::{self, Event as KEvent},
    },
    Api, Client,
};

use k8s_openapi::api::apps::v1::ReplicaSet;
use k8s_openapi::api::batch::v1::Job;
use k8s_openapi::api::core::v1::Pod;

type PodStream =
    Pin<Box<dyn Stream<Item = Result<Vec<Event<PodMeta>>, kube::runtime::watcher::Error>> + Send>>;

type OwnerStream = Pin<
    Box<dyn Stream<Item = Result<Vec<Event<OwnerMeta>>, kube::runtime::watcher::Error>> + Send>,
>;

/// High-level collector pipeline used by both the binary and tests.
///
/// Owns kube watcher streams, Stores, tombstone adapters, and the Matcher,
/// exposing a `next_messages` API that yields batches of `RenderEvent`s.
pub struct Collector {
    pod_stream: PodStream,
    owner_stream: OwnerStream,

    owners_writer: reflector::store::Writer<OwnerMeta>,
    pods_writer: reflector::store::Writer<PodMeta>,

    matcher: Matcher,
    owner_tomb: TombstoneAdapter<OwnerMeta>,
    pod_tomb: TombstoneAdapter<PodMeta>,
}

impl Collector {
    /// Construct a new collector pipeline using the default Kubernetes client.
    pub async fn new(cfg: Config) -> Result<Self, crate::Error> {
        let client = Client::try_default()
            .await
            .map_err(|_| crate::Error::Stopped)?;
        Ok(Self::with_client(cfg, client))
    }

    /// Construct a new collector pipeline from an existing Kubernetes client.
    pub fn with_client(cfg: Config, client: Client) -> Self {
        let pods_api: Api<Pod> = Api::all(client.clone());
        let rs_api: Api<ReplicaSet> = Api::all(client.clone());
        let job_api: Api<Job> = Api::all(client);

        let wc = watcher::Config::default();

        // Watcher streams mapped to our Event<T> protocol
        let pod_stream =
            watcher::watcher(pods_api, wc.clone()).map(|e| map_kube_event(e, pod_to_meta));
        let rs_stream =
            watcher::watcher(rs_api, wc.clone()).map(|e| map_kube_event(e, rs_to_owner));
        let job_stream = watcher::watcher(job_api, wc).map(|e| map_kube_event(e, job_to_owner));

        let owner_stream = stream::select(rs_stream, job_stream);

        let (owners_store, owners_writer) = reflector::store::store::<OwnerMeta>();
        let (pods_store, pods_writer) = reflector::store::store::<PodMeta>();
        let matcher = Matcher::new(owners_store, pods_store);

        let owner_tomb = TombstoneAdapter::new(cfg.delete_ttl, cfg.delete_capacity);
        let pod_tomb = TombstoneAdapter::new(cfg.delete_ttl, cfg.delete_capacity);

        Self {
            pod_stream: Box::pin(pod_stream),
            owner_stream: Box::pin(owner_stream),
            owners_writer,
            pods_writer,
            matcher,
            owner_tomb,
            pod_tomb,
        }
    }

    /// Fetch the next batch of render events from either the pod or owner
    /// watcher stream.
    ///
    /// Returns:
    /// - `Ok(events)` with zero or more `RenderEvent`s; the caller can skip
    ///   empty batches and continue calling to wait for more data.
    /// - `Err` on watcher failure.
    pub async fn next_messages(&mut self) -> Result<Vec<RenderEvent>, crate::Error> {
        loop {
            let mut out = Vec::new();
            tokio::select! {
                ev = self.pod_stream.next() => {
                    match ev {
                        Some(Ok(events)) => {
                            for e in events {
                                for fwd in self.pod_tomb.handle(e) {
                                    self.pods_writer.apply_watcher_event(&fwd);
                                    out.extend(self.matcher.handle_pod(fwd));
                                }
                            }
                        }
                        Some(Err(err)) => {
                            warn!("pod watcher error: {err:?}; will retry on next tick");
                            continue;
                        }
                        None => {
                            error!("pod watcher stream terminated unexpectedly");
                            return Err(crate::Error::Stopped);
                        }
                    }
                }
                ev = self.owner_stream.next() => {
                    match ev {
                        Some(Ok(events)) => {
                            for e in events {
                                for fwd in self.owner_tomb.handle(e) {
                                    self.owners_writer.apply_watcher_event(&fwd);
                                    out.extend(self.matcher.handle_owner(fwd));
                                }
                            }
                        }
                        Some(Err(err)) => {
                            warn!("owner watcher error: {err:?}; will retry on next tick");
                            continue;
                        }
                        None => {
                            error!("owner watcher stream terminated unexpectedly");
                            return Err(crate::Error::Stopped);
                        }
                    }
                }
            }

            if !out.is_empty() {
                return Ok(out);
            }
        }
    }

    /// Start a new resync epoch using the current store snapshots.
    pub fn start_new_epoch(&mut self) -> Vec<RenderEvent> {
        self.matcher.start_new_epoch()
    }

    /// Produce a human-readable snapshot of the internal matcher/stores.
    ///
    /// This is primarily intended for tests and debugging to inspect the
    /// Stores and live pod bookkeeping when assertions fail.
    pub fn debug_snapshot(&self) -> String {
        self.matcher.debug_snapshot()
    }
}

/// Run the collector to completion.
///
/// This function:
/// - Starts kube watchers for Pods, ReplicaSets, and Jobs
/// - Applies tombstone adapters to both owner and pod streams
/// - Matches and encodes events, writing them to the reducer
/// - Reconnects aggressively on socket errors and starts a new epoch per connect
pub async fn run(cfg: Config) -> Result<(), crate::Error> {
    use tokio::net::TcpStream;

    let mut pipeline = Collector::new(cfg.clone()).await?;

    let addr = std::net::SocketAddr::new(
        cfg.intake_host
            .parse()
            .unwrap_or(std::net::Ipv4Addr::LOCALHOST.into()),
        cfg.intake_port,
    );

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
        for ev in pipeline.start_new_epoch() {
            let buf = encode::encode(&ev, timestamp());
            if let Err(_e) = writer.send(&buf).await {
                // Connection failed while sending epoch; restart outer loop
                continue;
            }
        }
        let _ = writer.flush().await;

        // Inner loop: forward events until connection drops or the pipeline fails
        'connection: loop {
            match pipeline.next_messages().await {
                Ok(events) => {
                    for ev in events {
                        let buf = encode::encode(&ev, timestamp());
                        if let Err(_e) = writer.send(&buf).await {
                            // Connection dropped; break to reconnect.
                            break 'connection;
                        }
                    }
                    let _ = writer.flush().await;
                }
                Err(e) => {
                    // Fatal pipeline error (e.g., watcher stream terminated): propagate up.
                    error!("collector pipeline failed: {e}");
                    return Err(e);
                }
            }
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
