//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <ccan/list/list.h>
#include <cstddef>
#include <memory>
#include <platform/platform.h>
#include <util/fixed_hash.h>

template <
    class Key,
    class T,
    std::size_t ELEM_POOL_SZ,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>,
    class Allocator = std::allocator<T>>
class LRU {
public:
  using key_type = Key;
  struct value_type {
    template <typename K, typename... Args>
    value_type(K &&k, Args &&... args) : key(std::forward<K>(k)), node{}, value(std::forward<Args>(args)...)
    {}
    key_type key;
    struct list_node node;
    T value;
  };
  using map_type = FixedHash<key_type, value_type, ELEM_POOL_SZ, Hash, KeyEqual, Allocator>;
  using size_type = typename map_type::size_type;

  static constexpr typename map_type::index_type invalid = map_type::invalid;

public:
  /**
   * c'tor
   */
  LRU()
  {
    /* initialize the head */
    list_head_init(&lru_);
  }

  bool empty() const { return map_.empty(); }

  bool full() const { return map_.full(); }

  size_type size() const { return map_.size(); }

  size_type capacity() const { return map_.capacity(); }

  template <typename K> bool contains(const K &key) const { return map_.contains(key); }

  /**
   * Finds the given element.
   * @returns: pointer to the element, or nullptr if not found
   */
  template <typename K> T *find(const K &key)
  {
    auto pos = map_.find(key);
    if (pos.index == invalid)
      return nullptr;

    /* used; update LRU */
    list_del_from(&lru_, &pos.entry->node);
    list_add_tail(&lru_, &pos.entry->node);
    return &pos.entry->value;
  }

  /**
   * Inserts the value into the LRU with given key.
   * @returns the inserted value, or nullptr if the value exists
   */
  template <typename K, typename... Args> T *insert(K &&key, Args &&... args)
  {
    if (full()) {
      /* evict an entry */
      struct value_type *entry = list_pop(&lru_, struct value_type, node);
      map_.erase(entry->key);
    }

    auto pos = map_.insert(key, value_type{key, std::forward<Args>(args)...});

    if (pos.index == map_.invalid)
      return nullptr;

    list_add_tail(&lru_, &pos.entry->node);
    return &pos.entry->value;
  }

  /**
   * Removes the given element if it exists in the LRU
   */
  template <typename K> void remove(const K &key)
  {
    auto pos = map_.find(key);
    if (pos.index == invalid)
      return;

    list_del(&pos.entry->node);
    map_.erase(key);
    return;
  }

private:
  /* map sk -> table_index */
  map_type map_;

  struct list_head lru_;
};
