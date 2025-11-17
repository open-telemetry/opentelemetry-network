use std::collections::{BTreeMap, BTreeSet, HashMap};
use std::time::Duration;

use k8s_collector::matcher::Matcher;
use k8s_collector::output::RenderEvent;
use k8s_collector::tombstone_adapter::TombstoneAdapter;
use k8s_collector::types::{ContainerMeta, OwnerKind, OwnerMeta, OwnerRef, PodMeta};
use kube::runtime::reflector::store;
use kube::runtime::watcher::Event;
use proptest::prelude::*;

/// Minimal, synchronous harness that mirrors the production Collector pipeline:
/// - tombstone adapters in front of
/// - reflector writers feeding Stores used by
/// - Matcher, which produces RenderEvents.
struct PipelineHarness {
    matcher: Matcher,
    owners_writer: store::Writer<OwnerMeta>,
    pods_writer: store::Writer<PodMeta>,
    owner_tomb: TombstoneAdapter<OwnerMeta>,
    pod_tomb: TombstoneAdapter<PodMeta>,
}

impl PipelineHarness {
    fn new() -> Self {
        let (owners_store, owners_writer) = store::store::<OwnerMeta>();
        let (pods_store, pods_writer) = store::store::<PodMeta>();
        let matcher = Matcher::new(owners_store, pods_store);
        let owner_tomb = TombstoneAdapter::new(Duration::from_secs(0), 1024);
        let pod_tomb = TombstoneAdapter::new(Duration::from_secs(0), 1024);
        Self {
            matcher,
            owners_writer,
            pods_writer,
            owner_tomb,
            pod_tomb,
        }
    }

    fn apply_owner(&mut self, ev: Event<OwnerMeta>) -> Vec<RenderEvent> {
        let mut out = Vec::new();
        for fwd in self.owner_tomb.handle(ev) {
            self.owners_writer.apply_watcher_event(&fwd);
            out.extend(self.matcher.handle_owner(fwd));
        }
        out
    }

    fn apply_pod(&mut self, ev: Event<PodMeta>) -> Vec<RenderEvent> {
        let mut out = Vec::new();
        for fwd in self.pod_tomb.handle(ev) {
            self.pods_writer.apply_watcher_event(&fwd);
            out.extend(self.matcher.handle_pod(fwd));
        }
        out
    }
}

/// How a pod is owned in Kubernetes, restricted to the shapes
/// the collector actually supports (RS/Job with optional controller).
#[derive(Clone, Debug)]
enum OwnerScenario {
    NoOwner,
    ReplicaSetOnly,
    ReplicaSetWithDeployment,
    JobOnly,
    JobWithCronJob,
}

impl OwnerScenario {
    fn from_tag(tag: u8) -> Self {
        match tag % 5 {
            0 => OwnerScenario::NoOwner,
            1 => OwnerScenario::ReplicaSetOnly,
            2 => OwnerScenario::ReplicaSetWithDeployment,
            3 => OwnerScenario::JobOnly,
            _ => OwnerScenario::JobWithCronJob,
        }
    }

    /// Build the k8s meta objects for this scenario:
    /// - OwnerRef embedded into PodMeta (if any)
    /// - OwnerMeta objects for ReplicaSet/Job owners
    /// - The effective owner after escalation (Deployment/CronJob) for the model.
    fn build_for_pod(&self, pod_id: u8) -> (Option<OwnerRef>, Vec<OwnerMeta>, Option<OwnerRef>) {
        let rs_uid = format!("rs-{pod_id}");
        let rs_name = format!("rs-{pod_id}");
        let dep_uid = format!("dep-{pod_id}");
        let dep_name = format!("dep-{pod_id}");
        let job_uid = format!("job-{pod_id}");
        let job_name = format!("job-{pod_id}");
        let cron_uid = format!("cron-{pod_id}");
        let cron_name = format!("cron-{pod_id}");

        match self {
            OwnerScenario::NoOwner => (None, Vec::new(), None),
            OwnerScenario::ReplicaSetOnly => {
                let pod_owner = OwnerRef {
                    kind: OwnerKind::ReplicaSet,
                    uid: rs_uid.clone(),
                    name: rs_name.clone(),
                };
                let owner_meta = OwnerMeta {
                    uid: rs_uid,
                    controller: None,
                };
                (Some(pod_owner.clone()), vec![owner_meta], Some(pod_owner))
            }
            OwnerScenario::ReplicaSetWithDeployment => {
                let pod_owner = OwnerRef {
                    kind: OwnerKind::ReplicaSet,
                    uid: rs_uid.clone(),
                    name: rs_name.clone(),
                };
                let controller = OwnerRef {
                    kind: OwnerKind::Deployment,
                    uid: dep_uid.clone(),
                    name: dep_name.clone(),
                };
                let owner_meta = OwnerMeta {
                    uid: rs_uid,
                    controller: Some(controller.clone()),
                };
                (Some(pod_owner), vec![owner_meta], Some(controller))
            }
            OwnerScenario::JobOnly => {
                let pod_owner = OwnerRef {
                    kind: OwnerKind::Job,
                    uid: job_uid.clone(),
                    name: job_name.clone(),
                };
                let owner_meta = OwnerMeta {
                    uid: job_uid,
                    controller: None,
                };
                (Some(pod_owner.clone()), vec![owner_meta], Some(pod_owner))
            }
            OwnerScenario::JobWithCronJob => {
                let pod_owner = OwnerRef {
                    kind: OwnerKind::Job,
                    uid: job_uid.clone(),
                    name: job_name.clone(),
                };
                let controller = OwnerRef {
                    kind: OwnerKind::CronJob,
                    uid: cron_uid.clone(),
                    name: cron_name.clone(),
                };
                let owner_meta = OwnerMeta {
                    uid: job_uid,
                    controller: Some(controller.clone()),
                };
                (Some(pod_owner), vec![owner_meta], Some(controller))
            }
        }
    }
}

#[derive(Clone, Debug)]
struct ModelPod {
    pod_id: u8,
    uid: String,
    scenario: OwnerScenario,
    present: bool,
    has_ip: bool,
    effective_owner: Option<OwnerRef>,
}

#[derive(Clone, Debug, Default)]
struct ModelState {
    pods: BTreeMap<u8, ModelPod>,
    epoch: u64,
}

impl ModelState {
    fn is_live(&self, pod_id: u8) -> bool {
        self.pods
            .get(&pod_id)
            .map(|p| p.present && p.has_ip)
            .unwrap_or(false)
    }

    fn live_uids(&self) -> BTreeSet<String> {
        self.pods
            .values()
            .filter(|p| p.present && p.has_ip)
            .map(|p| p.uid.clone())
            .collect()
    }
}

/// Helper to construct a simple PodMeta (single container, fixed image)
/// with a stable UID/IP derived from pod_id.
fn build_pod_meta(pod_id: u8, owner: Option<OwnerRef>) -> PodMeta {
    let uid = format!("pod-{pod_id}");
    PodMeta {
        uid,
        ip: format!("10.0.0.{pod_id}"),
        name: format!("pod-{pod_id}"),
        namespace: "default".into(),
        owner,
        host_network: false,
        version: "'img:1'".into(),
        containers: vec![ContainerMeta {
            id: format!("c-{pod_id}"),
            name: "c".into(),
            image: "img:1".into(),
        }],
    }
}

#[derive(Clone, Debug)]
enum Op {
    AddPod { pod_id: u8, scenario: OwnerScenario },
    DeletePod { pod_id: u8 },
    Reinit,
}

#[test]
fn prop_collector_matches_pod_model() {
    let config = ProptestConfig {
        cases: 64,
        ..ProptestConfig::default()
    };

    // Each step carries:
    // - tag: selects op kind and also flips arrival order in AddPod
    // - pod_id: small identifier mapped to a UID
    // - owner_tag: mapped into an OwnerScenario
    proptest!(config, |(ops in proptest::collection::vec((any::<u8>(), any::<u8>(), any::<u8>()), 0..128))| {
        let mut harness = PipelineHarness::new();
        let mut model = ModelState::default();
        let mut first_seen: HashMap<String, &'static str> = HashMap::new();
        let mut last_epoch: u64 = 0;

        for (tag, pod_id, owner_tag) in ops {
            let scenario = OwnerScenario::from_tag(owner_tag);
            let op = match tag % 3 {
                0 => Op::AddPod { pod_id, scenario },
                1 => Op::DeletePod { pod_id },
                _ => Op::Reinit,
            };

            let mut step_events: Vec<RenderEvent> = Vec::new();

            match op {
                Op::AddPod { pod_id, scenario } => {
                    // Add or update a pod with a particular ownership scenario.
                    // We randomize whether owners or pod arrive first and assert:
                    // - exactly one PodNew when the pod becomes live
                    // - PodNew comes from the second arrival
                    // - PodNew owner fields match the scenario.
                    let was_live = model.is_live(pod_id);
                    let (pod_owner_ref, owners_meta, effective_owner) = scenario.build_for_pod(pod_id);
                    let uid = format!("pod-{pod_id}");
                    let pod_meta = build_pod_meta(pod_id, pod_owner_ref.clone());

                    let owners_first = (tag & 0x80) != 0;
                    let mut owner_events = Vec::new();
                    let mut pod_events = Vec::new();

                    if owners_first {
                        for owner in owners_meta.iter().cloned() {
                            owner_events.extend(harness.apply_owner(Event::Apply(owner)));
                        }
                        pod_events.extend(harness.apply_pod(Event::Apply(pod_meta)));
                    } else {
                        pod_events.extend(harness.apply_pod(Event::Apply(pod_meta)));
                        for owner in owners_meta.iter().cloned() {
                            owner_events.extend(harness.apply_owner(Event::Apply(owner)));
                        }
                    }

                    model.pods.insert(
                        pod_id,
                        ModelPod {
                            pod_id,
                            uid: uid.clone(),
                            scenario,
                            present: true,
                            has_ip: true,
                            effective_owner,
                        },
                    );
                    let now_live = model.is_live(pod_id);

                    if !was_live && now_live {
                        let num_podnew_owner = owner_events
                            .iter()
                            .filter(|ev| matches!(ev, RenderEvent::PodNew { uid: ev_uid, .. } if ev_uid == &uid))
                            .count();
                        let num_podnew_pod = pod_events
                            .iter()
                            .filter(|ev| matches!(ev, RenderEvent::PodNew { uid: ev_uid, .. } if ev_uid == &uid))
                            .count();
                        let total_podnew = num_podnew_owner + num_podnew_pod;
                        prop_assert_eq!(total_podnew, 1, "expected exactly one PodNew for uid {}", uid);

                        let mp = model.pods.get(&pod_id).expect("model should contain pod after AddPod");

                        match &mp.scenario {
                            OwnerScenario::NoOwner => {
                                prop_assert_eq!(num_podnew_pod, 1, "NoOwner PodNew should come from pod_events");
                                prop_assert_eq!(num_podnew_owner, 0, "NoOwner PodNew should not come from owner_events");
                            }
                            _ => {
                                if owners_first {
                                    prop_assert_eq!(num_podnew_pod, 1, "owners_first: PodNew should come from pod_events");
                                    prop_assert_eq!(num_podnew_owner, 0, "owners_first: PodNew should not come from owner_events");
                                } else {
                                    prop_assert_eq!(num_podnew_owner, 1, "pod_first: PodNew should come from owner_events");
                                    prop_assert_eq!(num_podnew_pod, 0, "pod_first: PodNew should not come from pod_events");
                                }
                            }
                        }

                        let podnew_ev = owner_events
                            .iter()
                            .chain(pod_events.iter())
                            .find(|ev| matches!(ev, RenderEvent::PodNew { uid: ev_uid, .. } if ev_uid == &uid))
                            .expect("PodNew must be present for live transition");

                        if let RenderEvent::PodNew {
                            owner_kind,
                            owner_uid,
                            owner_name,
                            pod_name,
                            ..
                        } = podnew_ev
                        {
                            match &mp.effective_owner {
                                None => {
                                    prop_assert_eq!(
                                        *owner_kind,
                                        OwnerKind::NoOwner,
                                        "AddPod: expected NoOwner for uid {}",
                                        uid
                                    );
                                    prop_assert!(
                                        owner_uid.is_empty(),
                                        "AddPod: owner_uid should be empty for NoOwner uid {}",
                                        uid
                                    );
                                    prop_assert_eq!(
                                        owner_name,
                                        pod_name,
                                        "AddPod: owner_name should equal pod_name for NoOwner uid {}",
                                        uid
                                    );
                                }
                                Some(o) => {
                                    prop_assert_eq!(
                                        *owner_kind,
                                        o.kind,
                                        "AddPod: owner_kind mismatch for uid {}",
                                        uid
                                    );
                                    prop_assert_eq!(
                                        owner_uid,
                                        &o.uid,
                                        "AddPod: owner_uid mismatch for uid {}",
                                        uid
                                    );
                                    prop_assert_eq!(
                                        owner_name,
                                        &o.name,
                                        "AddPod: owner_name mismatch for uid {}",
                                        uid
                                    );
                                }
                            }
                        }
                    }

                    step_events.extend(owner_events);
                    step_events.extend(pod_events);
                }
                Op::DeletePod { pod_id } => {
                    // Delete a pod if present; we only assert indirectly via
                    // subsequent resync snapshots (PodNew should no longer appear).
                    if let Some(mp) = model.pods.get_mut(&pod_id) {
                        mp.present = false;
                        let (owner_ref, _owners, _effective_owner) = mp.scenario.build_for_pod(pod_id);
                        let pod_meta = build_pod_meta(pod_id, owner_ref);
                        let events = harness.apply_pod(Event::Delete(pod_meta));
                        step_events.extend(events);
                    }
                }
                Op::Reinit => {
                    // Simulate a watcher re-init: Init/InitApply/InitDone for
                    // owners then pods. Matcher should emit:
                    // - a new PodResync{epoch}
                    // - PodNew for exactly the live pods in the model.
                    model.epoch = model.epoch.wrapping_add(1);

                    let mut owners_snapshot: Vec<OwnerMeta> = Vec::new();
                    for mp in model.pods.values() {
                        if !mp.present {
                            continue;
                        }
                        let (_owner_ref, owners_meta, _effective_owner) = mp.scenario.build_for_pod(mp.pod_id);
                        owners_snapshot.extend(owners_meta);
                    }

                    let mut pods_snapshot: Vec<PodMeta> = Vec::new();
                    for mp in model.pods.values() {
                        if !mp.present {
                            continue;
                        }
                        let (owner_ref, _owners_meta, _effective_owner) = mp.scenario.build_for_pod(mp.pod_id);
                        let pod_meta = build_pod_meta(mp.pod_id, owner_ref);
                        pods_snapshot.push(pod_meta);
                    }

                    let mut epoch_events: Vec<RenderEvent> = Vec::new();
                    epoch_events.extend(harness.apply_owner(Event::Init));
                    for owner in owners_snapshot.into_iter() {
                        epoch_events.extend(harness.apply_owner(Event::InitApply(owner)));
                    }
                    epoch_events.extend(harness.apply_owner(Event::InitDone));
                    epoch_events.extend(harness.apply_pod(Event::Init));
                    for pod_meta in pods_snapshot.into_iter() {
                        epoch_events.extend(harness.apply_pod(Event::InitApply(pod_meta)));
                    }
                    epoch_events.extend(harness.apply_pod(Event::InitDone));

                    let mut resync_epoch: Option<u64> = None;
                    let mut seen_resync = false;
                    let mut snapshot_uids: BTreeSet<String> = BTreeSet::new();
                    for ev in &epoch_events {
                        match ev {
                            RenderEvent::PodResync { epoch } => {
                                if !seen_resync {
                                    resync_epoch = Some(*epoch);
                                    seen_resync = true;
                                }
                            }
                            RenderEvent::PodNew { uid, .. } => {
                                if seen_resync {
                                    snapshot_uids.insert(uid.clone());
                                }
                            }
                            _ => {}
                        }
                    }

                    prop_assert!(seen_resync, "expected PodResync event on Reinit");
                    let epoch = resync_epoch.unwrap();
                    prop_assert!(epoch > last_epoch, "epoch should increase monotonically");
                    last_epoch = epoch;

                    let live = model.live_uids();
                    prop_assert_eq!(snapshot_uids, live, "snapshot PodNew uids must equal live set");

                    step_events.extend(epoch_events);
                }
            }

            for ev in &step_events {
                match ev {
                    RenderEvent::PodNew {
                        uid,
                        owner_kind,
                        owner_uid,
                        owner_name,
                        pod_name,
                        ..
                    } => {
                        // First event for a uid must be PodNew.
                        if !first_seen.contains_key(uid) {
                            first_seen.insert(uid.clone(), "PodNew");
                        }

                        let mp_opt = model
                            .pods
                            .values()
                            .find(|p| &p.uid == uid);
                        prop_assert!(mp_opt.is_some(), "PodNew for unknown uid {}", uid);
                        let mp = mp_opt.unwrap();
                        match &mp.effective_owner {
                            None => {
                                prop_assert_eq!(*owner_kind, OwnerKind::NoOwner, "expected NoOwner for uid {}", uid);
                                prop_assert!(owner_uid.is_empty(), "owner_uid should be empty for NoOwner uid {}", uid);
                                prop_assert_eq!(owner_name, pod_name, "owner_name should equal pod_name for NoOwner uid {}", uid);
                            }
                            Some(o) => {
                                prop_assert_eq!(*owner_kind, o.kind, "owner_kind mismatch for uid {}", uid);
                                prop_assert_eq!(owner_uid, &o.uid, "owner_uid mismatch for uid {}", uid);
                                prop_assert_eq!(owner_name, &o.name, "owner_name mismatch for uid {}", uid);
                            }
                        }
                    }
                    RenderEvent::PodContainer { uid, .. } => {
                        // PodContainer should only follow PodNew for a uid.
                        if !first_seen.contains_key(uid) {
                            prop_assert!(false, "first event for uid {} was PodContainer, not PodNew", uid);
                        }
                    }
                    RenderEvent::PodDelete { uid } => {
                        // PodDelete should only follow PodNew for a uid.
                        if !first_seen.contains_key(uid) {
                            prop_assert!(false, "first event for uid {} was PodDelete, not PodNew", uid);
                        }
                    }
                    _ => {}
                }
            }
        }
    });
}
