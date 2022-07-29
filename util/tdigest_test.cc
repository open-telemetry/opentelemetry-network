// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "util/tdigest.h"
#include "gtest/gtest.h"

namespace util {
namespace {

TEST(TDigestTest, Basic)
{
  TDigest digest;
  TDigestAccumulator accumulator(digest);

  for (int i = 1; i <= 100; ++i) {
    accumulator.add(static_cast<double>(i * 1.0));
  }
  accumulator.flush();

  EXPECT_EQ(digest.value_count(), 100u);
  EXPECT_EQ(digest.sum(), 5050.0);
  EXPECT_EQ(digest.mean(), 50.5);
  EXPECT_EQ(digest.min(), 1);
  EXPECT_EQ(digest.max(), 100);

  EXPECT_EQ(digest.estimate_value_at_quantile(0.001), 1);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.01), 2.0 - 0.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.5), 50.375);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.99), 99.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.999), 100);
}

TEST(TDigestTest, MergeMore)
{
  TDigest digest;
  TDigestAccumulator accumulator(digest);

  for (int i = 1; i <= 100; ++i) {
    accumulator.add(static_cast<double>(i * 1.0));
  }
  accumulator.flush();

  for (int i = 101; i <= 200; ++i) {
    accumulator.add(static_cast<double>(i * 1.0));
  }
  accumulator.flush();

  EXPECT_EQ(digest.value_count(), 200u);
  EXPECT_EQ(digest.sum(), 20100);
  EXPECT_EQ(digest.mean(), 100.5);
  EXPECT_EQ(digest.min(), 1);
  EXPECT_EQ(digest.max(), 200);

  EXPECT_EQ(digest.estimate_value_at_quantile(0.001), 1);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.01), 4.0 - 1.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.5), 100.25);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.99), 200.0 - 1.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.999), 200.0);
}

TEST(TDigestTest, OnlyOne)
{
  TDigest digest;
  TDigestAccumulator accumulator(digest);

  accumulator.add(1.0);
  accumulator.flush();

  EXPECT_EQ(digest.value_count(), 1u);
  EXPECT_EQ(digest.sum(), 1);
  EXPECT_EQ(digest.mean(), 1);
  EXPECT_EQ(digest.min(), 1);
  EXPECT_EQ(digest.max(), 1);

  EXPECT_EQ(digest.estimate_value_at_quantile(0.001), 1.0);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.01), 1.0);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.5), 1.0);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.99), 1.0);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.999), 1.0);
}

TEST(TDigestTest, OneK)
{
  TDigest digest;
  TDigestAccumulator accumulator(digest);

  for (int i = 1; i <= 1000; ++i) {
    accumulator.add(static_cast<double>(i * 1.0));
  }
  accumulator.flush();

  EXPECT_EQ(digest.value_count(), 1000u);
  EXPECT_EQ(digest.sum(), 500500.0);
  EXPECT_EQ(digest.mean(), 500.5);
  EXPECT_EQ(digest.min(), 1);
  EXPECT_EQ(digest.max(), 1000);

  EXPECT_EQ(digest.estimate_value_at_quantile(0.001), 1.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.01), 10.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.5), 500.25);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.99), 990.25);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.999), 999.5);
}

TEST(TDigestTest, NegativeValue)
{
  TDigest digest;
  TDigestAccumulator accumulator(digest);

  for (int i = 1; i <= 100; ++i) {
    accumulator.add(static_cast<double>(i * 1.0));
    accumulator.add(static_cast<double>(i * -1.0));
  }

  accumulator.flush();

  EXPECT_EQ(digest.value_count(), 200u);
  EXPECT_EQ(digest.sum(), 0);
  EXPECT_EQ(digest.mean(), 0);
  EXPECT_EQ(digest.min(), -100);
  EXPECT_EQ(digest.max(), 100);

  EXPECT_EQ(digest.estimate_value_at_quantile(0.001), -100);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.01), -98.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.5), 0);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.99), 98.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.999), 100);
}

TEST(TDigestTest, TwoTDigiests)
{
  TDigest digest;
  TDigest other;

  TDigestAccumulator accumulator(digest);
  TDigestAccumulator other_accumulator(other);

  for (int i = 1; i <= 50; ++i) {
    accumulator.add(static_cast<double>(i * 1.0));
  }
  accumulator.flush();

  for (int i = 51; i <= 100; ++i) {
    other_accumulator.add(static_cast<double>(i * 1.0));
  }
  other_accumulator.flush();

  digest.merge(other);

  EXPECT_EQ(digest.value_count(), 100u);
  EXPECT_EQ(digest.sum(), 5050.0);
  EXPECT_EQ(digest.mean(), 50.5);
  EXPECT_EQ(digest.min(), 1);
  EXPECT_EQ(digest.max(), 100);

  EXPECT_EQ(digest.estimate_value_at_quantile(0.001), 1);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.01), 2.0 - 0.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.5), 50.375);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.99), 99.5);
  EXPECT_EQ(digest.estimate_value_at_quantile(0.999), 100);
}

TEST(TDigestTest, TwoLargeTDigiests)
{
  TDigest digest;
  TDigest other;

  TDigestAccumulator accumulator(digest);
  TDigestAccumulator other_accumulator(other);

  for (int i = 1; i <= 100; ++i) {
    accumulator.add(static_cast<double>(i * 1.0));
  }
  accumulator.flush();

  for (int i = 1; i <= 100; ++i) {
    other_accumulator.add(static_cast<double>(i * 1.0));
  }
  other_accumulator.flush();

  digest.merge(other);
  /*
    EXPECT_EQ(digest.value_count(), 100u);
    EXPECT_EQ(digest.sum(), 5050.0);
    EXPECT_EQ(digest.mean(), 50.5);
    EXPECT_EQ(digest.min(), 1);
    EXPECT_EQ(digest.max(), 100);

    EXPECT_EQ(digest.estimate_value_at_quantile(0.001), 1);
    EXPECT_EQ(digest.estimate_value_at_quantile(0.01), 2.0 - 0.5);
    EXPECT_EQ(digest.estimate_value_at_quantile(0.5), 50.375);
    EXPECT_EQ(digest.estimate_value_at_quantile(0.99), 99.5);
    EXPECT_EQ(digest.estimate_value_at_quantile(0.999), 100);
  */
}
} // namespace
} // namespace util
