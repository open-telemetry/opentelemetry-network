//! Pod↔Owner matching engine.
//!
//! Maintains external kube reflectors Stores for owners and pods, correlates
//! pods with their
//! effective owners, and produces normalized render events:
//! - Emit PodNew(+containers) when a pod is resolvable and has an IP
//! - Emit PodContainer updates for subsequent Apply on live pods
//! - Emit PodDelete when a live pod is deleted
//! - Resync epochs on InitDone (double-buffer snapshot becomes active).
//!
//! Design notes:
//! - Stores are provided externally to `Matcher` and are fed by kube reflectors.
//!   `Init`/`InitApply`/`InitDone` are handled by the reflector writer; on
//!   `InitDone`, `Store::state()` exposes the new snapshot. `Matcher` uses this
//!   snapshot in `start_new_epoch()` to re-emit resolvable pods.
//! - `Lookup` is implemented for `PodMeta` and `OwnerMeta` so `Store` keys by
//!   UID as a cluster-scoped identifier. We construct `ObjectRef` keys via
//!   small helpers (`pod_ref`, `owner_ref`) and use `Store::get` for O(1)
//!   lookups.
//!
//! Owner escalation: RS→Deployment, Job→CronJob (when controller owner exists).

use std::borrow::Cow;
use std::collections::{HashMap, HashSet};

use crate::output::RenderEvent;
use crate::types::{OwnerKind, OwnerMeta, PodMeta};
use kube::runtime::reflector::{Lookup, ObjectRef, Store};
use kube::runtime::watcher::Event;

// Implement kube-runtime's Lookup trait for our compact meta types so that
// they can be used with reflector Stores. We key by UID as a cluster-scoped
// identifier to avoid namespace/name collisions and reduce state surface.
impl Lookup for PodMeta {
    type DynamicType = ();

    fn kind(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("PodMeta")
    }
    fn group(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("opentelemetry.network")
    }
    fn version(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("v1")
    }
    fn plural(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("podmeta")
    }
    fn name(&self) -> Option<Cow<'_, str>> {
        Some(Cow::Borrowed(self.uid.as_str()))
    }
    fn namespace(&self) -> Option<Cow<'_, str>> {
        None
    }
    fn resource_version(&self) -> Option<Cow<'_, str>> {
        None
    }
    fn uid(&self) -> Option<Cow<'_, str>> {
        Some(Cow::Borrowed(self.uid.as_str()))
    }
}

impl Lookup for OwnerMeta {
    type DynamicType = ();

    fn kind(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("OwnerMeta")
    }
    fn group(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("opentelemetry.network")
    }
    fn version(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("v1")
    }
    fn plural(_: &Self::DynamicType) -> Cow<'_, str> {
        Cow::Borrowed("ownermeta")
    }
    fn name(&self) -> Option<Cow<'_, str>> {
        Some(Cow::Borrowed(self.uid.as_str()))
    }
    fn namespace(&self) -> Option<Cow<'_, str>> {
        None
    }
    fn resource_version(&self) -> Option<Cow<'_, str>> {
        None
    }
    fn uid(&self) -> Option<Cow<'_, str>> {
        Some(Cow::Borrowed(self.uid.as_str()))
    }
}

pub struct Matcher {
    epoch: u64,
    owners: Store<OwnerMeta>,
    waiting_by_owner: HashMap<String, Vec<String>>, // owner uid -> pod uids
    pods: Store<PodMeta>,
    live_pods: HashSet<String>,
}

impl Matcher {
    /// Create a new matcher with the provided Stores.
    pub fn new(owners: Store<OwnerMeta>, pods: Store<PodMeta>) -> Self {
        Self {
            epoch: 0,
            owners,
            waiting_by_owner: HashMap::new(),
            pods,
            live_pods: HashSet::new(),
        }
    }

    /// Helper: build an ObjectRef for a Pod by UID (cluster-scoped key)
    fn pod_ref(uid: &str) -> ObjectRef<PodMeta> {
        ObjectRef::<PodMeta>::new(uid)
    }

    /// Helper: build an ObjectRef for an Owner by UID (cluster-scoped key)
    fn owner_ref(uid: &str) -> ObjectRef<OwnerMeta> {
        ObjectRef::<OwnerMeta>::new(uid)
    }

    /// Handle a single Owner event and return any resulting render events.
    pub fn handle_owner(&mut self, ev: Event<OwnerMeta>) -> Vec<RenderEvent> {
        match ev {
            Event::Init => Vec::new(),
            Event::InitApply(_m) => Vec::new(),
            Event::Apply(m) => {
                let uid = m.uid.clone();
                // Drain waiters for this owner if any
                let mut out = Vec::new();
                if let Some(pods) = self.waiting_by_owner.remove(&uid) {
                    for puid in pods {
                        if let Some(p) = self.pods.get(&Self::pod_ref(&puid)).as_deref().cloned() {
                            out.extend(self.try_emit_pod(p));
                        }
                    }
                }
                out
            }
            Event::Delete(_m) => Vec::new(),
            Event::InitDone => {
                // Scan waiting pods against the now-initialized owner store
                let mut out = Vec::new();
                let keys: Vec<String> = self.waiting_by_owner.keys().cloned().collect();
                for owner_uid in keys {
                    if self.owners.get(&Self::owner_ref(&owner_uid)).is_some() {
                        if let Some(pods) = self.waiting_by_owner.remove(&owner_uid) {
                            for puid in pods {
                                if let Some(p) =
                                    self.pods.get(&Self::pod_ref(&puid)).as_deref().cloned()
                                {
                                    out.extend(self.try_emit_pod(p));
                                }
                            }
                        }
                    }
                }
                out
            }
        }
    }

    /// Handle a single Pod event and return any resulting render events.
    pub fn handle_pod(&mut self, ev: Event<PodMeta>) -> Vec<RenderEvent> {
        match ev {
            Event::Init => Vec::new(),
            Event::InitApply(_p) => Vec::new(),
            Event::Apply(p) => {
                if self.live_pods.contains(&p.uid) {
                    // Live pod: containers only
                    return p
                        .containers
                        .into_iter()
                        .map(|c| RenderEvent::PodContainer {
                            uid: p.uid.clone(),
                            container: c,
                        })
                        .collect();
                }
                self.try_emit_pod(p)
            }
            Event::Delete(p) => {
                let mut out = Vec::new();
                if self.live_pods.remove(&p.uid) {
                    out.push(RenderEvent::PodDelete { uid: p.uid.clone() });
                }
                for waiters in self.waiting_by_owner.values_mut() {
                    waiters.retain(|w| w != &p.uid);
                }
                out
            }
            Event::InitDone => self.start_new_epoch(),
        }
    }

    /// Start a new resync epoch, clearing waiting queues and emitting a
    /// `PodResync` event followed by any pods now resolvable from the current
    /// store contents.
    pub fn start_new_epoch(&mut self) -> Vec<RenderEvent> {
        self.epoch = self.epoch.wrapping_add(1);
        self.waiting_by_owner.clear();
        let mut out = vec![RenderEvent::PodResync { epoch: self.epoch }];
        // Iterate pod store snapshot and emit resolvable ones
        for p in self.pods.state().into_iter().map(|a| (*a).clone()) {
            out.extend(self.try_emit_pod(p));
        }
        out
    }

    /// Attempt to emit `PodNew` (+containers) for the given pod when resolvable.
    ///
    fn try_emit_pod(&mut self, p: PodMeta) -> Vec<RenderEvent> {
        if !p.has_ip() {
            return Vec::new();
        }

        // No owner
        let owner = match &p.owner {
            None => {
                self.live_pods.insert(p.uid.clone());
                return crate::output::RenderEvent::from_pod_new(&p, None);
            }
            Some(o) => o.clone(),
        };

        if !owner.kind.is_rs_or_job() {
            self.live_pods.insert(p.uid.clone());
            return crate::output::RenderEvent::from_pod_new(&p, Some(&owner));
        }

        // Owner is RS/Job: check if owner info is known and possibly escalate
        if let Some(om) = self.owners.get(&Self::owner_ref(&owner.uid)).as_deref() {
            let escalated = match &om.controller {
                Some(ctrl) if matches!(ctrl.kind, OwnerKind::Deployment | OwnerKind::CronJob) => {
                    ctrl.clone()
                }
                _ => owner,
            };
            self.live_pods.insert(p.uid.clone());
            return crate::output::RenderEvent::from_pod_new(&p, Some(&escalated));
        }

        // Not known yet: queue
        let w = self.waiting_by_owner.entry(owner.uid.clone()).or_default();
        if !w.iter().any(|u| u == &p.uid) {
            w.push(p.uid.clone());
        }
        Vec::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::{ContainerMeta, OwnerRef};
    use kube::runtime::reflector::store;

    struct Harness {
        m: Matcher,
        owners_w: kube::runtime::reflector::store::Writer<OwnerMeta>,
        pods_w: kube::runtime::reflector::store::Writer<PodMeta>,
    }

    impl Harness {
        fn new() -> Self {
            let (owners_store, owners_w) = store::store::<OwnerMeta>();
            let (pods_store, pods_w) = store::store::<PodMeta>();
            let m = Matcher::new(owners_store, pods_store);
            Self {
                m,
                owners_w,
                pods_w,
            }
        }
        fn handle_owner(&mut self, ev: Event<OwnerMeta>) -> Vec<RenderEvent> {
            self.owners_w.apply_watcher_event(&ev);
            self.m.handle_owner(ev)
        }
        fn handle_pod(&mut self, ev: Event<PodMeta>) -> Vec<RenderEvent> {
            self.pods_w.apply_watcher_event(&ev);
            self.m.handle_pod(ev)
        }
    }

    fn pod(uid: &str, ip: &str, owner: Option<OwnerRef>) -> PodMeta {
        PodMeta {
            uid: uid.into(),
            ip: ip.into(),
            name: format!("pod-{}", uid),
            namespace: "default".into(),
            owner,
            host_network: false,
            version: "'img:1'".into(),
            containers: vec![ContainerMeta {
                id: format!("c-{}", uid),
                name: "c".into(),
                image: "img:1".into(),
            }],
        }
    }

    fn rs_owner(uid: &str, controller: Option<OwnerRef>) -> OwnerMeta {
        OwnerMeta {
            uid: uid.into(),
            controller,
        }
    }

    fn ref_kind(kind: OwnerKind, uid: &str, name: &str) -> OwnerRef {
        OwnerRef {
            kind,
            uid: uid.into(),
            name: name.into(),
        }
    }

    #[test]
    // Pod arrives first, then owner Apply arrives later.
    // The pod should be emitted once owner information is available.
    fn pod_first_then_owner_emits_on_owner_apply() {
        let mut h = Harness::new();
        let events = h.handle_pod(Event::Apply(pod(
            "p1",
            "10.0.0.1",
            Some(ref_kind(OwnerKind::ReplicaSet, "r1", "rs")),
        )));
        assert!(events.is_empty());
        let out = h.handle_owner(Event::Apply(rs_owner(
            "r1",
            Some(ref_kind(OwnerKind::Deployment, "d1", "dep")),
        )));
        assert_eq!(out.len(), 2);
        match &out[0] {
            RenderEvent::PodNew {
                owner_kind,
                owner_uid,
                ..
            } => {
                assert_eq!(*owner_kind, OwnerKind::Deployment);
                assert_eq!(owner_uid, "d1");
            }
            _ => panic!("expected PodNew"),
        }
        match &out[1] {
            RenderEvent::PodContainer { uid, .. } => assert_eq!(uid, "p1"),
            _ => panic!("expected PodContainer"),
        }
    }

    #[test]
    // Owner arrives first, then pod Apply.
    // The pod should be emitted immediately using escalated owner when applicable.
    fn owner_first_then_pod_emits_immediately() {
        let mut h = Harness::new();
        let _ = h.handle_owner(Event::Apply(rs_owner(
            "r2",
            Some(ref_kind(OwnerKind::Deployment, "d2", "dep")),
        )));
        let out = h.handle_pod(Event::Apply(pod(
            "p2",
            "10.0.0.2",
            Some(ref_kind(OwnerKind::ReplicaSet, "r2", "rs")),
        )));
        assert_eq!(out.len(), 2);
        match &out[0] {
            RenderEvent::PodNew {
                owner_kind,
                owner_uid,
                ..
            } => {
                assert_eq!(*owner_kind, OwnerKind::Deployment);
                assert_eq!(owner_uid, "d2");
            }
            _ => panic!("expected PodNew"),
        }
        match &out[1] {
            RenderEvent::PodContainer { uid, .. } => assert_eq!(uid, "p2"),
            _ => panic!("expected PodContainer"),
        }
    }

    #[test]
    fn initdone_starts_epoch_and_emits_resolvable() {
        let mut h = Harness::new();
        let _ = h.handle_owner(Event::InitApply(rs_owner(
            "r3",
            Some(ref_kind(OwnerKind::Deployment, "d3", "dep")),
        )));
        // Complete owner init to swap snapshot into the owner Store
        let _ = h.handle_owner(Event::InitDone);
        let _ = h.handle_pod(Event::InitApply(pod(
            "p3",
            "10.0.0.3",
            Some(ref_kind(OwnerKind::ReplicaSet, "r3", "rs")),
        )));
        // Complete pod init to swap snapshot into the pod Store and start new epoch
        let out = h.handle_pod(Event::InitDone);
        assert!(matches!(out.first(), Some(RenderEvent::PodResync { .. })));
        assert!(out.iter().any(|e| matches!(e, RenderEvent::PodNew { .. })));
    }

    #[test]
    // When a pod has no owner, emit PodNew with NoOwner and containers.
    fn ownerless_pod_emits_with_no_owner() {
        let mut h = Harness::new();
        let out = h.handle_pod(Event::Apply(pod("p0", "10.0.0.10", None)));
        assert_eq!(out.len(), 2);
        match &out[0] {
            RenderEvent::PodNew {
                owner_kind,
                owner_uid,
                ..
            } => {
                assert_eq!(*owner_kind, OwnerKind::NoOwner);
                assert!(owner_uid.is_empty());
            }
            _ => panic!("expected PodNew"),
        }
        match &out[1] {
            RenderEvent::PodContainer { uid, .. } => assert_eq!(uid, "p0"),
            _ => panic!("expected PodContainer"),
        }
    }

    #[test]
    // If the pod is owned by a non-RS/Job owner, emit directly with that owner.
    fn non_rs_job_owner_emits_directly() {
        let mut h = Harness::new();
        let owner = ref_kind(OwnerKind::Other, "x1", "owner");
        let out = h.handle_pod(Event::Apply(pod("p9", "10.0.0.9", Some(owner.clone()))));
        assert_eq!(out.len(), 2);
        match &out[0] {
            RenderEvent::PodNew {
                owner_kind,
                owner_uid,
                owner_name,
                ..
            } => {
                assert_eq!(*owner_kind, OwnerKind::Other);
                assert_eq!(owner_uid, "x1");
                assert_eq!(owner_name, "owner");
            }
            _ => panic!("expected PodNew"),
        }
        match &out[1] {
            RenderEvent::PodContainer { uid, .. } => assert_eq!(uid, "p9"),
            _ => panic!("expected PodContainer"),
        }
    }

    #[test]
    // Job→CronJob escalation: if the Job's controller is a CronJob,
    // emit the pod with the CronJob as the effective owner.
    fn job_cronjob_escalation() {
        let mut h = Harness::new();
        // pod owned by Job j1 but owner meta has controller CronJob c1
        let _ = h.handle_owner(Event::Apply(rs_owner(
            "j1",
            Some(ref_kind(OwnerKind::CronJob, "c1", "cj")),
        )));
        // Actually RS owner meta function used for RS/Job both - here using rs_owner to avoid boilerplate as both use OwnerMeta
        let out = h.handle_pod(Event::Apply(pod(
            "pj",
            "10.0.0.20",
            Some(ref_kind(OwnerKind::Job, "j1", "job")),
        )));
        assert_eq!(out.len(), 2);
        match &out[0] {
            RenderEvent::PodNew {
                owner_kind,
                owner_uid,
                owner_name,
                ..
            } => {
                assert_eq!(*owner_kind, OwnerKind::CronJob);
                assert_eq!(owner_uid, "c1");
                assert_eq!(owner_name, "cj");
            }
            _ => panic!("expected PodNew"),
        }
    }

    #[test]
    // Deleting a live pod should produce a PodDelete event and clear its state.
    fn delete_emits_pod_delete_when_live() {
        let mut h = Harness::new();
        let _ = h.handle_owner(Event::Apply(rs_owner(
            "r4",
            Some(ref_kind(OwnerKind::Deployment, "d4", "dep")),
        )));
        let _ = h.handle_pod(Event::Apply(pod(
            "pd",
            "10.0.0.44",
            Some(ref_kind(OwnerKind::ReplicaSet, "r4", "rs")),
        )));
        let out = h.handle_pod(Event::Delete(pod(
            "pd",
            "10.0.0.44",
            Some(ref_kind(OwnerKind::ReplicaSet, "r4", "rs")),
        )));
        assert_eq!(out.len(), 1);
        match &out[0] {
            RenderEvent::PodDelete { uid } => assert_eq!(uid, "pd"),
            _ => panic!("expected PodDelete"),
        }
    }

    #[test]
    // Owner flaps Apply+Delete: the delete is retained in the tombstone adapter.
    // While retained, the store still reflects the owner Apply, so a pod Apply
    // that depends on this owner should still emit.
    fn owner_flaps_apply_delete_then_pod_apply_emits() {
        let mut h = Harness::new();
        let mut owner_tomb = crate::tombstone_adapter::TombstoneAdapter::new(
            std::time::Duration::from_secs(60),
            100,
        );
        // Owner Apply forwarded
        for e in owner_tomb.handle(Event::Apply(rs_owner(
            "ro",
            Some(ref_kind(OwnerKind::Deployment, "dep", "dep")),
        ))) {
            let _ = h.handle_owner(e);
        }
        // Owner Delete retained (no events forwarded)
        assert!(owner_tomb
            .handle(Event::Delete(rs_owner(
                "ro",
                Some(ref_kind(OwnerKind::Deployment, "dep", "dep"))
            )))
            .next()
            .is_none());
        // Pod Apply should emit now because owner is still present in store
        let out = h.handle_pod(Event::Apply(pod(
            "pp",
            "10.0.0.60",
            Some(ref_kind(OwnerKind::ReplicaSet, "ro", "rs")),
        )));
        assert!(out.iter().any(|e| matches!(e, RenderEvent::PodNew { .. })));
    }

    #[test]
    // Pod flaps Apply+Delete: the delete is retained in the pod tombstone adapter.
    // The pod Apply is forwarded (no emit because owner missing); on a later owner Apply,
    // the pod should still emit despite the retained delete.
    fn pod_flaps_apply_delete_then_owner_apply_emits() {
        let mut h = Harness::new();
        let mut pod_tomb = crate::tombstone_adapter::TombstoneAdapter::new(
            std::time::Duration::from_secs(60),
            100,
        );
        // Pod Apply forwarded to matcher (no owner yet -> no output)
        for e in pod_tomb.handle(Event::Apply(pod(
            "pp2",
            "10.0.0.61",
            Some(ref_kind(OwnerKind::ReplicaSet, "ro2", "rs")),
        ))) {
            let out = h.handle_pod(e);
            assert!(out.is_empty());
        }
        // Pod Delete retained by tombstone adapter
        assert!(pod_tomb
            .handle(Event::Delete(pod(
                "pp2",
                "10.0.0.61",
                Some(ref_kind(OwnerKind::ReplicaSet, "ro2", "rs"))
            )))
            .next()
            .is_none());
        // Owner Apply arrives -> should emit pod despite pending delete
        let out = h.handle_owner(Event::Apply(rs_owner(
            "ro2",
            Some(ref_kind(OwnerKind::Deployment, "dep2", "dep")),
        )));
        assert!(out.iter().any(|e| matches!(e, RenderEvent::PodNew { .. })));
    }
}
