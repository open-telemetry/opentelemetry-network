/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/platform.h>
#include <util/iterable_bitmap.h>
#include <util/pool_allocator.h>

#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>

template <class T, std::size_t SIZE, class Allocator = std::allocator<T>> class Pool {
public:
  using index_type = typename std::conditional<(SIZE >= (1 << 16) - 1), u32, u16>::type;
  using element_type = T;
  using size_type = std::size_t;
  using bitmap_type = IterableBitmap<SIZE>;

  static constexpr size_type pool_size = SIZE;
  static constexpr index_type invalid = std::numeric_limits<index_type>::max();

private:
  /* force the used storage to at least hold a u32 */
  union poolable_type {
    element_type t;
    u32 __pool_allocator_bytes_when_destroyed;
  };

  /* POD type suitable for use as uninitialized storage */
  using storage_type = typename std::aligned_storage<sizeof(poolable_type), alignof(poolable_type)>::type;

  /* allocator traits rebound to the storage type */
  using allocator_traits = typename std::allocator_traits<Allocator>::template rebind_traits<storage_type>;

public:
  using allocator_type = typename allocator_traits::allocator_type;

  struct position {
    index_type index;
    element_type *entry;
  };

  /**
   * c'tor
   */
  Pool()
  {
    /* check pool allocator requirements */
    static_assert(pool_size > 0, "pool size must be larger than 0");

    /* allocate storage */
    elements_ = allocator_traits::allocate(allocator_, pool_size);

    /* initialize the pool allocator (does not throw) */
    pool_allocator_init(&pool_alloc_, &elements_[0], sizeof(poolable_type), pool_size);
  }

  ~Pool()
  {
    for (auto i : allocated_) {
      destroy(i);
    }

    allocator_traits::deallocate(allocator_, elements_, pool_size);
  }

  bool empty() const { return size() == 0; }

  bool full() const { return size() == capacity(); }

  size_type size() const { return elem_count_; }

  size_type max_size() const { return max_elem_count_; }

  size_type capacity() const { return pool_size; }

  const bitmap_type &allocated() const { return allocated_; }

  element_type &operator[](index_type index)
  {
    assert(index < pool_size);
    assert(allocated_.get(index));
    return *(element_type *)&elements_[index];
  }

  element_type const &operator[](index_type index) const
  {
    assert(index < pool_size);
    assert(allocated_.get(index));
    return *(element_type *)&elements_[index];
  }

  /**
   * Returns the index of the element
   * @assumes elem is in the pool and was returned from emplace->entry
   */
  size_type index_of(element_type *elem)
  {
    /* just sanity checks */
    assert((u64)elem >= (u64)&elements_[0]);
    assert((u64)elem < (u64)&elements_[0] + sizeof(elements_));

    return (poolable_type *)elem - (poolable_type *)&elements_[0];
  }

  /**
   * Emplaces an element into the pool.
   * @returns position of the new value, or {invalid,nullptr} if the container
   *   is full.
   */
  template <typename... Args> position emplace(Args &&... args)
  {
    if (full())
      return {invalid, nullptr};

    /* allocate an element from the pool */
    u32 index = pool_allocator_alloc(&pool_alloc_);
    assert(index != ~0u);
    bool disarm = false;
    DisarmGuard pool_guard(disarm, [index, this] { pool_allocator_free(&pool_alloc_, index); });

    /* construct the object, might throw! */
    allocator_traits::construct(allocator_, (element_type *)&elements_[index], std::forward<Args>(args)...);

    /* okay, we're good! */
    elem_count_++;
    allocated_.set(index);
    disarm = true; /* don't want to deallocate */

    max_elem_count_ = std::max(max_elem_count_, elem_count_);

    return {(index_type)index, (element_type *)&elements_[index]};
  }

  /**
   * Erase the value pointed to by key
   * @return true on success, false if key not found
   */
  void remove(index_type index)
  {
    assert(index < pool_size);
    assert(allocated_.get(index));

    /* call destructor */
    destroy(index);
    /* reclaim the element into the pool */
    pool_allocator_free(&pool_alloc_, index);

    elem_count_--;
    allocated_.clear(index);
  }

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

  allocator_type allocator_;

  storage_type *elements_;

  bitmap_type allocated_;

  pool_allocator pool_alloc_;

  size_type elem_count_{0};
  size_type max_elem_count_{0};

  /**
   * Destroys the element at @index.
   */
  void destroy(index_type index) { allocator_traits::destroy(allocator_, (T *)&elements_[(u32)index]); }
};
