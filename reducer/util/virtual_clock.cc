// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "virtual_clock.h"

#include <algorithm>

VirtualClock::VirtualClock(fast_div const &divider) : divider_(divider), timeslot_duration_(divider_.estimated_reciprocal()) {}

void VirtualClock::add_inputs(size_t n)
{
  inputs_.resize(inputs_.size() + n);
}

size_t VirtualClock::n_inputs() const
{
  return inputs_.size();
}

bool VirtualClock::is_current(size_t input_index)
{
  return current_timeslot_.has_value() && (inputs_.at(input_index).timeslot == current_timeslot_);
}

bool VirtualClock::can_update(size_t input_index)
{
  return (inputs_.at(input_index).timeslot == current_timeslot_);
}

int VirtualClock::update(size_t input_index, u64 timestamp)
{
  auto &input = inputs_.at(input_index);

  if (input.timeslot != current_timeslot_) {
    return -EPERM;
  }

  const timeslot_t timeslot = timestamp / divider_;

  if (input.timeslot) {
    timeslot_diff_t timeslot_diff = (timeslot_diff_t)timeslot - *input.timeslot;
    if (timeslot_diff >= 0) {
      *input.timeslot += timeslot_diff;
    } else {
      return -EINVAL;
    }
  } else {
    input.timeslot = timeslot;
  }

  return 0;
}

bool VirtualClock::advance()
{
  if (current_timeslot_) {
    if (auto advance_slots = min_input_advance().value_or(0); advance_slots > 0) {
      // All inputs have moved into newer timeslots.
      *current_timeslot_ += advance_slots;
      return true;
    }
  } else {
    // Initializing the current timeslot to the earliest input timeslot.
    current_timeslot_ = earliest_input_timeslot();
  }

  return false;
}

std::optional<VirtualClock::timeslot_t> VirtualClock::earliest_input_timeslot()
{
  std::optional<timeslot_t> min_timeslot;

  for (auto &input : inputs_) {
    if (!input.timeslot) {
      return std::nullopt;
    }

    min_timeslot = min_timeslot ? std::min(*min_timeslot, *input.timeslot) : *input.timeslot;
  }

  std::optional<timeslot_diff_t> min_diff;

  for (auto &input : inputs_) {
    timeslot_diff_t diff = (timeslot_diff_t)(*input.timeslot) - *min_timeslot;
    min_diff = min_diff ? std::min(*min_diff, diff) : diff;
  }

  return *min_timeslot + *min_diff;
}

std::optional<VirtualClock::timeslot_diff_t> VirtualClock::min_input_advance()
{
  std::optional<timeslot_diff_t> min_advance;

  for (auto &input : inputs_) {
    if (!input.timeslot) {
      return std::nullopt;
    }

    timeslot_diff_t advance = (timeslot_diff_t)(*input.timeslot) - *current_timeslot_;

    min_advance = min_advance ? std::min(*min_advance, advance) : advance;
  }

  return min_advance;
}
