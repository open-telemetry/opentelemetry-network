/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utility>

#include <cstdlib>

namespace data {

template <typename T> struct Gauge {
  Gauge() = default;

  Gauge(T data) : value_(data), min_(data), max_(data), sum_(std::move(data)), count_(1), changed_(true) {}

  template <typename Out = double> Out average() const;

  T const &value() const { return value_; }
  T const &min() const { return min_; }
  T const &max() const { return max_; }
  T const &sum() const { return sum_; }

  Gauge &operator+=(T const &value);
  Gauge &operator+=(Gauge const &value);

  bool changed() const { return changed_; }

  void reset();

  std::size_t count() const { return count_; }
  bool empty() const { return !count_; }

private:
  T value_ = {};
  T min_ = {};
  T max_ = {};
  T sum_ = {};
  std::size_t count_ = 0;
  bool changed_ = false;
};

} // namespace data

#include <util/gauge.inl>
