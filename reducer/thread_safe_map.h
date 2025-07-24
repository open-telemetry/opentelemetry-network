/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ThreadSafeMap is an associative map data structure that allows for quick
// thread-safe lookups and inserts. Only a handful of operations are supported,
// and the inserted values cannot be mutated.
//
// NOTE: By default this class uses abseil's mutexes. These are fast, but
// don't have ThreadSanitizer support by default. If you wish to use thread
// sanitization and don't care about the performance hit, just compile with
// `-DUSE_STL_MUTEX`.

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/synchronization/mutex.h>

#include <cstdlib>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

template <typename Key, typename Value> class ThreadSafeMap {
public:
  // TODO(dandrino): Add customization in how the underlying values are stored
  // if that becomes a feature that we need.

  // `bucket_count` specifies how many buckets to use.
  explicit ThreadSafeMap(std::size_t bucket_count = kDefaultBucketCount);

  // Looks up the value for the given key and returns it as a shared_ptr. In
  // the even that the underlying map entry has been overwritten by another
  // thread, outstanding references to this value will still be valid.
  std::shared_ptr<const Value> get(const Key &key) const;

  // Inserts the given value into the map for the provided key. If an entry
  // already exists for the given key, will overwrite it. Returns the
  // shared_ptr that would be returned by calling `get` on the same key.
  std::shared_ptr<const Value> insert(const Key &key, Value value);

  // Looks up the value for the given key. If none exists, inserts the value
  // created using the provided function. Returns a shared_key to the value.
  std::shared_ptr<const Value> get_or_insert(const Key &key, std::function<Value()> value_func);

  // Removes the value associated with the provided key from the map.
  // Returns true if removed, false otherwise.
  bool erase(const Key &key);

  // Writes the contents of this map to the provided STL-map-like data
  // structure. There is no guaranteed that the populated values are the same
  // as those in this class at the moment of invocation.
  // NOTE: This is sort of a poor man's alternative to adding full iterator
  // support, and primarily exists now for debugging purposes.
  template <typename Map> void write_to(Map *out) const;

private:
  // A bucket is a subpartition of the map space that contend for the same
  // mutex lock.
  struct Bucket {
    absl::flat_hash_map<Key, std::shared_ptr<Value>> map;
#ifdef USE_STL_MUTEX
    mutable std::shared_mutex mu;
#else
    mutable absl::Mutex mu;
#endif
  };

  // Returns the bucket for the given key.
  Bucket *get_bucket(const Key &key);
  const Bucket *get_bucket(const Key &key) const { return const_cast<ThreadSafeMap<Key, Value> *>(this)->get_bucket(key); }

  // 256 ought to be enough for anybody.
  static const std::size_t kDefaultBucketCount = 256;

  std::vector<Bucket> buckets_;

  const absl::Hash<Key> hasher_ = {};
};

// Implementation below.

template <typename Key, typename Value>
ThreadSafeMap<Key, Value>::ThreadSafeMap(const std::size_t bucket_count) : buckets_(bucket_count)
{}

template <typename Key, typename Value> std::shared_ptr<const Value> ThreadSafeMap<Key, Value>::get(const Key &key) const
{
  const Bucket *const bucket = get_bucket(key);
#ifdef USE_STL_MUTEX
  std::shared_lock l(bucket->mu);
#else
  absl::ReaderMutexLock l(&bucket->mu);
#endif

  auto it = bucket->map.find(key);
  if (it == bucket->map.end())
    return {}; // Not found

  return it->second;
}

template <typename Key, typename Value>
std::shared_ptr<const Value> ThreadSafeMap<Key, Value>::insert(const Key &key, Value value)
{
  auto inserted_value = std::make_shared<Value>(std::move(value));

  Bucket *const bucket = get_bucket(key);
#ifdef USE_STL_MUTEX
  std::unique_lock l(bucket->mu);
#else
  absl::MutexLock l(&bucket->mu);
#endif

  bucket->map[key] = inserted_value;
  return inserted_value;
}

template <typename Key, typename Value>
std::shared_ptr<const Value> ThreadSafeMap<Key, Value>::get_or_insert(const Key &key, std::function<Value()> value_func)
{
  Bucket *const bucket = get_bucket(key);
#ifdef USE_STL_MUTEX
  std::unique_lock l(bucket->mu);
#else
  absl::MutexLock l(&bucket->mu);
#endif

  auto it = bucket->map.find(key);
  if (it != bucket->map.end()) {
    return it->second;
  } else {
    auto inserted_value = std::make_shared<Value>(value_func());
    bucket->map[key] = inserted_value;
    return inserted_value;
  }
}

template <typename Key, typename Value> bool ThreadSafeMap<Key, Value>::erase(const Key &key)
{
  Bucket *const bucket = get_bucket(key);
#ifdef USE_STL_MUTEX
  std::unique_lock l(bucket->mu);
#else
  absl::MutexLock l(&bucket->mu);
#endif

  if (bucket->map.erase(key) > 0) {
    return true;
  } else {
    return false;
  }
}

template <typename Key, typename Value>
typename ThreadSafeMap<Key, Value>::Bucket *ThreadSafeMap<Key, Value>::get_bucket(const Key &key)
{
  const std::size_t index = hasher_(key) % buckets_.size();
  return &buckets_[index];
}

template <typename Key, typename Value> template <typename Map> void ThreadSafeMap<Key, Value>::write_to(Map *const out) const
{
  for (const Bucket &bucket : buckets_) {
#ifdef USE_STL_MUTEX
    std::shared_lock l(bucket.mu);
#else
    absl::ReaderMutexLock l(&bucket.mu);
#endif
    std::copy(bucket.map.begin(), bucket.map.end(), std::inserter(*out, out->end()));
  }
}
