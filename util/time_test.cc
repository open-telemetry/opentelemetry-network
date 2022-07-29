// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/time.h>

#include <platform/types.h>
#include <util/log_formatters.h>

#include <gtest/gtest.h>

#include <chrono>

constexpr std::size_t ITERATIONS = 1'000'000;

TEST(time, integer_time_duration)
{
  EXPECT_EQ(1, integer_time<std::chrono::seconds>(1s));
  EXPECT_EQ(1'000, integer_time<std::chrono::milliseconds>(1s));
  EXPECT_EQ(1'000'000, integer_time<std::chrono::microseconds>(1s));
  EXPECT_EQ(1'000'000'000, integer_time<std::chrono::nanoseconds>(1s));

  EXPECT_EQ(0, integer_time<std::chrono::seconds>(1ms));
  EXPECT_EQ(1, integer_time<std::chrono::milliseconds>(1ms));
  EXPECT_EQ(1'000, integer_time<std::chrono::microseconds>(1ms));
  EXPECT_EQ(1'000'000, integer_time<std::chrono::nanoseconds>(1ms));

  EXPECT_EQ(0, integer_time<std::chrono::seconds>(1us));
  EXPECT_EQ(0, integer_time<std::chrono::milliseconds>(1us));
  EXPECT_EQ(1, integer_time<std::chrono::microseconds>(1us));
  EXPECT_EQ(1'000, integer_time<std::chrono::nanoseconds>(1us));

  EXPECT_EQ(0, integer_time<std::chrono::seconds>(1ns));
  EXPECT_EQ(0, integer_time<std::chrono::milliseconds>(1ns));
  EXPECT_EQ(0, integer_time<std::chrono::microseconds>(1ns));
  EXPECT_EQ(1, integer_time<std::chrono::nanoseconds>(1ns));
}

template <typename R, typename P> auto tp(std::chrono::duration<R, P> duration)
{
  return std::chrono::time_point<monotonic_clock, std::chrono::duration<R, P>>{duration};
}

TEST(time, integer_time_timepoint)
{
  EXPECT_EQ(1, integer_time<std::chrono::seconds>(tp(1s)));
  EXPECT_EQ(1'000, integer_time<std::chrono::milliseconds>(tp(1s)));
  EXPECT_EQ(1'000'000, integer_time<std::chrono::microseconds>(tp(1s)));
  EXPECT_EQ(1'000'000'000, integer_time<std::chrono::nanoseconds>(tp(1s)));

  EXPECT_EQ(0, integer_time<std::chrono::seconds>(tp(1ms)));
  EXPECT_EQ(1, integer_time<std::chrono::milliseconds>(tp(1ms)));
  EXPECT_EQ(1'000, integer_time<std::chrono::microseconds>(tp(1ms)));
  EXPECT_EQ(1'000'000, integer_time<std::chrono::nanoseconds>(tp(1ms)));

  EXPECT_EQ(0, integer_time<std::chrono::seconds>(tp(1us)));
  EXPECT_EQ(0, integer_time<std::chrono::milliseconds>(tp(1us)));
  EXPECT_EQ(1, integer_time<std::chrono::microseconds>(tp(1us)));
  EXPECT_EQ(1'000, integer_time<std::chrono::nanoseconds>(tp(1us)));

  EXPECT_EQ(0, integer_time<std::chrono::seconds>(tp(1ns)));
  EXPECT_EQ(0, integer_time<std::chrono::milliseconds>(tp(1ns)));
  EXPECT_EQ(0, integer_time<std::chrono::microseconds>(tp(1ns)));
  EXPECT_EQ(1, integer_time<std::chrono::nanoseconds>(tp(1ns)));
}

TEST(time, from_clock_ticks)
{
  EXPECT_EQ(1, from_clock_ticks<std::chrono::seconds>(clock_ticks_per_second).count());
  EXPECT_EQ(1'000, from_clock_ticks<std::chrono::milliseconds>(clock_ticks_per_second).count());
  EXPECT_EQ(1'000'000, from_clock_ticks<std::chrono::microseconds>(clock_ticks_per_second).count());
  EXPECT_EQ(1'000'000'000, from_clock_ticks<std::chrono::nanoseconds>(clock_ticks_per_second).count());
}

#define TEST_MONOTONIC_CLOCK(Clock)                                                                                            \
  do {                                                                                                                         \
    using clock_type = Clock;                                                                                                  \
    auto const start = clock_type::now();                                                                                      \
    auto timestamp = start;                                                                                                    \
                                                                                                                               \
    for (auto i = ITERATIONS; i--;) {                                                                                          \
      auto const now = clock_type::now();                                                                                      \
      EXPECT_LE(timestamp, now);                                                                                               \
      timestamp = now;                                                                                                         \
    }                                                                                                                          \
                                                                                                                               \
    std::cout << "finished " << ITERATIONS << " iterations of " #Clock "::now() in " << (timestamp - start)                    \
              << " with an average of " << ((timestamp - start) / ITERATIONS) << " per iteration" << std::endl;                \
  } while (false)

TEST(time, monotonic_clock)
{
  TEST_MONOTONIC_CLOCK(monotonic_clock);
}

TEST(time, rdtsc_clock)
{
  TEST_MONOTONIC_CLOCK(rdtsc_clock);
}

TEST(time, steady_clock)
{
  TEST_MONOTONIC_CLOCK(std::chrono::steady_clock);
}
