//! Deterministic unit tests for PerfectHashMap.
//! These cover construction, basic operations, collisions, clear, queries,
//! iterators, out-of-range panics, and generic hash closure usage.

use perfect_hash_map::{HashFn, PerfectHashMap, TryInsertError};

fn phash(k: u32) -> u32 {
    k % 8
}

/// Validates new map state, capacity and emptiness.
#[test]
fn new_and_capacity() {
    let m = PerfectHashMap::<i32, HashFn>::new(8, phash as HashFn);
    assert!(m.is_empty());
    assert_eq!(m.capacity(), 8);
    assert_eq!(m.len(), 0);
}

/// Basic insert, get, replace, and remove lifecycle.
#[test]
fn basic_insert_get_update_remove() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    assert_eq!(m.insert(1, "one"), Ok(None));
    assert!(m.contains_key(&1));
    assert_eq!(m.len(), 1);
    assert_eq!(m.get(&1), Some(&"one"));
    assert_eq!(m.insert(1, "uno"), Ok(Some("one")));
    assert_eq!(m.get(&1), Some(&"uno"));
    assert_eq!(m.remove(&1), Some("uno"));
    assert!(!m.contains_key(&1));
    assert_eq!(m.len(), 0);
}

/// Validates collision handling between different keys mapping to the same slot.
#[test]
fn collision_handling() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    // keys 1 and 9 collide (1 % 8 == 1, 9 % 8 == 1)
    assert_eq!(m.insert(1, "one"), Ok(None));
    let err = m.insert(9, "nine").unwrap_err();
    assert_eq!(err.existing_key, 1);
    match m.try_insert(9, "nine") {
        Err(TryInsertError::Collision { existing_key, .. }) => assert_eq!(existing_key, 1),
        _ => panic!("expected collision"),
    }
    // same-key occupied: try_insert should return Occupied with access to entry
    match m.try_insert(1, "uno") {
        Err(TryInsertError::Occupied { mut entry, .. }) => {
            assert_eq!(entry.replace("eins"), "one");
            assert_eq!(m.get(&1), Some(&"eins"));
        }
        _ => panic!("expected occupied"),
    }
}

/// clear() empties the map and resets counters; subsequent inserts behave as from empty.
#[test]
fn clear_empties_map() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    for k in [0u32, 2, 4] {
        let _ = m.insert(k, k * 10);
    }
    assert_eq!(m.len(), 3);
    m.clear();
    assert!(m.is_empty());
    for k in [0u32, 2, 4] {
        assert!(!m.contains_key(&k));
    }
    assert_eq!(m.insert(2, 42), Ok(None));
    assert_eq!(m.get(&2), Some(&42));
}

/// contains_key/get/get_mut reflect presence and allow in-place mutation.
#[test]
fn queries_and_get_mut() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    assert_eq!(m.get(&3), None);
    assert_eq!(m.insert(3, 7), Ok(None));
    assert!(m.contains_key(&3));
    if let Some(v) = m.get_mut(&3) {
        *v += 1;
    }
    assert_eq!(m.get(&3), Some(&8));
    // a different key hashing to the same slot should not be visible
    assert_eq!(m.get(&11), None); // 11 % 8 == 3
}

/// Iterator correctness and mutation via iter_mut.
#[test]
fn iterators() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    for k in [0u32, 2, 4] {
        let _ = m.insert(k, k * 10);
    }
    // iter yields all pairs
    let mut kv: Vec<(u32, u32)> = m.iter().map(|(k, v)| (*k, *v)).collect();
    kv.sort_by_key(|x| x.0);
    assert_eq!(kv, vec![(0, 0), (2, 20), (4, 40)]);
    // iter_mut allows mutation
    for (k, v) in m.iter_mut() {
        if k == 2 {
            *v = 99;
        }
    }
    assert_eq!(m.get(&2), Some(&99));
    // keys() and values() projections
    let keys: Vec<u32> = m.keys().copied().collect();
    assert!(keys.contains(&0) && keys.contains(&2) && keys.contains(&4));
    let values: Vec<u32> = m.values().copied().collect();
    assert!(values.contains(&0) && values.contains(&99) && values.contains(&40));
}

/// Insert/try_insert panic on out-of-range hash; get/remove return None.
#[test]
#[should_panic]
fn insert_panics_on_out_of_range_hash() {
    let mut m = PerfectHashMap::new(0, |_k| 0);
    let _ = m.insert(0, 1); // panics: slot out-of-range
}

/// try_insert panic path for out-of-range hash.
#[test]
#[should_panic]
fn try_insert_panics_on_out_of_range_hash() {
    let mut m = PerfectHashMap::new(0, |_k| 0);
    let _ = m.try_insert(0, 1);
}

/// get/remove do not panic for out-of-range hash and return None.
#[test]
fn get_remove_out_of_range_hash() {
    let mut m: PerfectHashMap<i32, _> = PerfectHashMap::new(0, |_k| 0);
    assert_eq!(m.get(&0), None);
    assert_eq!(m.remove(&0), None);
}

/// Generic hash function via a closure capturing a seed.
#[test]
fn generic_hash_closure() {
    let cap = 8usize;
    let seed: u32 = 7;
    let h = move |k: u32| (k ^ seed) % (cap as u32);
    let mut m = PerfectHashMap::new(cap, h);
    assert_eq!(m.insert(1, 10), Ok(None));
    assert_eq!(m.get(&1), Some(&10));
}
