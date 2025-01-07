/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <set>

#include "absl/base/thread_annotations.h"
#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>

#include <absl/types/span.h>

// LoadBalancer helps associate integer load values with a list of elements.
// Each element is initialized to a load of zero, and its load can be
// incremented or decremented as needed, with the least-loaded element being
// queryable. This class works best if type T is inexpensive to copy.
// This class is thread-safe.
template <typename T> class LoadBalancer {
public:
  // `elems` is the list of elements across which this class will load
  // balance. For best performance, T should be cheap to copy.
  explicit LoadBalancer(absl::Span<const T> elems);

  // Returns a copy to the least loaded element. If there is a tie, one
  // of the elements is returned, but it is undefined which one will be.
  // Runs in O(1) time.
  T least_loaded() const;

  // Updates the load of `elem` by `load_delta`. `elem` must have been
  // provided in the constructor. Runs in O(log n) time.
  void increment_load(const T &elem, int load_delta);

private:
  struct Entry {
    T elem;
    int load = 0;
    bool operator<(const Entry &rhs) const;
  };

  using EntrySet = std::multiset<Entry>;
  using EntryIterator = typename EntrySet::const_iterator;

  EntrySet entries_ ABSL_GUARDED_BY(mu_);
  absl::flat_hash_map<T, EntryIterator> elem_to_entry_it_ ABSL_GUARDED_BY(mu_);
  mutable absl::Mutex mu_;
};

// Implementation below.

template <typename T> LoadBalancer<T>::LoadBalancer(const absl::Span<const T> elems)
{
  assert(!elems.empty());
  absl::MutexLock l(&mu_);
  for (const T &elem : elems) {
    auto entry_it = entries_.insert(Entry{.elem = elem});
    elem_to_entry_it_[elem] = entry_it;
  }
}

template <typename T> T LoadBalancer<T>::least_loaded() const
{
  absl::ReaderMutexLock l(&mu_);
  return entries_.begin()->elem;
}

template <typename T> void LoadBalancer<T>::increment_load(const T &elem, const int load_delta)
{
  absl::MutexLock l(&mu_);

  // Get the iterator in `entries_` corresponding to `elem` (which must have
  // been provided to the constructor).
  auto entry_it_it = elem_to_entry_it_.find(elem);
  assert(entry_it_it != elem_to_entry_it_.end());
  const EntryIterator entry_it = entry_it_it->second;

  // Create the new entry with the updated load.
  Entry new_entry{
      .elem = elem,
      .load = entry_it->load + load_delta,
  };

  // Remove the old entry and add the new one.
  entries_.erase(entry_it);
  const EntryIterator new_entry_it = entries_.insert(std::move(new_entry));

  // Re-associate `elem` to the new entry.
  entry_it_it->second = new_entry_it;
}

template <typename T> bool LoadBalancer<T>::Entry::operator<(const Entry &rhs) const
{
  return load < rhs.load;
}
