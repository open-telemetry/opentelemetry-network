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
