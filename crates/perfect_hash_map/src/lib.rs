//! PerfectHashMap: a HashMap-like container backed by a generator-provided
//! perfect hash function.
//!
//! Storage is a fixed-size `Vec<Option<Entry<V>>>` with capacity equal to the
//! provided hash size; there are no re-allocations. Keys are `u32` RPC IDs.
//! The perfect hash maps a key to a unique slot in `[0, hash_size)` for the
//! known key set. For safety with unknown keys, each slot stores the key
//! alongside the value, and lookups verify the exact key match. Inserts that
//! would overwrite a different key in the same slot return a `Collision` error
//! (no probing).
//!
//! Differences from `std::collections::HashMap`:
//! - Fixed capacity equal to the perfect-hash size (no growth or rehashing).
//! - `insert` returns a `CollisionError` if a different key occupies the slot.
//! - `try_insert` returns `Occupied` (same key) or `Collision` (different key).
//! - `iter_mut` yields `(Key, &mut V)` (the key by value) to avoid aliasing.
//!
//! Example
//! ```
//! use perfect_hash_map::PerfectHashMap;
//!
//! // A toy "perfect" hash for this example: modulo 8
//! let hash = |k: u32| k % 8;
//! let mut m: PerfectHashMap<&'static str, _> = PerfectHashMap::new(8, hash);
//!
//! assert!(m.is_empty());
//! assert_eq!(m.insert(1, "one"), Ok(None));
//! assert_eq!(m.get(&1), Some(&"one"));
//!
//! // Replacing the same key returns the previous value
//! assert_eq!(m.insert(1, "uno"), Ok(Some("one")));
//! assert_eq!(m.get(&1), Some(&"uno"));
//!
//! // Colliding with a different key (1 and 9 map to the same slot)
//! let err = m.insert(9, "nine").unwrap_err();
//! assert_eq!(err.existing_key, 1);
//! ```

#![deny(missing_docs)]

/// Key type for the map (rpc_id).
pub type Key = u32;

/// Type of a generator-provided perfect hash function.
pub type HashFn = fn(Key) -> u32;

/// An entry stored in a map slot, keeping the key for validation.
#[derive(Debug)]
pub struct Entry<V> {
    /// The rpc_id key placed at this slot.
    pub key: Key,
    /// The value associated with the key.
    pub value: V,
}

/// Error for `insert` when the slot is occupied by a different key.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct CollisionError {
    /// Key already occupying the computed slot.
    pub existing_key: Key,
}

/// A handle to an occupied entry, giving access to the existing value.
pub struct OccupiedEntry<'a, V> {
    value: &'a mut V,
}

impl<'a, V> OccupiedEntry<'a, V> {
    /// Returns an immutable reference to the existing value.
    pub fn get(&self) -> &V {
        self.value
    }

    /// Returns a mutable reference to the existing value.
    pub fn get_mut(&mut self) -> &mut V {
        self.value
    }

    /// Replaces the existing value with `new_value`, returning the old value.
    pub fn replace(&mut self, new_value: V) -> V {
        core::mem::replace(self.value, new_value)
    }
}

/// Error for `try_insert` with more detailed outcomes.
pub enum TryInsertError<'a, V> {
    /// The same key is already present; use the entry to access/modify it.
    Occupied {
        /// The value that could not be inserted.
        value: V,
        /// A handle to the existing entry's value.
        entry: OccupiedEntry<'a, V>,
    },
    /// A different key occupies the slot; insertion is rejected.
    Collision {
        /// The value that could not be inserted.
        value: V,
        /// The key already occupying the target slot.
        existing_key: Key,
    },
}

impl<'a, V> core::fmt::Debug for TryInsertError<'a, V> {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            Self::Occupied { .. } => f.debug_struct("Occupied").finish_non_exhaustive(),
            Self::Collision { existing_key, .. } => f
                .debug_struct("Collision")
                .field("existing_key", existing_key)
                .finish(),
        }
    }
}

/// Fixed-capacity perfect-hash map.
///
/// - `V` is the stored value type.
/// - `F` is a callable that implements `Fn(u32) -> u32` and maps keys to slots
///   in `[0, capacity)`. Typically this is the generated `app_hash` function.
pub struct PerfectHashMap<V, F = HashFn>
where
    F: Fn(Key) -> u32 + Copy,
{
    hash: F,
    slots: Vec<Option<Entry<V>>>,
    len: usize,
}

impl<V, F> PerfectHashMap<V, F>
where
    F: Fn(Key) -> u32 + Copy,
{
    /// Creates a new map with a fixed capacity equal to `hash_size` and using
    /// the provided perfect hash function `hash`.
    ///
    /// Panics
    /// - Methods that write (e.g., `insert`, `try_insert`) will panic if the
    ///   hash function returns a slot `>= hash_size`.
    pub fn new(hash_size: usize, hash: F) -> Self {
        let mut slots = Vec::with_capacity(hash_size);
        slots.resize_with(hash_size, || None);
        Self {
            hash,
            slots,
            len: 0,
        }
    }

    /// Number of key-value pairs in the map.
    pub fn len(&self) -> usize {
        self.len
    }

    /// Returns true if the map contains no elements.
    pub fn is_empty(&self) -> bool {
        self.len == 0
    }

    /// Fixed capacity (equals the provided hash size).
    pub fn capacity(&self) -> usize {
        self.slots.len()
    }

    /// Returns true if the given key is present.
    pub fn contains_key(&self, key: &Key) -> bool {
        self.get(key).is_some()
    }

    /// Returns a shared reference to the value corresponding to the key.
    ///
    /// If the hashed slot is out of range, this returns `None`.
    pub fn get(&self, key: &Key) -> Option<&V> {
        let i = (self.hash)(*key) as usize;
        match self.slots.get(i) {
            Some(Some(e)) if e.key == *key => Some(&e.value),
            _ => None,
        }
    }

    /// Returns a mutable reference to the value corresponding to the key.
    ///
    /// If the hashed slot is out of range, this returns `None`.
    pub fn get_mut(&mut self, key: &Key) -> Option<&mut V> {
        let i = (self.hash)(*key) as usize;
        match self.slots.get_mut(i) {
            Some(Some(e)) if e.key == *key => Some(&mut e.value),
            _ => None,
        }
    }

    /// Removes a key from the map, returning the value if present.
    pub fn remove(&mut self, key: &Key) -> Option<V> {
        let i = (self.hash)(*key) as usize;
        if i >= self.slots.len() {
            return None;
        }
        let match_key = matches!(self.slots[i].as_ref(), Some(e) if e.key == *key);
        if match_key {
            let entry = self.slots[i].take().expect("was Some above");
            self.len -= 1;
            Some(entry.value)
        } else {
            None
        }
    }

    /// Clears the map, removing all key-value pairs.
    pub fn clear(&mut self) {
        for slot in &mut self.slots {
            if slot.is_some() {
                *slot = None;
            }
        }
        self.len = 0;
    }

    /// Inserts a key-value pair.
    ///
    /// Returns `Ok(None)` if the key was not present, or `Ok(Some(old))` if
    /// the key was present and the value was replaced. Returns
    /// `Err(CollisionError)` if the slot is occupied by a different key.
    ///
    /// Panics
    /// - If the hash function returns a slot `>= capacity()`.
    pub fn insert(&mut self, key: Key, value: V) -> Result<Option<V>, CollisionError> {
        let i = (self.hash)(key) as usize;
        match self.slots.get_mut(i) {
            Some(slot) => match slot {
                None => {
                    *slot = Some(Entry { key, value });
                    self.len += 1;
                    Ok(None)
                }
                Some(e) if e.key == key => Ok(Some(core::mem::replace(&mut e.value, value))),
                Some(e) => Err(CollisionError {
                    existing_key: e.key,
                }),
            },
            None => panic!("hash function returned out-of-range slot"),
        }
    }

    /// Attempts to insert a key-value pair.
    ///
    /// On success, returns a mutable reference to the inserted value.
    /// On failure due to same-key occupancy, returns an `Occupied` error with
    /// a handle to the existing value. On different-key collision, returns a
    /// `Collision` error without modifying the map.
    ///
    /// Panics
    /// - If the hash function returns a slot `>= capacity()`.
    pub fn try_insert(&mut self, key: Key, value: V) -> Result<&mut V, TryInsertError<'_, V>> {
        let i = (self.hash)(key) as usize;
        match self.slots.get_mut(i) {
            Some(slot) => {
                if slot.is_none() {
                    *slot = Some(Entry { key, value });
                    self.len += 1;
                    let e = slot.as_mut().unwrap();
                    return Ok(&mut e.value);
                }
                let e = slot.as_mut().unwrap();
                if e.key == key {
                    Err(TryInsertError::Occupied {
                        value,
                        entry: OccupiedEntry {
                            value: &mut e.value,
                        },
                    })
                } else {
                    Err(TryInsertError::Collision {
                        value,
                        existing_key: e.key,
                    })
                }
            }
            None => panic!("hash function returned out-of-range slot"),
        }
    }

    /// Returns an iterator over `(&Key, &V)` pairs.
    ///
    /// Items are produced in slot order (increasing slot index).
    pub fn iter(&self) -> Iter<'_, V> {
        Iter {
            slots: &self.slots,
            pos: 0,
        }
    }

    /// Returns a mutable iterator over `(Key, &mut V)` pairs.
    ///
    /// The key is yielded by value to avoid `(&Key, &mut V)` aliasing.
    pub fn iter_mut(&mut self) -> IterMut<'_, V> {
        IterMut {
            slots: self.slots.as_mut_ptr(),
            len: self.slots.len(),
            pos: 0,
            _marker: core::marker::PhantomData,
        }
    }

    /// Returns an iterator over keys.
    pub fn keys(&self) -> Keys<'_, V> {
        Keys { inner: self.iter() }
    }

    /// Returns an iterator over values.
    pub fn values(&self) -> Values<'_, V> {
        Values { inner: self.iter() }
    }

    /// Returns a mutable iterator over values.
    pub fn values_mut(&mut self) -> ValuesMut<'_, V> {
        ValuesMut {
            inner: self.iter_mut(),
        }
    }
}

/// Immutable iterator yielding `(&Key, &V)`.
pub struct Iter<'a, V> {
    slots: &'a [Option<Entry<V>>],
    pos: usize,
}

impl<'a, V> Iterator for Iter<'a, V> {
    type Item = (&'a Key, &'a V);
    fn next(&mut self) -> Option<Self::Item> {
        while self.pos < self.slots.len() {
            let i = self.pos;
            self.pos += 1;
            if let Some(ref e) = self.slots[i] {
                return Some((&e.key, &e.value));
            }
        }
        None
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        (0, Some(self.slots.len().saturating_sub(self.pos)))
    }
}

use core::marker::PhantomData;

/// Mutable iterator yielding `(Key, &mut V)`.
pub struct IterMut<'a, V> {
    slots: *mut Option<Entry<V>>,
    len: usize,
    pos: usize,
    _marker: PhantomData<&'a mut [Option<Entry<V>>]>,
}

impl<'a, V> Iterator for IterMut<'a, V> {
    type Item = (Key, &'a mut V);
    fn next(&mut self) -> Option<Self::Item> {
        let n = self.len;
        while self.pos < n {
            let i = self.pos;
            self.pos += 1;
            // SAFETY: we yield at most one &mut to any slot across iterations.
            // The iterator holds exclusive &mut access to the entire slice for
            // its lifetime via PhantomData. We never alias two &mut to the
            // same element.
            let slot = unsafe { &mut *self.slots.add(i) };
            if let Some(entry) = slot.as_mut() {
                let k = entry.key;
                let v: *mut V = &mut entry.value;
                return Some((k, unsafe { &mut *v }));
            }
        }
        None
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        (0, Some(self.len.saturating_sub(self.pos)))
    }
}

/// Iterator over keys.
pub struct Keys<'a, V> {
    inner: Iter<'a, V>,
}

impl<'a, V> Iterator for Keys<'a, V> {
    type Item = &'a Key;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(k, _)| k)
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

/// Iterator over values.
pub struct Values<'a, V> {
    inner: Iter<'a, V>,
}

impl<'a, V> Iterator for Values<'a, V> {
    type Item = &'a V;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(_, v)| v)
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

/// Mutable iterator over values.
pub struct ValuesMut<'a, V> {
    inner: IterMut<'a, V>,
}

impl<'a, V> Iterator for ValuesMut<'a, V> {
    type Item = &'a mut V;
    fn next(&mut self) -> Option<Self::Item> {
        self.inner.next().map(|(_, v)| v)
    }
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }
}

// unit tests live in `tests/` to avoid duplication and improve discoverability
