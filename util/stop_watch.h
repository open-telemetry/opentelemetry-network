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

#include <util/log_formatters.h>
#include <util/time.h>

#include <chrono>

template <typename Clock = ::monotonic_clock> class StopWatch {
  using clock = Clock;
  using time_point = typename clock::time_point;

  static_assert(clock::is_steady, "stop watch needs a monotonic clock");

public:
  using duration = typename clock::duration;

  StopWatch() : start_(clock::now()) {}

  /**
   * Resets the stop watch.
   */
  void reset() { start_ = clock::now(); }

  /**
   * Computes elapsed time since start to now without resetting the stop watch.
   *
   * Returns the result in `Unit` time unit. Default time unit is unspecified.
   */
  template <typename Unit = duration> Unit elapsed() const { return std::chrono::duration_cast<Unit>(clock::now() - start_); }

  /**
   * Tells whether the given duration has already been elapsed since start.
   */
  template <typename Duration> bool elapsed(Duration duration) const { return duration <= (clock::now() - start_); }

  /**
   * Computes elapsed time since start and resets the stop watch.
   *
   * Returns the result in `Unit` time unit. Default time unit is unspecified.
   */
  template <typename Unit = duration> Unit elapsed_reset()
  {
    auto const now = clock::now();
    auto const result = now - start_;
    start_ = now;
    return std::chrono::duration_cast<Unit>(result);
  }

  template <typename Out> friend Out &&operator<<(Out &&out, StopWatch const &stop_watch)
  {
    out << stop_watch.elapsed();
    return std::forward<Out>(out);
  }

private:
  time_point start_;
};
