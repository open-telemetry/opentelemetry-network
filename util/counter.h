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

#include <optional>
#include <utility>

#include <cassert>

namespace data {

template <typename T> struct Counter {
  Counter() = default;
  Counter(Counter const &) = default;
  Counter(Counter &&) = default;

  Counter(T data) : value_(std::move(data)) {}

  Counter &operator+=(T const &value)
  {
    value_ = value;
    return *this;
  }

  Counter &operator+=(T &&value)
  {
    value_ = std::move(value);
    return *this;
  }

  T const &value() const
  {
    assert(value_.has_value());
    return *value_;
  }

  T const *try_value() const { return value_.has_value() ? &*value_ : nullptr; }

  void reset() { value_.reset(); }
  bool empty() const { return !value_.has_value(); }

  T const *operator->() const { return &value(); }
  T const &operator*() const { return value(); }

private:
  std::optional<T> value_ = {};
};

} // namespace data

#include <util/counter.inl>
