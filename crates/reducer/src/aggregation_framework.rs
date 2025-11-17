//! Aggregation Framework: generic group-by + fold utilities for reducer
//!
//! Goals
//! - Provide a minimal, generic API to aggregate arbitrary items by a key `Range` and fold values into `T`.
//! - Use closures (no traits) for projection and reduction to keep call sites simple and flexible.
//! - Support both a simple one-shot API that returns a map and an advanced API that accumulates into an existing map
//!   with a custom initializer.
//!
//! Core idea
//! - Inputs are arbitrary iterator items `E`.
//! - A projection function maps an item to a key: `project(&E) -> Range`.
//! - An add function folds each item into `T`: `add(&mut T, &E)`.
//! - Aggregation iterates once, grouping by `Range` and reducing into a `HashMap<Range, T>`.
//!
//! Ownership & lifetimes
//! - `project(&E) -> R` should produce an owned `R` (e.g., `String`, `u64`, tuples). Avoid returning references into
//!   ephemeral data inside `E` that won't live past the iteration.
//! - If keys are large or expensive to clone, consider interning or using `Arc<str>`/`Arc<[u8]>` inside `project`.
//!
//! Complexity
//! - Time: expected O(n) with the default hasher.
//! - Memory: proportional to the number of distinct `Range` keys.
//! - Iteration order of the returned map is unspecified.
//!
//! Example
//! ```
//! use std::collections::HashMap;
//! use reducer::aggregation_framework::aggregate;
//!
//! // Count by parity of u8
//! let data: Vec<(u8, ())> = vec![(1,()), (2,()), (4,()), (5,())];
//! let project = |(d, _s): &(u8, ())| d % 2;        // key: even/odd
//! let add = |t: &mut u64, _item: &(u8, ())| *t += 1; // count occurrences
//! let by_parity: HashMap<u8, u64> = aggregate(data, project, add);
//! assert_eq!(by_parity.get(&0), Some(&2)); // 2 and 4
//! assert_eq!(by_parity.get(&1), Some(&2)); // 1 and 5
//! ```

use std::collections::HashMap;
use std::hash::{BuildHasher, Hash};

/// Advanced aggregation: accumulate into an existing map with a custom initializer.
///
/// - `dest`: destination map to accumulate into.
/// - `iter`: items to aggregate. Consumed exactly once.
/// - `project`: maps each item `E` to an owned key `R`.
/// - `init`: constructs a fresh `T` when a new key is encountered.
/// - `add`: folds each item into the corresponding `T`.
pub fn aggregate_with<I, E, R, T, P, F, A, H>(
    dest: &mut HashMap<R, T, H>,
    iter: I,
    mut project: P,
    mut init: F,
    mut add: A,
) where
    I: IntoIterator<Item = E>,
    P: FnMut(&E) -> R,
    F: FnMut() -> T,
    A: FnMut(&mut T, &E),
    R: Eq + Hash,
    H: BuildHasher,
{
    let it = iter.into_iter();
    let (lower, _) = it.size_hint();
    dest.reserve(lower);
    for item in it {
        let r = project(&item);
        let entry = dest.entry(r).or_insert_with(&mut init);
        add(entry, &item);
    }
}

/// Simple aggregation: one-shot group-by + fold that returns a fresh `HashMap`.
///
/// This is sugar over `aggregate_with`, using `T: Default` for initialization and the default hasher.
pub fn aggregate<I, E, R, T, P, A>(iter: I, project: P, add: A) -> HashMap<R, T>
where
    I: IntoIterator<Item = E>,
    P: FnMut(&E) -> R,
    A: FnMut(&mut T, &E),
    R: Eq + Hash,
    T: Default,
{
    let inner = iter.into_iter();
    let (lower, _) = inner.size_hint();
    let mut map: HashMap<R, T> = HashMap::with_capacity(lower);
    aggregate_with(&mut map, inner, project, T::default, add);
    map
}

#[cfg(test)]
mod tests {
    use super::*;
    use proptest::collection::vec;
    use proptest::prelude::*;

    #[test]
    fn empty_iterator_returns_empty_map() {
        let data: Vec<(u32, u32)> = vec![];
        let m: HashMap<u32, u64> = aggregate(
            data,
            |(d, _s): &(u32, u32)| *d,
            |t: &mut u64, (_d, s): &(u32, u32)| *t += *s as u64,
        );
        assert!(m.is_empty());
    }

    #[test]
    fn single_key_all_elements_aggregated() {
        let data: Vec<(u8, u16)> = (0..10).map(|i| (i, 1u16)).collect();
        let m: HashMap<u8, u64> =
            aggregate(data, |_item: &(u8, u16)| 0u8, |t, (_d, s)| *t += *s as u64);
        assert_eq!(m.len(), 1);
        assert_eq!(m.get(&0u8), Some(&10u64));
    }

    #[test]
    fn distinct_keys_each_once() {
        let data: Vec<(u32, u16)> = (0..100).map(|i| (i, 1u16)).collect();
        let m: HashMap<u32, u64> = aggregate(
            data,
            |(d, _s): &(u32, u16)| *d,
            |t, (_d, s)| *t += *s as u64,
        );
        assert_eq!(m.len(), 100);
        for i in 0u32..100 {
            assert_eq!(m.get(&i), Some(&1u64));
        }
    }

    #[test]
    fn aggregate_with_custom_init_non_default_type() {
        #[derive(Debug, PartialEq, Eq)]
        struct NonDefault {
            sum: u64,
            flag: bool,
        }

        let data = vec![(1u8, 5u16), (1u8, 7u16), (2u8, 10u16)];
        let mut dest: HashMap<u8, NonDefault> = HashMap::new();
        aggregate_with(
            &mut dest,
            data,
            |(d, _s): &(u8, u16)| *d,
            || NonDefault { sum: 1, flag: true }, // custom init
            |t: &mut NonDefault, (_d, s): &(u8, u16)| t.sum += *s as u64,
        );

        assert_eq!(dest.len(), 2);
        assert_eq!(
            dest.get(&1u8),
            Some(&NonDefault {
                sum: 1 + 5 + 7,
                flag: true
            })
        );
        assert_eq!(
            dest.get(&2u8),
            Some(&NonDefault {
                sum: 1 + 10,
                flag: true
            })
        );
    }

    proptest! {
        #[test]
        fn proptest_equivalence_to_naive_fold(values in vec((any::<u8>(), any::<u16>()), 0..200), k in 1u8..=8u8) {
            // naive fold
            let mut naive: HashMap<u8, u64> = HashMap::new();
            for (d, s) in &values {
                let key = d % k;
                *naive.entry(key).or_insert(0) += *s as u64;
            }

            // aggregate helper
            let project = |(d, _s): &(u8, u16)| d % k;
            let add = |t: &mut u64, (_d, s): &(u8, u16)| *t += *s as u64;
            let got: HashMap<u8, u64> = aggregate(values.clone(), project, add);

            prop_assert_eq!(got, naive);
        }

        #[test]
        fn proptest_chunked_aggregation_matches_one_shot(values in vec((any::<u8>(), any::<u16>()), 0..200), k in 1u8..=8u8) {
            let mid = values.len().saturating_div(2);
            let (left, right) = values.split_at(mid);

            let mut dest: HashMap<u8, u64> = HashMap::new();
            let project = |(d, _s): &(u8, u16)| d % k;
            let add = |t: &mut u64, (_d, s): &(u8, u16)| *t += *s as u64;

            aggregate_with(&mut dest, left.to_vec(), project, || 0u64, add);
            aggregate_with(&mut dest, right.to_vec(), |(d, _s): &(u8, u16)| d % k, || 0u64, |t, (_d, s): &(u8, u16)| *t += *s as u64);

            // one-shot
            let one_shot: HashMap<u8, u64> = aggregate(values.clone(), |(d, _s): &(u8, u16)| d % k, |t, (_d, s): &(u8, u16)| *t += *s as u64);

            prop_assert_eq!(dest, one_shot);
        }
    }
}
