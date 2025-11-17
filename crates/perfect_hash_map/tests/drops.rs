//! Drop semantics tests: ensure values are dropped exactly once on replace,
//! remove, clear, and map drop.

use std::cell::Cell;
use std::rc::Rc;

use perfect_hash_map::{HashFn, PerfectHashMap};

fn phash(k: u32) -> u32 {
    k % 8
}

#[derive(Debug)]
struct DropCounter(Rc<Cell<usize>>);

impl Drop for DropCounter {
    fn drop(&mut self) {
        self.0.set(self.0.get() + 1);
    }
}

/// Replacement drops the old value exactly once.
#[test]
fn drop_on_replace() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    let count = Rc::new(Cell::new(0));
    let _ = m.insert(1, DropCounter(count.clone()));
    let prev = m.insert(1, DropCounter(count.clone())).unwrap();
    assert!(prev.is_some());
    // Drop the returned previous value now.
    drop(prev);
    assert_eq!(count.get(), 1);
}

/// Removing a present key drops its value exactly once.
#[test]
fn drop_on_remove() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    let count = Rc::new(Cell::new(0));
    let _ = m.insert(1, DropCounter(count.clone()));
    let val = m.remove(&1).unwrap();
    drop(val);
    assert_eq!(count.get(), 1);
}

/// clear() drops all present values.
#[test]
fn drop_on_clear() {
    let mut m = PerfectHashMap::new(8, phash as HashFn);
    let count = Rc::new(Cell::new(0));
    let _ = m.insert(1, DropCounter(count.clone()));
    let _ = m.insert(2, DropCounter(count.clone()));
    m.clear();
    assert_eq!(count.get(), 2);
}

/// Map drop drops remaining values.
#[test]
fn drop_on_map_drop() {
    let count = Rc::new(Cell::new(0));
    {
        let mut m = PerfectHashMap::new(8, phash as HashFn);
        let _ = m.insert(1, DropCounter(count.clone()));
        let _ = m.insert(2, DropCounter(count.clone()));
        assert_eq!(count.get(), 0);
    }
    // Map dropped, both values dropped
    assert_eq!(count.get(), 2);
}
