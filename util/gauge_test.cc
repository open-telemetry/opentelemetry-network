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

#include <util/gauge.h>

#include <platform/types.h>

#include <gtest/gtest.h>

#include <chrono>

namespace data {

TEST(gauge_int, default_ctor)
{
  Gauge<int> const gauge;
  EXPECT_TRUE(gauge.empty());
  EXPECT_EQ(0ul, gauge.count());
  EXPECT_EQ(0, gauge.min());
  EXPECT_EQ(0, gauge.max());
  EXPECT_EQ(0, gauge.sum());
  EXPECT_EQ(0.0, gauge.average());
}

TEST(gauge_int, cast_constructor)
{
  Gauge<int> const gauge(10);
  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(1ul, gauge.count());
  EXPECT_EQ(10, gauge.min());
  EXPECT_EQ(10, gauge.max());
  EXPECT_EQ(10, gauge.sum());
  EXPECT_EQ(10.0, gauge.average());
}

TEST(gauge_int, default_ctor_add_4_values)
{
  Gauge<int> gauge;
  gauge += 10;
  gauge += 20;
  gauge += 30;
  gauge += 40;
  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(4ul, gauge.count());
  EXPECT_EQ(10, gauge.min());
  EXPECT_EQ(40, gauge.max());
  EXPECT_EQ(100, gauge.sum());
  EXPECT_EQ(25.0, gauge.average());
}

TEST(gauge_int, cast_ctor_add_4_values)
{
  Gauge<int> gauge(60);
  gauge += 90;
  gauge += 70;
  gauge += 50;
  gauge += 80;
  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(5ul, gauge.count());
  EXPECT_EQ(50, gauge.min());
  EXPECT_EQ(90, gauge.max());
  EXPECT_EQ(350, gauge.sum());
  EXPECT_EQ(70.0, gauge.average());
}

TEST(gauge_int, merge_assignment)
{
  Gauge<int> gauge;
  gauge += 10;
  gauge += 20;
  gauge += 30;
  gauge += 40;

  Gauge<int> other(60);
  other += 90;
  other += 70;
  other += 50;
  other += 80;

  gauge += other;

  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(9ul, gauge.count());
  EXPECT_EQ(10, gauge.min());
  EXPECT_EQ(90, gauge.max());
  EXPECT_EQ(450, gauge.sum());
  EXPECT_EQ(50.0, gauge.average());

  EXPECT_FALSE(other.empty());
  EXPECT_EQ(5ul, other.count());
  EXPECT_EQ(50, other.min());
  EXPECT_EQ(90, other.max());
  EXPECT_EQ(350, other.sum());
  EXPECT_EQ(70.0, other.average());
}

TEST(gauge_int, reset)
{
  Gauge<int> gauge(60);
  gauge += 90;
  gauge += 70;
  gauge += 50;
  gauge += 80;

  gauge.reset();
  EXPECT_TRUE(gauge.empty());
  EXPECT_EQ(0ul, gauge.count());
  EXPECT_EQ(0, gauge.min());
  EXPECT_EQ(0, gauge.max());
  EXPECT_EQ(0, gauge.sum());
  EXPECT_EQ(0.0, gauge.average());
}

TEST(gauge_chrono, default_ctor)
{
  Gauge<std::chrono::seconds> const gauge;
  EXPECT_TRUE(gauge.empty());
  EXPECT_EQ(0ul, gauge.count());
  EXPECT_EQ(0s, gauge.min());
  EXPECT_EQ(0s, gauge.max());
  EXPECT_EQ(0s, gauge.sum());
  EXPECT_EQ(0ms, gauge.average<std::chrono::seconds>());
}

TEST(gauge_chrono, cast_constructor)
{
  Gauge<std::chrono::seconds> const gauge(10s);
  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(1ul, gauge.count());
  EXPECT_EQ(10s, gauge.min());
  EXPECT_EQ(10s, gauge.max());
  EXPECT_EQ(10s, gauge.sum());
  EXPECT_EQ(10s, gauge.average<std::chrono::seconds>());
}

TEST(gauge_chrono, default_ctor_add_4_values)
{
  Gauge<std::chrono::seconds> gauge;
  gauge += 10s;
  gauge += 20s;
  gauge += 30s;
  gauge += 40s;
  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(4ul, gauge.count());
  EXPECT_EQ(10s, gauge.min());
  EXPECT_EQ(40s, gauge.max());
  EXPECT_EQ(100s, gauge.sum());
  EXPECT_EQ(25s, gauge.average<std::chrono::seconds>());
}

TEST(gauge_chrono, cast_ctor_add_4_values)
{
  Gauge<std::chrono::seconds> gauge(60s);
  gauge += 90s;
  gauge += 70s;
  gauge += 50s;
  gauge += 80s;
  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(5ul, gauge.count());
  EXPECT_EQ(50s, gauge.min());
  EXPECT_EQ(90s, gauge.max());
  EXPECT_EQ(350s, gauge.sum());
  EXPECT_EQ(70s, gauge.average<std::chrono::seconds>());
}

TEST(gauge_chrono, merge_assignment)
{
  Gauge<std::chrono::seconds> gauge;
  gauge += 10s;
  gauge += 20s;
  gauge += 30s;
  gauge += 40s;

  Gauge<std::chrono::seconds> other(60s);
  other += 90s;
  other += 70s;
  other += 50s;
  other += 80s;

  gauge += other;

  EXPECT_FALSE(gauge.empty());
  EXPECT_EQ(9ul, gauge.count());
  EXPECT_EQ(10s, gauge.min());
  EXPECT_EQ(90s, gauge.max());
  EXPECT_EQ(450s, gauge.sum());
  EXPECT_EQ(50s, gauge.average<std::chrono::seconds>());

  EXPECT_FALSE(other.empty());
  EXPECT_EQ(5ul, other.count());
  EXPECT_EQ(50s, other.min());
  EXPECT_EQ(90s, other.max());
  EXPECT_EQ(350s, other.sum());
  EXPECT_EQ(70s, other.average<std::chrono::seconds>());
}

TEST(gauge_chrono, reset)
{
  Gauge<std::chrono::seconds> gauge(60s);
  gauge += 90s;
  gauge += 70s;
  gauge += 50s;
  gauge += 80s;

  gauge.reset();
  EXPECT_TRUE(gauge.empty());
  EXPECT_EQ(0ul, gauge.count());
  EXPECT_EQ(0s, gauge.min());
  EXPECT_EQ(0s, gauge.max());
  EXPECT_EQ(0s, gauge.sum());
  EXPECT_EQ(0s, gauge.average<std::chrono::seconds>());
}

} // namespace data
