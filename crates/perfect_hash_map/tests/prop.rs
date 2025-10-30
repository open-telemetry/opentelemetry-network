//! Property-based tests using proptest. We model expected behavior with a
//! shadow vector that mirrors PerfectHashMap semantics (no probing; key must
//! match at the hashed slot). Each random operation is checked against both
//! models and invariants are asserted after every step.

use perfect_hash_map::PerfectHashMap;
use proptest::prelude::*;

// Simple shadow model mirroring PerfectHashMap behavior.
#[derive(Clone, Debug)]
struct Shadow {
    slots: Vec<Option<(u32, i32)>>,
}

impl Shadow {
    fn new(n: usize) -> Self {
        Self {
            slots: vec![None; n],
        }
    }
    fn capacity(&self) -> usize {
        self.slots.len()
    }
    fn hash(&self, seed: u32, k: u32) -> usize {
        ((k ^ seed) % (self.capacity() as u32)) as usize
    }

    fn insert(&mut self, seed: u32, key: u32, val: i32) -> Result<Option<i32>, u32> {
        let i = self.hash(seed, key);
        match &mut self.slots[i] {
            None => {
                self.slots[i] = Some((key, val));
                Ok(None)
            }
            Some((k, v)) if *k == key => Ok(Some(std::mem::replace(v, val))),
            Some((k, _)) => Err(*k),
        }
    }
    fn try_insert(&mut self, seed: u32, key: u32, val: i32) -> Result<(), TryOutcome> {
        let i = self.hash(seed, key);
        match &mut self.slots[i] {
            None => {
                self.slots[i] = Some((key, val));
                Ok(())
            }
            Some((k, _v)) if *k == key => Err(TryOutcome::Occupied),
            Some((k, _)) => Err(TryOutcome::Collision { existing_key: *k }),
        }
    }
    fn get(&self, seed: u32, key: u32) -> Option<i32> {
        let i = self.hash(seed, key);
        match &self.slots[i] {
            Some((k, v)) if *k == key => Some(*v),
            _ => None,
        }
    }
    fn get_mut_add(&mut self, seed: u32, key: u32, delta: i32) -> bool {
        let i = self.hash(seed, key);
        match &mut self.slots[i] {
            Some((k, v)) if *k == key => {
                *v += delta;
                true
            }
            _ => false,
        }
    }
    fn remove(&mut self, seed: u32, key: u32) -> Option<i32> {
        let i = self.hash(seed, key);
        match &mut self.slots[i] {
            Some((k, _)) if *k == key => self.slots[i].take().map(|(_, v)| v),
            _ => None,
        }
    }
    fn clear(&mut self) {
        for s in &mut self.slots {
            *s = None;
        }
    }
    fn len(&self) -> usize {
        self.slots.iter().filter(|e| e.is_some()).count()
    }
}

#[derive(Clone, Debug)]
enum TryOutcome {
    Occupied,
    Collision { existing_key: u32 },
}

#[derive(Clone, Debug)]
enum Op {
    Insert { key: u32, val: i32 },
    TryInsert { key: u32, val: i32 },
    Remove { key: u32 },
    Get { key: u32 },
    GetMutAdd { key: u32, delta: i32 },
    Clear,
}

/// Executes a random sequence of operations and checks behavior against a shadow model,
/// asserting invariants after each step.
#[test]
fn prop_sequence_matches_shadow() {
    let config = ProptestConfig {
        cases: 64,
        ..ProptestConfig::default()
    };
    proptest!(config, |(cap in 1usize..16, seed in any::<u32>(), ops in prop::collection::vec((any::<u8>(), any::<u32>(), any::<i32>()), 0..100))| {
        let hash = move |k: u32| (k ^ seed) % (cap as u32);
        let mut m = PerfectHashMap::new(cap, hash);
        let mut sh = Shadow::new(cap);

        for (tag, kraw, vraw) in ops.iter().copied() {
            let key = if cap == 0 { 0 } else { kraw % ((cap as u32) * 4) };
            let delta = (vraw % 51) as i32; // small delta for get_mut_add
            let op = match tag % 6 {
                0 => Op::Insert { key, val: vraw },
                1 => Op::TryInsert { key, val: vraw },
                2 => Op::Remove { key },
                3 => Op::Get { key },
                4 => Op::GetMutAdd { key, delta },
                _ => Op::Clear,
            };
            match op {
                Op::Insert { key, val } => {
                    let model = sh.insert(seed, key, val);
                    let dut = m.insert(key, val);
                    match (model, dut) {
                        (Ok(None), Ok(None)) => {},
                        (Ok(Some(x)), Ok(Some(y))) => assert_eq!(x, y),
                        (Err(km), Err(ke)) => assert_eq!(km, ke.existing_key),
                        other => panic!("mismatch on insert: {:?}", other),
                    }
                }
                Op::TryInsert { key, val } => {
                    let model = sh.try_insert(seed, key, val);
                    let dut = m.try_insert(key, val);
                    match (model, dut) {
                        (Ok(()), Ok(_)) => {},
                        (Err(TryOutcome::Occupied), Err(perfect_hash_map::TryInsertError::Occupied { .. })) => {},
                        (Err(TryOutcome::Collision { existing_key: km }), Err(perfect_hash_map::TryInsertError::Collision { existing_key: ke, .. })) => assert_eq!(km, ke),
                        other => panic!("mismatch on try_insert: {:?}", other),
                    }
                }
                Op::Remove { key } => {
                    let model = sh.remove(seed, key);
                    let dut = m.remove(&key);
                    prop_assert_eq!(dut, model);
                }
                Op::Get { key } => {
                    let model = sh.get(seed, key);
                    let dut = m.get(&key).copied();
                    prop_assert_eq!(dut, model);
                }
                Op::GetMutAdd { key, delta } => {
                    let _ = sh.get_mut_add(seed, key, delta);
                    if let Some(v) = m.get_mut(&key) { *v = v.saturating_add(delta); }
                }
                Op::Clear => {
                    sh.clear();
                    m.clear();
                }
            }
            // Verify invariants after each operation
            prop_assert_eq!(m.len(), sh.len());
            // Compare by slot order via iter; build current snapshot from shadow
            let mut exp = Vec::new();
            for s in &sh.slots { if let Some((k,v)) = s { exp.push((*k,*v)); } }
            let got: Vec<(u32, i32)> = m.iter().map(|(k,v)| (*k,*v)).collect();
            prop_assert_eq!(got, exp);
        }
    });
}

/// After a random state is built, applying iter_mut with a transform updates all values as expected.
#[test]
fn prop_iter_mut_transform() {
    let config = ProptestConfig {
        cases: 32,
        ..ProptestConfig::default()
    };
    proptest!(config, |(cap in 1usize..16, seed in any::<u32>(), pairs in prop::collection::vec((any::<u32>(), -1000i32..1000), 0..128))| {
        let hash = move |k: u32| (k ^ seed) % (cap as u32);
        let mut m = PerfectHashMap::new(cap, hash);
        let mut sh = Shadow::new(cap);
        for (kraw, v) in pairs.iter().copied() {
            let k = if cap == 0 { 0 } else { kraw % ((cap as u32) * 4) };
            let _ = sh.insert(seed, k, v);
            let _ = m.insert(k, v);
        }
        for (_, v) in m.iter_mut() { *v = v.saturating_add(1); }
        for s in &mut sh.slots { if let Some((_, v)) = s { *v = v.saturating_add(1); } }
        let exp: Vec<(u32, i32)> = sh.slots.iter().filter_map(|e| e.map(|(k,v)| (k,v))).collect();
        let got: Vec<(u32, i32)> = m.iter().map(|(k, v)| (*k, *v)).collect();
        prop_assert_eq!(got, exp);
    });
}
