use std::path::PathBuf;
use std::time::Duration;

use anyhow::{bail, Context};
use k8s_openapi::api::apps::v1::{Deployment, DeploymentSpec, ReplicaSet, ReplicaSetSpec};
use k8s_openapi::api::batch::v1::{CronJob, CronJobSpec, Job, JobSpec};
use k8s_openapi::api::core::v1::{Container, Namespace, Pod, PodSpec, PodStatus, PodTemplateSpec};
use k8s_openapi::apimachinery::pkg::apis::meta::v1::{LabelSelector, ObjectMeta, OwnerReference};
use kube::{
    api::{Api, DeleteParams, ListParams, PostParams, ResourceExt},
    Client,
};
use serde_json::json;
use tokio::time::Instant;

use k8s_collector::collector::Collector;
use k8s_collector::config::Config;
use k8s_collector::output::RenderEvent;
use k8s_collector::types::OwnerKind;
use log::info;

fn init_test_logger() {
    if std::env::var_os("RUST_BACKTRACE").is_none() {
        std::env::set_var("RUST_BACKTRACE", "1");
    }
    if std::env::var_os("RUST_LIB_BACKTRACE").is_none() {
        std::env::set_var("RUST_LIB_BACKTRACE", "1");
    }
    let _ = env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .is_test(true)
        .try_init();
}

async fn ensure_namespace(client: &Client, name: &str) -> anyhow::Result<()> {
    let namespaces: Api<Namespace> = Api::all(client.clone());
    match namespaces.get(name).await {
        Ok(_) => Ok(()),
        Err(kube::Error::Api(ae)) if ae.code == 404 => {
            let ns: Namespace = serde_json::from_value(json!({
                "apiVersion": "v1",
                "kind": "Namespace",
                "metadata": { "name": name },
            }))?;
            namespaces.create(&PostParams::default(), &ns).await?;
            Ok(())
        }
        Err(e) => Err(e.into()),
    }
}

async fn delete_if_exists<T>(api: &Api<T>, name: &str) -> anyhow::Result<()>
where
    T: k8s_openapi::Metadata<Ty = ObjectMeta>
        + Clone
        + serde::de::DeserializeOwned
        + serde::Serialize
        + std::fmt::Debug,
{
    let dp = DeleteParams {
        grace_period_seconds: Some(0),
        ..DeleteParams::default()
    };
    match api.delete(name, &dp).await {
        Ok(_o) => Ok(()),
        Err(kube::Error::Api(ae)) if ae.code == 404 => Ok(()),
        Err(e) => Err(e.into()),
    }
}

async fn wait_for_pod_running_by_label(
    pods: &Api<Pod>,
    label_selector: &str,
    timeout: Duration,
) -> anyhow::Result<Pod> {
    let deadline = Instant::now() + timeout;
    loop {
        let lp = ListParams::default().labels(label_selector);
        let pod_list = pods.list(&lp).await?;
        for pod in pod_list {
            if let Some(status) = &pod.status {
                if status.phase.as_deref() == Some("Running") && containers_ready(status) {
                    return Ok(pod);
                }
            }
        }

        if Instant::now() >= deadline {
            bail!(
                "timed out waiting for a Running pod with label selector {}",
                label_selector
            );
        }
        tokio::time::sleep(Duration::from_secs(1)).await;
    }
}

fn containers_ready(status: &PodStatus) -> bool {
    status
        .container_statuses
        .as_ref()
        .map(|statuses| statuses.iter().all(|cs| cs.ready))
        .unwrap_or(false)
}

async fn create_sleep_deployment(
    client: &Client,
    namespace: &str,
    name: &str,
) -> anyhow::Result<Deployment> {
    let deployments: Api<Deployment> = Api::namespaced(client.clone(), namespace);

    let labels: std::collections::BTreeMap<String, String> =
        [("k8s-collector-e2e".to_string(), name.to_string())]
            .into_iter()
            .collect();
    let deployment = Deployment {
        metadata: ObjectMeta {
            name: Some(name.to_string()),
            namespace: Some(namespace.to_string()),
            ..ObjectMeta::default()
        },
        spec: Some(DeploymentSpec {
            replicas: Some(1),
            selector: LabelSelector {
                match_labels: Some(labels.clone()),
                ..LabelSelector::default()
            },
            template: PodTemplateSpec {
                metadata: Some(ObjectMeta {
                    labels: Some(labels),
                    ..ObjectMeta::default()
                }),
                spec: Some(PodSpec {
                    containers: vec![Container {
                        name: "main".into(),
                        image: Some("busybox".into()),
                        command: Some(vec!["/bin/sh".into(), "-c".into(), "sleep 3600".into()]),
                        image_pull_policy: Some("IfNotPresent".into()),
                        ..Container::default()
                    }],
                    ..PodSpec::default()
                }),
            },
            ..DeploymentSpec::default()
        }),
        status: None,
    };

    Ok(deployments
        .create(&PostParams::default(), &deployment)
        .await?)
}

async fn create_replicaset(
    client: &Client,
    namespace: &str,
    name: &str,
) -> anyhow::Result<ReplicaSet> {
    let replicasets: Api<ReplicaSet> = Api::namespaced(client.clone(), namespace);

    let labels: std::collections::BTreeMap<String, String> =
        [("k8s-collector-e2e".to_string(), name.to_string())]
            .into_iter()
            .collect();
    let rs = ReplicaSet {
        metadata: ObjectMeta {
            name: Some(name.to_string()),
            namespace: Some(namespace.to_string()),
            ..ObjectMeta::default()
        },
        spec: Some(ReplicaSetSpec {
            replicas: Some(1),
            selector: LabelSelector {
                match_labels: Some(labels.clone()),
                ..LabelSelector::default()
            },
            template: Some(PodTemplateSpec {
                metadata: Some(ObjectMeta {
                    labels: Some(labels),
                    ..ObjectMeta::default()
                }),
                spec: Some(PodSpec {
                    containers: vec![Container {
                        name: "main".into(),
                        image: Some("busybox".into()),
                        command: Some(vec!["/bin/sh".into(), "-c".into(), "sleep 3600".into()]),
                        image_pull_policy: Some("IfNotPresent".into()),
                        ..Container::default()
                    }],
                    ..PodSpec::default()
                }),
            }),
            ..ReplicaSetSpec::default()
        }),
        status: None,
    };

    Ok(replicasets.create(&PostParams::default(), &rs).await?)
}

async fn create_cronjob(client: &Client, namespace: &str, name: &str) -> anyhow::Result<CronJob> {
    let cronjobs: Api<CronJob> = Api::namespaced(client.clone(), namespace);
    let labels: std::collections::BTreeMap<String, String> =
        [("k8s-collector-e2e".to_string(), name.to_string())]
            .into_iter()
            .collect();

    let cj = CronJob {
        metadata: ObjectMeta {
            name: Some(name.to_string()),
            namespace: Some(namespace.to_string()),
            ..ObjectMeta::default()
        },
        spec: Some(CronJobSpec {
            schedule: "* * * * *".into(),
            job_template: k8s_openapi::api::batch::v1::JobTemplateSpec {
                metadata: Some(ObjectMeta {
                    labels: Some(labels.clone()),
                    ..ObjectMeta::default()
                }),
                spec: Some(JobSpec {
                    template: PodTemplateSpec {
                        metadata: Some(ObjectMeta {
                            labels: Some(labels),
                            ..ObjectMeta::default()
                        }),
                        spec: Some(PodSpec {
                            containers: vec![Container {
                                name: "main".into(),
                                image: Some("busybox".into()),
                                command: Some(vec![
                                    "/bin/sh".into(),
                                    "-c".into(),
                                    "sleep 3600".into(),
                                ]),
                                image_pull_policy: Some("IfNotPresent".into()),
                                ..Container::default()
                            }],
                            restart_policy: Some("Never".into()),
                            ..PodSpec::default()
                        }),
                    },
                    ..JobSpec::default()
                }),
            },
            ..CronJobSpec::default()
        }),
        status: None,
    };

    Ok(cronjobs.create(&PostParams::default(), &cj).await?)
}

async fn create_job_with_cron_owner(
    client: &Client,
    namespace: &str,
    cron: &CronJob,
    job_name: &str,
) -> anyhow::Result<Job> {
    let jobs: Api<Job> = Api::namespaced(client.clone(), namespace);
    let cron_meta = cron.metadata.clone();
    let cron_name = cron_meta
        .name
        .clone()
        .context("CronJob missing metadata.name")?;
    let cron_uid = cron_meta
        .uid
        .clone()
        .context("CronJob missing metadata.uid")?;

    let labels: std::collections::BTreeMap<String, String> =
        [("k8s-collector-e2e".to_string(), job_name.to_string())]
            .into_iter()
            .collect();

    let job = Job {
        metadata: ObjectMeta {
            name: Some(job_name.to_string()),
            namespace: Some(namespace.to_string()),
            owner_references: Some(vec![OwnerReference {
                api_version: "batch/v1".into(),
                kind: "CronJob".into(),
                name: cron_name,
                uid: cron_uid,
                controller: Some(true),
                ..OwnerReference::default()
            }]),
            ..ObjectMeta::default()
        },
        spec: Some(JobSpec {
            template: PodTemplateSpec {
                metadata: Some(ObjectMeta {
                    labels: Some(labels),
                    ..ObjectMeta::default()
                }),
                spec: Some(PodSpec {
                    containers: vec![Container {
                        name: "main".into(),
                        image: Some("busybox".into()),
                        command: Some(vec!["/bin/sh".into(), "-c".into(), "sleep 3600".into()]),
                        image_pull_policy: Some("IfNotPresent".into()),
                        ..Container::default()
                    }],
                    restart_policy: Some("Never".into()),
                    ..PodSpec::default()
                }),
            },
            ..JobSpec::default()
        }),
        status: None,
    };

    Ok(jobs.create(&PostParams::default(), &job).await?)
}

struct ObservedPod {
    uid: String,
    name: String,
    expect_owner_kind: OwnerKind,
    expect_owner_name: String,
    saw_pod_new: bool,
    saw_container: bool,
}

fn update_observed_from_event(obs: &mut ObservedPod, ev: &RenderEvent) {
    match ev {
        RenderEvent::PodNew {
            uid,
            owner_kind,
            owner_name,
            ..
        } if uid == &obs.uid => {
            obs.saw_pod_new = true;
            assert_eq!(
                *owner_kind, obs.expect_owner_kind,
                "unexpected owner kind for pod {}",
                uid
            );
            assert_eq!(
                owner_name, &obs.expect_owner_name,
                "unexpected owner name for pod {}",
                uid
            );
            info!(
                "k8s-collector e2e: verified owner for pod uid={} name={} -> kind={:?} name={}",
                uid, obs.name, owner_kind, owner_name
            );
        }
        RenderEvent::PodContainer { uid, .. } if uid == &obs.uid => {
            obs.saw_container = true;
        }
        _ => {}
    }
}

fn observed_done(obs: &ObservedPod) -> bool {
    obs.saw_pod_new && obs.saw_container
}

async fn pump_until<F>(collector: &mut Collector, timeout: Duration, mut f: F) -> anyhow::Result<()>
where
    F: FnMut(&RenderEvent) -> bool,
{
    let deadline = Instant::now() + timeout;
    loop {
        if Instant::now() >= deadline {
            info!(
                "k8s-collector e2e: pump_until outer deadline {:?} elapsed; collector snapshot:\n{}",
                timeout,
                collector.debug_snapshot()
            );
            bail!("timed out waiting for expected collector events");
        }

        let remaining = deadline
            .checked_duration_since(Instant::now())
            .unwrap_or_else(|| Duration::from_secs(1));
        let batch = match tokio::time::timeout(remaining, collector.next_messages()).await {
            Ok(Ok(batch)) => batch,
            Ok(Err(e)) => {
                info!(
                    "k8s-collector e2e: pump_until collector.next_messages() returned error: {:?}; snapshot:\n{}",
                    e,
                    collector.debug_snapshot()
                );
                return Err(e.into());
            }
            Err(elapsed) => {
                info!(
                    "k8s-collector e2e: pump_until tokio timeout after {:?} while waiting for collector.next_messages(); snapshot:\n{}",
                    remaining,
                    collector.debug_snapshot()
                );
                bail!(
                    "tokio timeout waiting for collector.next_messages(): {}",
                    elapsed
                );
            }
        };
        if batch.is_empty() {
            continue;
        }

        for ev in &batch {
            info!("k8s-collector e2e: pump_until observed event: {:?}", ev);
            if f(ev) {
                return Ok(());
            }
        }
    }
}

#[tokio::test]
#[cfg(target_os = "linux")]
#[ignore]
async fn test_k8s_collector_end_to_end_e2e() -> anyhow::Result<()> {
    init_test_logger();

    // Use a per-run suffix to ensure that resource names are unique across test
    // invocations, so we never accidentally match pods from a previous run.
    let run_suffix = {
        use std::time::{SystemTime, UNIX_EPOCH};
        let nanos = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_nanos();
        let hex = format!("{:x}", nanos);
        // Take the last 6 hex chars to keep names short but unique-enough.
        hex.chars()
            .rev()
            .take(6)
            .collect::<String>()
            .chars()
            .rev()
            .collect::<String>()
    };
    info!("k8s-collector e2e: using run suffix {}", run_suffix);

    // Skip the test if no kubeconfig is available; this environment
    // does not guarantee a Kubernetes cluster.
    let kubeconfig_env = std::env::var_os("KUBECONFIG");
    let default_kubeconfig_missing = std::env::var_os("HOME")
        .map(|home| {
            let mut p = PathBuf::from(home);
            p.push(".kube/config");
            !p.exists()
        })
        .unwrap_or(true);
    if kubeconfig_env.is_none() && default_kubeconfig_missing {
        eprintln!(
            "KUBECONFIG not set and default kubeconfig missing; skipping k8s-collector e2e test"
        );
        return Ok(());
    }

    let client = Client::try_default().await?;
    let namespace = "k8s-collector-e2e";
    info!(
        "k8s-collector e2e: starting test in namespace {}",
        namespace
    );

    ensure_namespace(&client, namespace).await?;
    info!("k8s-collector e2e: namespace {} is ready", namespace);

    let pods: Api<Pod> = Api::namespaced(client.clone(), namespace);
    let deployments: Api<Deployment> = Api::namespaced(client.clone(), namespace);
    let jobs: Api<Job> = Api::namespaced(client.clone(), namespace);
    let cronjobs: Api<CronJob> = Api::namespaced(client.clone(), namespace);
    let replicasets: Api<ReplicaSet> = Api::namespaced(client.clone(), namespace);

    // Per-run resource names with a unique suffix to avoid collisions
    // between test invocations.
    let pre_deploy_name = format!("kc-e2e-pre-deploy-{}", run_suffix);
    let pre_cron_name = format!("kc-e2e-pre-cron-{}", run_suffix);
    let pre_job_name = format!("kc-e2e-pre-job-{}", run_suffix);
    let pre_rs_name = format!("kc-e2e-pre-rs-{}", run_suffix);
    let new_deploy_name = format!("kc-e2e-new-deploy-{}", run_suffix);
    let new_cron_name = format!("kc-e2e-new-cron-{}", run_suffix);
    let new_job_name = format!("kc-e2e-new-job-{}", run_suffix);

    // Best-effort cleanup from previous runs with the same naming pattern.
    let _ = delete_if_exists(&deployments, &pre_deploy_name).await;
    let _ = delete_if_exists(&deployments, &new_deploy_name).await;
    let _ = delete_if_exists(&cronjobs, &pre_cron_name).await;
    let _ = delete_if_exists(&cronjobs, &new_cron_name).await;
    let _ = delete_if_exists(&jobs, &pre_job_name).await;
    let _ = delete_if_exists(&jobs, &new_job_name).await;
    let _ = delete_if_exists(&replicasets, &pre_rs_name).await;
    info!("k8s-collector e2e: completed pre-test cleanup of prior resources");

    // Pre-existing Deployment and CronJob+Job before the collector starts.
    let pre_deploy = create_sleep_deployment(&client, namespace, &pre_deploy_name).await?;
    let pre_cron = create_cronjob(&client, namespace, &pre_cron_name).await?;
    let _pre_job = create_job_with_cron_owner(&client, namespace, &pre_cron, &pre_job_name).await?;
    let pre_rs = create_replicaset(&client, namespace, &pre_rs_name).await?;

    // Wait for their pods to be running and capture UIDs.
    let pre_deploy_pod = wait_for_pod_running_by_label(
        &pods,
        &format!("k8s-collector-e2e={}", pre_deploy_name),
        Duration::from_secs(120),
    )
    .await?;
    let pre_deploy_uid = pre_deploy_pod
        .metadata
        .uid
        .clone()
        .context("pre-deploy pod missing uid")?;
    let pre_deploy_pod_name = pre_deploy_pod.name_any();

    let pre_cron_pod = wait_for_pod_running_by_label(
        &pods,
        &format!("k8s-collector-e2e={}", pre_job_name),
        Duration::from_secs(120),
    )
    .await?;
    let pre_cron_pod_uid = pre_cron_pod
        .metadata
        .uid
        .clone()
        .context("pre-cron pod missing uid")?;
    let pre_cron_pod_name = pre_cron_pod.name_any();
    let pre_rs_pod = wait_for_pod_running_by_label(
        &pods,
        &format!("k8s-collector-e2e={}", pre_rs_name),
        Duration::from_secs(120),
    )
    .await?;
    let pre_rs_uid = pre_rs_pod
        .metadata
        .uid
        .clone()
        .context("pre-rs pod missing uid")?;
    let pre_rs_pod_name = pre_rs_pod.name_any();
    info!(
        "k8s-collector e2e: pre-existing pods ready: deploy name={} uid={}, cronpod name={} uid={}, rs name={} uid={}",
        pre_deploy_pod_name, pre_deploy_uid, pre_cron_pod_name, pre_cron_pod_uid, pre_rs_pod_name, pre_rs_uid
    );

    let cfg = Config::default();
    // delete_ttl/delete_capacity already have defaults; we use them as-is.
    let mut collector = Collector::with_client(cfg, client.clone());
    info!("k8s-collector e2e: collector pipeline initialized");

    let mut pre_deploy_obs = ObservedPod {
        uid: pre_deploy_uid.clone(),
        name: pre_deploy_pod_name.clone(),
        expect_owner_kind: OwnerKind::Deployment,
        expect_owner_name: pre_deploy.name_any(),
        saw_pod_new: false,
        saw_container: false,
    };
    let mut pre_cron_obs = ObservedPod {
        uid: pre_cron_pod_uid.clone(),
        name: pre_cron_pod_name.clone(),
        expect_owner_kind: OwnerKind::CronJob,
        expect_owner_name: pre_cron.name_any(),
        saw_pod_new: false,
        saw_container: false,
    };
    let mut pre_rs_obs = ObservedPod {
        uid: pre_rs_uid.clone(),
        name: pre_rs_pod_name.clone(),
        expect_owner_kind: OwnerKind::ReplicaSet,
        expect_owner_name: pre_rs.name_any(),
        saw_pod_new: false,
        saw_container: false,
    };

    let mut last_epoch = 0u64;

    // Pump until we have seen a PodResync and both pre-existing pods fully emitted.
    pump_until(
        &mut collector,
        Duration::from_secs(60),
        |ev: &RenderEvent| {
            if let RenderEvent::PodResync { epoch } = ev {
                last_epoch = *epoch;
            }
            update_observed_from_event(&mut pre_deploy_obs, ev);
            update_observed_from_event(&mut pre_cron_obs, ev);
            update_observed_from_event(&mut pre_rs_obs, ev);
            last_epoch > 0
                && observed_done(&pre_deploy_obs)
                && observed_done(&pre_cron_obs)
                && observed_done(&pre_rs_obs)
        },
    )
    .await?;

    assert!(last_epoch > 0, "expected at least one resync epoch");
    info!(
        "k8s-collector e2e: observed initial resync epoch {} and full emission for pre-existing pods",
        last_epoch
    );

    // Create new Deployment and CronJob+Job while collector is running.
    info!("k8s-collector e2e: creating new Deployment and CronJob/Job resources");
    let new_deploy = create_sleep_deployment(&client, namespace, &new_deploy_name).await?;
    let new_cron = create_cronjob(&client, namespace, &new_cron_name).await?;
    let _new_job = create_job_with_cron_owner(&client, namespace, &new_cron, &new_job_name).await?;

    let new_deploy_pod = wait_for_pod_running_by_label(
        &pods,
        &format!("k8s-collector-e2e={}", new_deploy_name),
        Duration::from_secs(120),
    )
    .await?;
    let new_deploy_uid = new_deploy_pod
        .metadata
        .uid
        .clone()
        .context("new-deploy pod missing uid")?;
    let new_deploy_pod_name = new_deploy_pod.name_any();

    let new_cron_pod = wait_for_pod_running_by_label(
        &pods,
        &format!("k8s-collector-e2e={}", new_job_name),
        Duration::from_secs(120),
    )
    .await?;
    let new_cron_pod_uid = new_cron_pod
        .metadata
        .uid
        .clone()
        .context("new-cron pod missing uid")?;
    let new_cron_pod_name = new_cron_pod.name_any();
    info!(
        "k8s-collector e2e: new pods ready: deploy name={} uid={}, cronpod name={} uid={}",
        new_deploy_pod_name, new_deploy_uid, new_cron_pod_name, new_cron_pod_uid
    );

    let mut new_deploy_obs = ObservedPod {
        uid: new_deploy_uid.clone(),
        name: new_deploy_pod_name.clone(),
        expect_owner_kind: OwnerKind::Deployment,
        expect_owner_name: new_deploy.name_any(),
        saw_pod_new: false,
        saw_container: false,
    };
    let mut new_cron_obs = ObservedPod {
        uid: new_cron_pod_uid.clone(),
        name: new_cron_pod_name.clone(),
        expect_owner_kind: OwnerKind::CronJob,
        expect_owner_name: new_cron.name_any(),
        saw_pod_new: false,
        saw_container: false,
    };

    pump_until(
        &mut collector,
        Duration::from_secs(60),
        |ev: &RenderEvent| {
            update_observed_from_event(&mut new_deploy_obs, ev);
            update_observed_from_event(&mut new_cron_obs, ev);
            observed_done(&new_deploy_obs) && observed_done(&new_cron_obs)
        },
    )
    .await?;
    info!("k8s-collector e2e: observed PodNew+PodContainer for new deploy and cronjob pods");

    // Force an explicit resync epoch and verify it includes all pods.
    info!(
        "k8s-collector e2e: forcing explicit resync epoch after last_epoch={}",
        last_epoch
    );
    let epoch_events = collector.start_new_epoch();
    let mut resync_epoch = None;
    let mut seen_uids = Vec::new();
    for ev in &epoch_events {
        match ev {
            RenderEvent::PodResync { epoch } => {
                resync_epoch = Some(*epoch);
            }
            RenderEvent::PodNew { uid, .. } => {
                seen_uids.push(uid.clone());
            }
            _ => {}
        }
    }

    let resync_epoch = resync_epoch.context("expected PodResync event from start_new_epoch")?;
    assert!(
        resync_epoch > last_epoch,
        "expected resync epoch to be greater than previous epoch"
    );
    info!(
        "k8s-collector e2e: resync epoch {} emitted PodNew snapshot for {} pods",
        resync_epoch,
        seen_uids.len()
    );

    // For each observed pod, if it still exists in the cluster under the same
    // name+UID, require that it appears in the resync snapshot. Pods that have
    // been deleted or replaced are skipped. When a UID is missing, emit
    // detailed debugging about the snapshot contents.
    for obs in [
        &pre_deploy_obs,
        &pre_cron_obs,
        &pre_rs_obs,
        &new_deploy_obs,
        &new_cron_obs,
    ] {
        match pods.get(&obs.name).await {
            Ok(pod) => {
                let current_uid = pod.metadata.uid.as_deref().unwrap_or_default();
                if current_uid == obs.uid {
                    if !seen_uids.contains(&obs.uid) {
                        info!(
                            "k8s-collector e2e: missing expected uid {} in resync snapshot; dumping PodNew events",
                            obs.uid
                        );
                        for ev in &epoch_events {
                            if let RenderEvent::PodNew {
                                uid, pod_name, ns, ..
                            } = ev
                            {
                                info!(
                                    "k8s-collector e2e: snapshot PodNew pod_name={} ns={} uid={}",
                                    pod_name, ns, uid
                                );
                            }
                        }
                        info!("k8s-collector e2e: snapshot seen_uids={:?}", seen_uids);
                        info!(
                            "k8s-collector e2e: collector matcher state snapshot:\n{}",
                            collector.debug_snapshot()
                        );
                        panic!("expected uid {} in resync snapshot", obs.uid);
                    }
                    info!(
                        "k8s-collector e2e: snapshot contains live pod name={} uid={}",
                        obs.name, obs.uid
                    );
                } else {
                    info!(
                        "k8s-collector e2e: pod name={} originally uid={} now uid={}; treating as replaced",
                        obs.name, obs.uid, current_uid
                    );
                }
            }
            Err(kube::Error::Api(ae)) if ae.code == 404 => {
                info!(
                    "k8s-collector e2e: pod name={} uid={} no longer exists; skipping snapshot assertion",
                    obs.name, obs.uid
                );
            }
            Err(e) => return Err(e.into()),
        }
    }

    // Best-effort cleanup.
    let _ = delete_if_exists(&deployments, &pre_deploy_name).await;
    let _ = delete_if_exists(&deployments, &new_deploy_name).await;
    let _ = delete_if_exists(&cronjobs, &pre_cron_name).await;
    let _ = delete_if_exists(&cronjobs, &new_cron_name).await;
    let _ = delete_if_exists(&jobs, &pre_job_name).await;
    let _ = delete_if_exists(&jobs, &new_job_name).await;

    Ok(())
}
