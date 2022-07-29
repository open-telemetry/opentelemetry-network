/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <platform/platform.h>
#include <tuple>
#include <type_traits>
#include <util/iterable_bitmap.h>

template <class T, std::size_t HASH_SIZE, typename HASH_FN, class Allocator = std::allocator<T>> class PerfectHash {
public:
  using key_type = u32;
  using index_type = u32;
  using value_type = std::pair<const key_type, T>;

  PerfectHash() : elem_count_(0) {}

  ~PerfectHash()
  {
    for (auto i : allocated_) {
      destroy(i);
    }
  }

  T *find(key_type k)
  {
    index_type index = hash_fn(k);

    if (!allocated_.get(index))
      return nullptr;

    if (((value_type *)&values_[index])->first != k)
      return nullptr;

    return &((value_type *)&values_[index])->second;
  }

  template <typename... Args> T *insert(key_type k, Args &&... args)
  {
    index_type index = hash_fn(k);

    /* is the entry present? */
    if (allocated_.get(index))
      return nullptr;

    /* construct the object, might throw! */
    traits_::construct(
        allocator_,
        (value_type *)&values_[index],
        std::piecewise_construct,
        std::forward_as_tuple(std::forward<key_type>(k)),
        std::forward_as_tuple(std::forward<Args>(args)...));

    /* okay, we're good! */
    elem_count_++;
    allocated_.set(index);

    return &((value_type *)&values_[index])->second;
  }

  bool erase(key_type k)
  {
    index_type index = hash_fn(k);

    /* is the entry missing? */
    if (!allocated_.get(index))
      return false;

    /* does the key match? */
    if (((value_type *)&values_[index])->first != k)
      return false;

    /* ok, can free */
    destroy(index);
    elem_count_--;
    allocated_.clear(index);
    return true;
  }

private:
  using traits_ = typename std::allocator_traits<Allocator>::template rebind_traits<value_type>;
  using allocator_type = typename traits_::allocator_type;

  /**
   * Destroys the element at @index.
   */
  void destroy(index_type index) { traits_::destroy(allocator_, &values_[index]); }

  HASH_FN hash_fn;

  allocator_type allocator_;

  std::array<typename std::aligned_storage<sizeof(value_type), alignof(value_type)>::type, HASH_SIZE> values_;

  IterableBitmap<HASH_SIZE> allocated_;

  u32 elem_count_;
};
