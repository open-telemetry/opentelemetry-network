// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/counter_to_rate.h>

#include <platform/types.h>

#include <gtest/gtest.h>

#include <chrono>

namespace data {

TEST(counter_to_rate_int, default_ctor)
{
  CounterToRate<int> counter_to_rate;
  EXPECT_TRUE(counter_to_rate.empty());
  EXPECT_EQ(0ul, counter_to_rate.count());
  EXPECT_EQ(0, counter_to_rate.value());
  EXPECT_EQ(0, counter_to_rate.prev());
  EXPECT_EQ(0, counter_to_rate.peek_rate());
  EXPECT_EQ(0, counter_to_rate.commit_rate());
  EXPECT_EQ(0, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_int, cast_constructor)
{
  CounterToRate<int> counter_to_rate(10);
  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(1ul, counter_to_rate.count());
  EXPECT_EQ(10, counter_to_rate.value());
  EXPECT_EQ(0, counter_to_rate.prev());
  EXPECT_EQ(10, counter_to_rate.peek_rate());
  EXPECT_EQ(10, counter_to_rate.commit_rate());
  EXPECT_EQ(0, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_int, default_ctor_add_4_values)
{
  CounterToRate<int> counter_to_rate;
  EXPECT_TRUE(counter_to_rate.empty());

  counter_to_rate += 11;
  counter_to_rate += 22;
  counter_to_rate += 32;
  counter_to_rate += 44;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(4ul, counter_to_rate.count());
  EXPECT_EQ(44, counter_to_rate.value());
  EXPECT_EQ(0, counter_to_rate.prev());
  EXPECT_EQ(44, counter_to_rate.peek_rate());
  EXPECT_EQ(44, counter_to_rate.commit_rate());
  EXPECT_EQ(0, counter_to_rate.peek_rate());

  counter_to_rate += 55;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(5ul, counter_to_rate.count());
  EXPECT_EQ(55, counter_to_rate.value());
  EXPECT_EQ(44, counter_to_rate.prev());
  EXPECT_EQ(11, counter_to_rate.peek_rate());
  EXPECT_EQ(11, counter_to_rate.commit_rate());
  EXPECT_EQ(0, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_int, cast_ctor_add_4_values)
{
  CounterToRate<int> counter_to_rate{19};
  EXPECT_FALSE(counter_to_rate.empty());

  counter_to_rate += 28;
  counter_to_rate += 37;
  counter_to_rate += 46;
  counter_to_rate += 55;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(5ul, counter_to_rate.count());
  EXPECT_EQ(55, counter_to_rate.value());
  EXPECT_EQ(0, counter_to_rate.prev());
  EXPECT_EQ(55, counter_to_rate.peek_rate());
  EXPECT_EQ(55, counter_to_rate.commit_rate());
  EXPECT_EQ(0, counter_to_rate.peek_rate());

  counter_to_rate += 64;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(6ul, counter_to_rate.count());
  EXPECT_EQ(64, counter_to_rate.value());
  EXPECT_EQ(55, counter_to_rate.prev());
  EXPECT_EQ(9, counter_to_rate.peek_rate());
  EXPECT_EQ(9, counter_to_rate.commit_rate());
  EXPECT_EQ(0, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_int, reset)
{
  CounterToRate<int> counter_to_rate(60);
  counter_to_rate += 90;
  counter_to_rate += 70;
  counter_to_rate += 50;
  counter_to_rate += 80;

  counter_to_rate.reset();
  EXPECT_TRUE(counter_to_rate.empty());
  EXPECT_EQ(0ul, counter_to_rate.count());
  EXPECT_EQ(0, counter_to_rate.prev());
  EXPECT_EQ(0, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_chrono, default_ctor)
{
  CounterToRate<std::chrono::seconds> counter_to_rate;
  EXPECT_TRUE(counter_to_rate.empty());
  EXPECT_EQ(0ul, counter_to_rate.count());
  EXPECT_EQ(0s, counter_to_rate.prev());
  EXPECT_EQ(0s, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_chrono, cast_constructor)
{
  CounterToRate<std::chrono::seconds> counter_to_rate(10s);
  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(1ul, counter_to_rate.count());
  EXPECT_EQ(10s, counter_to_rate.value());
  EXPECT_EQ(0s, counter_to_rate.prev());
  EXPECT_EQ(10s, counter_to_rate.peek_rate());
  EXPECT_EQ(10s, counter_to_rate.commit_rate());
  EXPECT_EQ(0s, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_chrono, default_ctor_add_4_values)
{
  CounterToRate<std::chrono::seconds> counter_to_rate;
  EXPECT_TRUE(counter_to_rate.empty());

  counter_to_rate += 11s;
  counter_to_rate += 22s;
  counter_to_rate += 32s;
  counter_to_rate += 44s;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(4ul, counter_to_rate.count());
  EXPECT_EQ(44s, counter_to_rate.value());
  EXPECT_EQ(0s, counter_to_rate.prev());
  EXPECT_EQ(44s, counter_to_rate.peek_rate());
  EXPECT_EQ(44s, counter_to_rate.commit_rate());
  EXPECT_EQ(0s, counter_to_rate.peek_rate());

  counter_to_rate += 55s;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(5ul, counter_to_rate.count());
  EXPECT_EQ(55s, counter_to_rate.value());
  EXPECT_EQ(44s, counter_to_rate.prev());
  EXPECT_EQ(11s, counter_to_rate.peek_rate());
  EXPECT_EQ(11s, counter_to_rate.commit_rate());
  EXPECT_EQ(0s, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_chrono, cast_ctor_add_4_values)
{
  CounterToRate<std::chrono::seconds> counter_to_rate(60s);
  EXPECT_FALSE(counter_to_rate.empty());

  counter_to_rate += 28s;
  counter_to_rate += 37s;
  counter_to_rate += 46s;
  counter_to_rate += 55s;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(5ul, counter_to_rate.count());
  EXPECT_EQ(55s, counter_to_rate.value());
  EXPECT_EQ(0s, counter_to_rate.prev());
  EXPECT_EQ(55s, counter_to_rate.peek_rate());
  EXPECT_EQ(55s, counter_to_rate.commit_rate());
  EXPECT_EQ(0s, counter_to_rate.peek_rate());

  counter_to_rate += 64s;

  EXPECT_FALSE(counter_to_rate.empty());
  EXPECT_EQ(6ul, counter_to_rate.count());
  EXPECT_EQ(64s, counter_to_rate.value());
  EXPECT_EQ(55s, counter_to_rate.prev());
  EXPECT_EQ(9s, counter_to_rate.peek_rate());
  EXPECT_EQ(9s, counter_to_rate.commit_rate());
  EXPECT_EQ(0s, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_chrono, reset)
{
  CounterToRate<std::chrono::seconds> counter_to_rate(60s);
  counter_to_rate += 90s;
  counter_to_rate += 70s;
  counter_to_rate += 50s;
  counter_to_rate += 80s;

  counter_to_rate.reset();
  EXPECT_TRUE(counter_to_rate.empty());
  EXPECT_EQ(0ul, counter_to_rate.count());
  EXPECT_EQ(0s, counter_to_rate.prev());
  EXPECT_EQ(0s, counter_to_rate.peek_rate());
}

TEST(counter_to_rate_int, commit_rate__value_if_unitary__default_ctor)
{
  CounterToRate<int> counter_to_rate;
  EXPECT_EQ(0, counter_to_rate.commit_rate(false));

  counter_to_rate += 50;
  EXPECT_EQ(50, counter_to_rate.commit_rate(false));

  counter_to_rate += 90;
  EXPECT_EQ(40, counter_to_rate.commit_rate(false));
}

TEST(counter_to_rate_int, commit_rate__empty_if_unitary__default_ctor)
{
  CounterToRate<int> counter_to_rate;
  EXPECT_EQ(0, counter_to_rate.commit_rate(true));

  counter_to_rate += 50;
  EXPECT_EQ(0, counter_to_rate.commit_rate(true));

  counter_to_rate += 90;
  EXPECT_EQ(40, counter_to_rate.commit_rate(true));
}

TEST(counter_to_rate_int, commit_rate__value_if_unitary__cast_ctor)
{
  CounterToRate<int> counter_to_rate{50};
  EXPECT_EQ(50, counter_to_rate.commit_rate(false));

  counter_to_rate += 90;
  EXPECT_EQ(40, counter_to_rate.commit_rate(false));
}

TEST(counter_to_rate_int, commit_rate__empty_if_unitary__cast_ctor)
{
  CounterToRate<int> counter_to_rate{50};
  EXPECT_EQ(0, counter_to_rate.commit_rate(true));

  counter_to_rate += 90;
  EXPECT_EQ(40, counter_to_rate.commit_rate(true));
}

} // namespace data
