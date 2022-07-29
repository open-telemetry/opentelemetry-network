/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
