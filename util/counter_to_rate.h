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

template <typename T> struct CounterToRate {
  constexpr CounterToRate() = default;
  constexpr CounterToRate(CounterToRate const &) = default;
  constexpr CounterToRate(CounterToRate &&) = default;

  constexpr CounterToRate(T data, T prev = {}) : value_{std::move(data)}, prev_{std::move(prev)}, count_{1} {}

  constexpr T peek_rate() const { return value_ - prev_; }
  constexpr T commit_rate(bool empty_if_unitary = false);

  constexpr T const &value() const { return value_; }
  constexpr T const &prev() const { return prev_; }

  constexpr std::size_t count() const { return count_; }
  constexpr bool empty() const { return !count_; }

  CounterToRate &reset();

  template <typename U> CounterToRate &operator+=(U &&value);

  template <typename Out, typename U> friend Out &&operator<<(Out &&out, CounterToRate<U> const &what);

private:
  T value_ = {};
  T prev_ = {};
  std::size_t count_ = 0;
};

} // namespace data

#include <util/counter_to_rate.inl>
