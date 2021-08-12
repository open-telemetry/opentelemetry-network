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
