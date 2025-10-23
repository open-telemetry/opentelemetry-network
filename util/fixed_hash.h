/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/platform.h>
#include <util/iterable_bitmap.h>
#include <util/pool.h>

#ifndef NDEBUG_SANITIZER
#include <absl/container/flat_hash_map.h>
#else
#include <unordered_map>
#endif

#include <cstddef>
#include <limits>
#include <memory>
#include <functional>
#include <string.h>
#include <type_traits>

template <
    class Key,
    class T,
    std::size_t ELEM_POOL_SZ,
    class Hash,
    class KeyEqual = std::equal_to<Key>,
    class Allocator = std::allocator<T>>
class FixedHash {
public:
  using key_type = Key;
  using value_type = T;
  using pool_type = Pool<value_type, ELEM_POOL_SZ, Allocator>;
  using index_type = typename pool_type::index_type;
  using size_type = std::size_t;
  // Use default allocator for the map to avoid allocator rebind issues with Abseil.
#ifndef NDEBUG_SANITIZER
  using map_type = absl::flat_hash_map<key_type, index_type, Hash, KeyEqual>;
#else
  using map_type = std::unordered_map<key_type, index_type, Hash, KeyEqual>;
#endif
  using bitmap_type = typename pool_type::bitmap_type;
  using position = typename pool_type::position;
  static constexpr index_type invalid = pool_type::invalid;

  /**
   * c'tor
   */
  FixedHash() {}

  bool empty() const { return pool_.empty(); }
  bool full() const { return pool_.full(); }
  size_type size() const { return pool_.size(); }
  size_type max_size() const { return pool_.max_size(); }
  size_type capacity() const { return pool_.capacity(); }
  value_type const &operator[](index_type index) const { return pool_[index]; }
  value_type &operator[](index_type index) { return pool_[index]; }
  bitmap_type allocated() const { return pool_.allocated(); }

  template <typename K> bool contains(const K &key) const { return map_.count(key) == 1; }

  /**
   * Finds the given element.
   * @returns: the index of the element, or {invalid,nullptr} if not found
   */
  template <typename K> position find(const K &key)
  {
    auto it = map_.find(key);
    if (it == map_.end())
      return {invalid, nullptr};

    /* get the found entry */
    u32 index = it->second;
    return {(index_type)index, (value_type *)&pool_[index]};
  }

  /**
   * Inserts the value into the hash with given key.
   * @returns position of the new value, or {invalid,nullptr} if key exists or
   *  the container is full.
   */
  template <typename K, typename... Args> position insert(K &&key, Args &&... args)
  {
    if (full())
      return {invalid, nullptr};

    /* add to the hash */
    auto it = map_.insert({key, -1});
    if (!it.second)
      return {invalid, nullptr};
    bool disarm = false;
    DisarmGuard map_guard(disarm, [this, &key] { map_.erase(key); });

    /* add the object to the pool, might throw! */
    auto pos = pool_.emplace(std::forward<Args>(args)...);
    if (pos.index == invalid)
      return {invalid, nullptr}; /* map_guard will free */

    /* okay, we're good! */
    it.first->second = pos.index; /* save the index in the map */
    disarm = true;                /* don't want to deallocate */
    return pos;
  }

  /**
   * Erase the value pointed to by key
   * @return true on success, false if key not found
   */
  template <typename K> bool erase(const K &key)
  {
    /* find the key */
    auto it = map_.find(key);
    if (it == map_.end())
      return false;

    u32 index = it->second;

    /* remove from pool */
    pool_.remove(index);
    /* remove from hash */
    map_.erase(key);

    return true;
  }

  typename map_type::const_iterator begin() const { return map_.begin(); }
  typename map_type::const_iterator end() const { return map_.end(); }

  struct value_iterator {
    value_iterator(typename map_type::const_iterator i, pool_type &pool) : i_(i), pool_(pool) {}

    value_iterator &operator++()
    {
      ++i_;
      return *this;
    }

    value_iterator operator++(int)
    {
      auto copy = *this;
      ++*this;
      return copy;
    }

    value_type const *operator->() const { return &pool_[i_->second]; }
    value_type *operator->() { return &pool_[i_->second]; }

    value_type const &operator*() const { return pool_[i_->second]; }
    value_type &operator*() { return pool_[i_->second]; }

    bool operator==(value_iterator const &rhs) const { return i_ == rhs.i_; }
    bool operator!=(value_iterator const &rhs) const { return i_ != rhs.i_; }

  private:
    typename map_type::const_iterator i_;
    pool_type &pool_;
  };

  struct values_iterable {
    values_iterable(typename map_type::const_iterator begin, typename map_type::const_iterator end, pool_type &pool)
        : begin_(begin), end_(end), pool_(pool)
    {}

    value_iterator begin() { return {begin_, pool_}; }
    value_iterator end() { return {end_, pool_}; }

  private:
    typename map_type::const_iterator begin_;
    typename map_type::const_iterator end_;
    pool_type &pool_;
  };

  values_iterable values() { return {map_.begin(), map_.end(), pool_}; }

private:
  struct DisarmGuard {
    template <typename F> DisarmGuard(bool &disarm_b, F fn) : fn_(fn), disarm_(disarm_b) {}
    ~DisarmGuard()
    {
      if (!disarm_)
        fn_();
    }

  private:
    std::function<void(void)> fn_;
    bool &disarm_;
  };

  map_type map_;
  pool_type pool_;
};
