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

#include <platform/platform.h>
#include <util/iterable_bitmap.h>

#include <cstddef>
#include <memory>
#include <type_traits>

// Simple array-like container in which elements are constructed only when
// they are first accessed.
//
template <typename T, std::size_t SIZE, class Allocator = std::allocator<T>> class LazyArray {
public:
  static_assert(std::is_default_constructible<T>::value);

  using value_type = T;
  using size_type = std::size_t;

  static constexpr size_type size = SIZE;

  LazyArray(LazyArray const &) = delete;
  LazyArray(LazyArray &&) = delete;
  LazyArray &operator=(LazyArray const &) = delete;
  LazyArray &operator=(LazyArray &&) = delete;

private:
  /* POD type suitable for use as uninitialized storage */
  using storage_type = typename std::aligned_storage<sizeof(value_type), alignof(value_type)>::type;

  /* allocator traits rebound to the storage type */
  using allocator_traits = typename std::allocator_traits<Allocator>::template rebind_traits<storage_type>;

public:
  LazyArray() { array_ = allocator_traits::allocate(allocator_, size); }

  ~LazyArray()
  {
    for (auto i : constructed_) {
      allocator_traits::destroy(allocator_, (value_type *)&array_[i]);
    }

    allocator_traits::deallocate(allocator_, array_, size);
  }

  value_type &operator[](size_type i)
  {
    if (!constructed_.get(i)) {
      allocator_traits::construct(allocator_, (value_type *)&array_[i]);
      constructed_.set(i);
    }

    return *(value_type *)&array_[i];
  }

  value_type const &operator[](size_type i) const
  {
    if (!constructed_.get(i)) {
      allocator_traits::construct(allocator_, (value_type *)&array_[i]);
      constructed_.set(i);
    }

    return *(value_type const *)&array_[i];
  }

private:
  using allocator_type = typename allocator_traits::allocator_type;
  using bitmap_type = IterableBitmap<SIZE>;

  allocator_type allocator_;

  mutable bitmap_type constructed_;

  storage_type *array_;
};
