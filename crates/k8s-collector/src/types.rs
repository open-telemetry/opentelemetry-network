//! Compact metadata types used by the collector.
//!
//! These types are derived from Kubernetes objects and used internally by the
//! matcher and output layers.

use std::fmt;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum OwnerKind {
    /// apps/v1 ReplicaSet
    ReplicaSet,
    /// apps/v1 Deployment
    Deployment,
    /// batch/v1 Job
    Job,
    /// batch/v1 CronJob
    CronJob,
    /// Explicitly indicates no owner
    NoOwner,
    /// Any other kind; treated as opaque
    Other,
}

impl OwnerKind {
    pub fn is_rs_or_job(self) -> bool {
        matches!(self, OwnerKind::ReplicaSet | OwnerKind::Job)
    }
}

impl std::str::FromStr for OwnerKind {
    type Err = ();

    /// Parse a human-readable Kubernetes kind string into an [`OwnerKind`].
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Ok(match s {
            "ReplicaSet" => OwnerKind::ReplicaSet,
            "Deployment" => OwnerKind::Deployment,
            "Job" => OwnerKind::Job,
            "CronJob" => OwnerKind::CronJob,
            "NoOwner" => OwnerKind::NoOwner,
            _ => OwnerKind::Other,
        })
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct OwnerRef {
    /// Kubernetes owner kind
    pub kind: OwnerKind,
    /// Owner UID
    pub uid: String,
    /// Owner name
    pub name: String,
}

impl fmt::Display for OwnerRef {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{}:{}:{}",
            match self.kind {
                OwnerKind::ReplicaSet => "ReplicaSet",
                OwnerKind::Deployment => "Deployment",
                OwnerKind::Job => "Job",
                OwnerKind::CronJob => "CronJob",
                OwnerKind::NoOwner => "NoOwner",
                OwnerKind::Other => "Other",
            },
            self.name,
            self.uid
        )
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct ContainerMeta {
    /// Container runtime ID (e.g., docker://... or containerd://...)
    pub id: String,
    /// Container name
    pub name: String,
    /// Container image reference
    pub image: String,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct PodMeta {
    /// Pod UID
    pub uid: String,
    /// Pod IP address (empty if not yet assigned)
    pub ip: String,
    /// Pod name
    pub name: String,
    /// Namespace name
    pub namespace: String,
    /// Controller owner (if any)
    pub owner: Option<OwnerRef>,
    /// Whether the Pod uses host networking
    pub host_network: bool,
    /// Deterministic version built from container images
    pub version: String,
    /// Containers observed for this Pod
    pub containers: Vec<ContainerMeta>,
}

impl PodMeta {
    pub fn has_ip(&self) -> bool {
        !self.ip.is_empty()
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct OwnerMeta {
    /// UID of the owner (ReplicaSet or Job)
    pub uid: String,
    /// Its owner reference, if any (e.g. Deployment for RS, CronJob for Job)
    pub controller: Option<OwnerRef>,
}
