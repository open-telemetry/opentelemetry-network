//! Normalized events produced by the matcher and consumed by the encoder.
//!
//! Events mirror the messages the reducer expects. `PodNew` is accompanied by
//! one or more `PodContainer` events. `PodResync` indicates a new epoch.

use crate::types::{ContainerMeta, OwnerKind, OwnerRef, PodMeta};

#[derive(Clone, Debug)]
pub enum RenderEvent {
    /// Begin a new resync epoch with a monotonically increasing counter.
    PodResync { epoch: u64 },
    PodNew {
        /// Pod UID
        uid: String,
        /// Pod IPv4 string
        ip: String,
        /// Pod name
        pod_name: String,
        /// Namespace
        ns: String,
        /// Deterministic version from images
        version: String,
        /// Effective owner kind
        owner_kind: OwnerKind,
        /// Effective owner UID (empty for NoOwner)
        owner_uid: String,
        /// Effective owner name (pod name for NoOwner)
        owner_name: String,
        /// Host network flag
        is_host_network: bool,
    },
    PodContainer {
        /// Pod UID for which the container is reported
        uid: String,
        /// Container metadata
        container: ContainerMeta,
    },
    /// Delete a previously live pod
    PodDelete { uid: String },
}

impl RenderEvent {
    /// Expand a `PodNew` event into the full set of render events; includes
    /// `PodNew` itself and one `PodContainer` per container.
    pub fn from_pod_new(pod: &PodMeta, owner: Option<&OwnerRef>) -> Vec<RenderEvent> {
        let mut out = Vec::new();
        let (owner_kind, owner_uid, owner_name) = match owner {
            Some(o) => (o.kind, o.uid.clone(), o.name.clone()),
            None => (OwnerKind::NoOwner, String::new(), pod.name.clone()),
        };
        out.push(RenderEvent::PodNew {
            uid: pod.uid.clone(),
            ip: pod.ip.clone(),
            pod_name: pod.name.clone(),
            ns: pod.namespace.clone(),
            version: pod.version.clone(),
            owner_kind,
            owner_uid,
            owner_name,
            is_host_network: pod.host_network,
        });
        for c in &pod.containers {
            out.push(RenderEvent::PodContainer {
                uid: pod.uid.clone(),
                container: c.clone(),
            });
        }
        out
    }
}
