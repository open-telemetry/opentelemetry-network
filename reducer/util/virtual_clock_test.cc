// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "virtual_clock.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>

constexpr auto TIMESLOT_MIN = std::numeric_limits<VirtualClock::timeslot_t>::min();
constexpr auto TIMESLOT_MAX = std::numeric_limits<VirtualClock::timeslot_t>::max();

static const VirtualClock DEFAULT_CLOCK;

static const u64 TIMESTAMP_STEP = (u64)std::ceil(DEFAULT_CLOCK.timeslot_duration());

TEST(virtual_clock, empty)
{
  VirtualClock clock = DEFAULT_CLOCK;

  ASSERT_EQ(clock.n_inputs(), (size_t)0);
  ASSERT_FALSE(clock.current_timeslot().has_value());
}

TEST(virtual_clock, add_inputs)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  ASSERT_EQ(clock.n_inputs(), (size_t)2);
  ASSERT_FALSE(clock.current_timeslot().has_value());
}

TEST(virtual_clock, current_timeslot)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  u64 timestamp = 0;

  clock.update(0, timestamp);

  ASSERT_FALSE(clock.current_timeslot().has_value());

  clock.update(1, timestamp);

  ASSERT_FALSE(clock.current_timeslot().has_value());

  clock.advance();

  ASSERT_TRUE(clock.current_timeslot().has_value());
}

TEST(virtual_clock, can_update)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  u64 timestamp = 0;

  ASSERT_TRUE(clock.can_update(0));
  ASSERT_TRUE(clock.can_update(1));

  clock.update(0, timestamp);
  clock.update(1, timestamp);

  ASSERT_FALSE(clock.can_update(0));
  ASSERT_FALSE(clock.can_update(1));

  clock.advance();

  ASSERT_TRUE(clock.can_update(0));
  ASSERT_TRUE(clock.can_update(1));

  // update input 1 past the current slot
  ASSERT_EQ(clock.update(1, timestamp + TIMESTAMP_STEP), 0);

  // can't advance until input 0 advances
  ASSERT_EQ(clock.advance(), false);

  ASSERT_TRUE(clock.can_update(0));
  ASSERT_FALSE(clock.can_update(1));
  ASSERT_NE(clock.update(1, timestamp + TIMESTAMP_STEP), 0);

  // advance input 0 and the clock
  ASSERT_EQ(clock.update(0, timestamp + TIMESTAMP_STEP), 0);
  ASSERT_EQ(clock.advance(), true);

  // inputs are in sync
  ASSERT_TRUE(clock.is_current(0));
  ASSERT_TRUE(clock.is_current(1));
  ASSERT_TRUE(clock.can_update(0));
  ASSERT_TRUE(clock.can_update(1));
}

TEST(virtual_clock, initial_slot_min)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  clock.update(0, TIMESTAMP_STEP * (TIMESLOT_MIN + 42));
  clock.update(1, TIMESTAMP_STEP * (TIMESLOT_MIN + 43));
  clock.advance();

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MIN + 42);
}

TEST(virtual_clock, initial_slot_mid)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  const auto TIMESLOT_MID = TIMESLOT_MAX / 2;

  clock.update(0, TIMESTAMP_STEP * (TIMESLOT_MID - 10));
  clock.update(1, TIMESTAMP_STEP * (TIMESLOT_MID + 10));
  clock.advance();

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MID - 10);
}

TEST(virtual_clock, initial_slot_wrap)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  clock.update(0, TIMESTAMP_STEP * TIMESLOT_MAX);
  clock.update(1, TIMESTAMP_STEP * (TIMESLOT_MAX + 1));
  clock.advance();

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MAX);
}

TEST(virtual_clock, initial_slot_wrap_2)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  const auto TIMESLOT_MID = TIMESLOT_MAX / 2;

  clock.update(0, TIMESTAMP_STEP * (TIMESLOT_MID + 10));
  clock.update(1, TIMESTAMP_STEP * (TIMESLOT_MAX + 1));
  clock.advance();

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MID + 10);
}

TEST(virtual_clock, initial_slot_wrap_3)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  const auto TIMESLOT_MID = TIMESLOT_MAX / 2;

  clock.update(0, TIMESTAMP_STEP * (TIMESLOT_MID - 10));
  clock.update(1, TIMESTAMP_STEP * (TIMESLOT_MAX + 1));
  clock.advance();

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MIN);
}

TEST(virtual_clock, advance)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  u64 timestamp = 0;

  clock.update(0, timestamp);
  clock.update(1, timestamp);

  ASSERT_EQ(clock.advance(), false);

  timestamp += TIMESTAMP_STEP;

  clock.update(0, timestamp);
  clock.update(1, timestamp);

  ASSERT_EQ(clock.advance(), true);
}

TEST(virtual_clock, advance_catchup)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  u64 timestamp = TIMESTAMP_STEP * 42;

  clock.update(0, timestamp);
  clock.update(1, timestamp + 2 * TIMESTAMP_STEP);

  ASSERT_EQ(clock.advance(), false);

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), 42);

  clock.update(0, timestamp + TIMESTAMP_STEP);
  ASSERT_EQ(clock.advance(), true);
  ASSERT_EQ(clock.current_timeslot().value(), 43);

  clock.update(0, timestamp + 2 * TIMESTAMP_STEP);
  ASSERT_EQ(clock.advance(), true);
  ASSERT_EQ(clock.current_timeslot().value(), 44);

  ASSERT_TRUE(clock.is_current(0));
  ASSERT_TRUE(clock.is_current(1));
}

TEST(virtual_clock, advance_skipslots)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  u64 timestamp = TIMESTAMP_STEP * 42;

  clock.update(0, timestamp);
  clock.update(1, timestamp + 2 * TIMESTAMP_STEP);
  ASSERT_EQ(clock.advance(), false);

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), 42);

  clock.update(0, timestamp + 2 * TIMESTAMP_STEP);
  ASSERT_EQ(clock.advance(), true);
  ASSERT_EQ(clock.current_timeslot().value(), 44);

  ASSERT_TRUE(clock.is_current(0));
  ASSERT_TRUE(clock.is_current(1));
}

TEST(virtual_clock, advance_wraparound)
{
  VirtualClock clock = DEFAULT_CLOCK;
  clock.add_inputs(2);

  u64 timestamp = TIMESTAMP_STEP * TIMESLOT_MAX;

  clock.update(0, timestamp);
  clock.update(1, timestamp);
  ASSERT_EQ(clock.advance(), false);

  ASSERT_TRUE(clock.current_timeslot().has_value());
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MAX);

  timestamp += TIMESTAMP_STEP;

  clock.update(0, timestamp);
  ASSERT_EQ(clock.advance(), false);
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MAX);

  clock.update(1, timestamp);
  ASSERT_EQ(clock.advance(), true);
  ASSERT_EQ(clock.current_timeslot().value(), TIMESLOT_MIN);

  ASSERT_TRUE(clock.is_current(0));
  ASSERT_TRUE(clock.is_current(1));
}
