//! Converters from Kubernetes API objects to compact metadata.
//!
//! These functions strip extraneous fields and normalize key properties used by
//! the matcher and output layers.

use k8s_openapi::api::apps::v1::ReplicaSet;
use k8s_openapi::api::batch::v1::Job;
use k8s_openapi::api::core::v1::Pod;
use kube::ResourceExt;

use crate::types::{ContainerMeta, OwnerKind, OwnerMeta, OwnerRef, PodMeta};

/// Convert a Kubernetes `Pod` into a [`PodMeta`].
///
/// - Picks the first controller ownerReference as the pod owner (if any)
/// - Collects container runtime IDs, names, and images from status.container_statuses
/// - Computes a deterministic version string from container images
pub fn pod_to_meta(pod: Pod) -> PodMeta {
    let uid = pod.metadata.uid.clone().unwrap_or_default();
    let name = pod.name_any();
    let namespace = pod.namespace().unwrap_or_else(|| "default".to_string());

    let host_network = pod
        .spec
        .as_ref()
        .and_then(|s| s.host_network)
        .unwrap_or(false);
    let ip = pod
        .status
        .as_ref()
        .and_then(|s| s.pod_ip.clone())
        .unwrap_or_default();

    let containers = pod
        .status
        .as_ref()
        .and_then(|s| s.container_statuses.as_ref())
        .map(|v| {
            v.iter()
                .map(|cs| ContainerMeta {
                    id: cs.container_id.clone().unwrap_or_default(),
                    name: cs.name.clone(),
                    image: cs.image.clone(),
                })
                .collect::<Vec<_>>()
        })
        .unwrap_or_default();

    let images = pod
        .status
        .as_ref()
        .and_then(|s| s.container_statuses.as_ref())
        .map(|v| v.iter().map(|cs| cs.image.clone()).collect::<Vec<_>>())
        .unwrap_or_default();
    let version = build_version_string(images);

    let owner = pod
        .metadata
        .owner_references
        .unwrap_or_default()
        .into_iter()
        .find(|o| o.controller.unwrap_or(false))
        .map(|o| OwnerRef {
            kind: o.kind.parse().unwrap_or(OwnerKind::Other),
            uid: o.uid,
            name: o.name,
        });

    PodMeta {
        uid,
        ip,
        name,
        namespace,
        owner,
        host_network,
        version,
        containers,
    }
}

/// Convert a Kubernetes `ReplicaSet` into an [`OwnerMeta`].
///
/// If the ReplicaSet has a controller owner (e.g., Deployment), it is stored in
/// `controller` for escalation by the matcher.
pub fn rs_to_owner(rs: ReplicaSet) -> OwnerMeta {
    let uid = rs.metadata.uid.unwrap_or_default();
    let controller = rs
        .metadata
        .owner_references
        .unwrap_or_default()
        .into_iter()
        .find(|o| o.controller.unwrap_or(false))
        .map(|o| OwnerRef {
            kind: o.kind.parse().unwrap_or(OwnerKind::Other),
            uid: o.uid,
            name: o.name,
        });
    OwnerMeta { uid, controller }
}

/// Convert a Kubernetes `Job` into an [`OwnerMeta`].
///
/// If the Job has a controller owner (e.g., CronJob), it is stored in
/// `controller` for escalation by the matcher.
pub fn job_to_owner(job: Job) -> OwnerMeta {
    let uid = job.metadata.uid.unwrap_or_default();
    let controller = job
        .metadata
        .owner_references
        .unwrap_or_default()
        .into_iter()
        .find(|o| o.controller.unwrap_or(false))
        .map(|o| OwnerRef {
            kind: o.kind.parse().unwrap_or(OwnerKind::Other),
            uid: o.uid,
            name: o.name,
        });
    OwnerMeta { uid, controller }
}

/// Build a stable version string from a set of image references.
///
/// Images are quoted, sorted, and joined by commas to maintain consistency
/// across updates and ordering changes.
fn build_version_string(images: Vec<String>) -> String {
    let mut imgs = images
        .into_iter()
        .filter(|s| !s.is_empty())
        .map(|i| format!("'{}'", i))
        .collect::<Vec<_>>();
    imgs.sort();
    imgs.join(",")
}
