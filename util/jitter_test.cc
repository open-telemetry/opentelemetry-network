// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <util/jitter.h>

#include <platform/types.h>

constexpr std::size_t TIGHT_LOOP_ITERATIONS = 1'000'000;

TEST(jitter, compute_jitter_same_type)
{
  constexpr std::chrono::milliseconds lower_bound = -10s;
  constexpr std::chrono::milliseconds upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = compute_jitter(lower_bound, upper_bound);
    EXPECT_GE(result, lower_bound);
    EXPECT_LE(result, upper_bound);
  }
}

TEST(jitter, compute_jitter_different_types)
{
  constexpr std::chrono::milliseconds lower_bound = -10s;
  constexpr std::chrono::seconds upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = compute_jitter(lower_bound, upper_bound);
    EXPECT_GE(result, lower_bound);
    EXPECT_LE(result, upper_bound);
  }
}

TEST(jitter, add_jitter_duration_same_type)
{
  std::chrono::milliseconds const timepoint = 100s;
  std::chrono::milliseconds const lower_bound = -10s;
  std::chrono::milliseconds const upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = add_jitter(timepoint, lower_bound, upper_bound);
    EXPECT_GE(result, timepoint + lower_bound);
    EXPECT_LE(result, timepoint + upper_bound);
  }
}

TEST(jitter, add_jitter_duration_same_type_jitter)
{
  auto const timepoint = 100s;
  std::chrono::milliseconds const lower_bound = -10s;
  std::chrono::milliseconds const upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = add_jitter(timepoint, lower_bound, upper_bound);
    EXPECT_GE(result, timepoint + lower_bound);
    EXPECT_LE(result, timepoint + upper_bound);
  }
}

TEST(jitter, add_jitter_duration_different_types)
{
  auto const timepoint = 100s;
  std::chrono::milliseconds const lower_bound = -10s;
  std::chrono::seconds const upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = add_jitter(timepoint, lower_bound, upper_bound);
    EXPECT_GE(result, timepoint + lower_bound);
    EXPECT_LE(result, timepoint + upper_bound);
  }
}

TEST(jitter, add_jitter_timepoint_same_type)
{
  auto const timepoint = std::chrono::system_clock::now();
  std::chrono::milliseconds const lower_bound = -10s;
  std::chrono::milliseconds const upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = add_jitter(timepoint, lower_bound, upper_bound);
    EXPECT_GE(result, timepoint + lower_bound);
    EXPECT_LE(result, timepoint + upper_bound);
  }
}

TEST(jitter, add_jitter_timepoint_same_type_jitter)
{
  auto const timepoint = std::chrono::system_clock::now();
  std::chrono::milliseconds const lower_bound = -10s;
  std::chrono::milliseconds const upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = add_jitter(timepoint, lower_bound, upper_bound);
    EXPECT_GE(result, timepoint + lower_bound);
    EXPECT_LE(result, timepoint + upper_bound);
  }
}

TEST(jitter, add_jitter_timepoint_different_types)
{
  auto const timepoint = std::chrono::system_clock::now();
  std::chrono::milliseconds const lower_bound = -10s;
  std::chrono::seconds const upper_bound = +10s;

  for (auto iterations = TIGHT_LOOP_ITERATIONS; iterations--;) {
    auto const result = add_jitter(timepoint, lower_bound, upper_bound);
    EXPECT_GE(result, timepoint + lower_bound);
    EXPECT_LE(result, timepoint + upper_bound);
  }
}
