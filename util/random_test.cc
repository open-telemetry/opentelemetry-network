// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <util/random.h>

constexpr std::size_t TIGHT_LOOP_ITERATIONS = 1'000'000;

TEST(rng_32, next)
{
  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = rng_32::next();
    EXPECT_GE(result, std::numeric_limits<rng_32::result_type>::min());
    EXPECT_LE(result, std::numeric_limits<rng_32::result_type>::max());
  }
}

TEST(rng_32, next_upper_bound)
{
  constexpr std::int32_t upper_bound = 100;
  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = rng_32::next(upper_bound);
    EXPECT_GE(result, 0);
    EXPECT_LE(result, upper_bound);
  }
}

TEST(rng_32, next_lower_upper_bound)
{
  constexpr std::int32_t lower_bound = -100;
  constexpr std::int32_t upper_bound = 100;
  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = rng_32::next(lower_bound, upper_bound);
    EXPECT_GE(result, lower_bound);
    EXPECT_LE(result, upper_bound);
  }
}

TEST(rng_64, next)
{
  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = rng_64::next();
    EXPECT_GE(result, std::numeric_limits<rng_64::result_type>::min());
    EXPECT_LE(result, std::numeric_limits<rng_64::result_type>::max());
  }
}

TEST(rng_64, next_upper_bound)
{
  constexpr std::int64_t upper_bound = 100;
  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = rng_64::next(upper_bound);
    EXPECT_GE(result, 0);
    EXPECT_LE(result, upper_bound);
  }
}

TEST(rng_64, next_lower_upper_bound)
{
  constexpr std::int64_t lower_bound = -100;
  constexpr std::int64_t upper_bound = 100;
  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = rng_64::next(lower_bound, upper_bound);
    EXPECT_GE(result, lower_bound);
    EXPECT_LE(result, upper_bound);
  }
}
