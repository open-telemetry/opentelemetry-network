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

#include <platform/generic.h>

#include <chrono>
#include <ratio>

#include <cassert>
#include <cstdint>

/**
 * Converts duration to desired unit and returns its value in integer form.
 *
 * Example:
 *
 *  void foo(std::chrono::nanoseconds timestamp) {
 *    std::cout << "Unix time: " << integer_time<std::chrono::seconds>(timestamp);
 *  }
 */
template <typename OutputDuration, typename Output = typename OutputDuration::rep, typename R, typename P>
constexpr Output integer_time(std::chrono::duration<R, P> duration)
{
  return std::chrono::duration_cast<OutputDuration>(duration).count();
}

/**
 * Converts time point to desired unit and returns its value in integer form.
 *
 * Example:
 *
 *  auto const timestamp = std::chrono::system_clock::now();
 *  std::cout << "Unix time: " << integer_time<std::chrono::seconds>(timestamp);
 */
template <typename OutputDuration, typename Output = typename OutputDuration::rep, typename C, typename D>
constexpr Output integer_time(std::chrono::time_point<C, D> time_point)
{
  return integer_time<OutputDuration>(time_point.time_since_epoch());
}

/**
 * The number of clock ticks per second.
 *
 * This is resolved at process start up time. An exception is thrown on failure.
 */
extern std::uint64_t const clock_ticks_per_second;

/**
 * Converts clock ticks to desired unit.
 *
 * Example:
 *
 *  void foo(std::uint64_t clock_ticks) {
 *    std::cout << "Unix time: " << from_clock_ticks<std::chrono::seconds>(clock_ticks);
 *  }
 */
template <typename OutputDuration, typename T>
constexpr OutputDuration from_clock_ticks(T clock_ticks, std::uint64_t clock_ticks_per_second)
{
  static_assert(
      std::ratio_less_equal_v<typename OutputDuration::period, std::chrono::seconds::period>,
      "can't convert clock ticks to a unit less precise than seconds");
  return OutputDuration{integer_time<OutputDuration>(std::chrono::seconds{clock_ticks}) / clock_ticks_per_second};
}

template <typename OutputDuration, typename T> OutputDuration from_clock_ticks(T clock_ticks)
{
  return from_clock_ticks<OutputDuration>(clock_ticks, clock_ticks_per_second);
}

/**
 * Realtime, std::chrono compatible clock using `CLOCK_REALTIME` as the backend.
 *
 * Note: `glib` should take advantage of `vdso` to avoid making a system call.
 */
struct realtime_clock {
  constexpr static bool is_steady = false;

  using rep = std::uint_fast64_t;
  using period = std::nano;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<realtime_clock>;

  static inline __attribute__((always_inline)) time_point now()
  {
    struct timespec time;
    int const result = clock_gettime(CLOCK_REALTIME, &time);
    if (unlikely(result)) {
      return time_point{duration::zero()};
    }

    return time_point{duration{static_cast<u64>((time.tv_sec * (1000 * 1000 * 1000)) + time.tv_nsec)}};
  }
};

/**
 * Monotonic, std::chrono compatible clock using `CLOCK_MONOTONIC` as the backend.
 *
 * Note: `glib` should take advantage of `vdso` to avoid making a system call.
 */
struct monotonic_clock {
  constexpr static bool is_steady = true;

  using rep = std::uint_fast64_t;
  using period = std::nano;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<monotonic_clock>;

  static inline __attribute__((always_inline)) time_point now()
  {
    struct timespec time;
    int const result = clock_gettime(CLOCK_MONOTONIC, &time);
    if (unlikely(result)) {
      return time_point{duration::zero()};
    }

    return time_point{duration{static_cast<u64>((time.tv_sec * (1000 * 1000 * 1000)) + time.tv_nsec)}};
  }
};

#include <x86intrin.h>
/**
 * Monotonic, std::chrono compatible clock using `__rdtsc()` as the backend.
 */
struct rdtsc_clock {
  constexpr static bool is_steady = true;

  using rep = std::uint_fast64_t;
  using period = std::nano;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<monotonic_clock>;

  static inline __attribute__((always_inline)) time_point now() { return time_point{duration{__rdtsc()}}; }
};
