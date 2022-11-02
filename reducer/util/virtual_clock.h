/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>
#include <util/fast_div.h>

#include <optional>
#include <vector>

// Clock driven by multiple inputs.
//
// Input timestamps are divided into timeslots based on the divider which can
// be supplied to the constructor. Once all imputs move out of the current
// timeslot, the clock can advance.
//
// Inputs are first added using the `add_inputs()` method.
//
class VirtualClock {
public:
  typedef u16 timeslot_t;

  // Constructs the object by using the specified timestamp divider.
  explicit VirtualClock(fast_div const &divider = {1e9, 16});

  // Adds `n` additional inputs.
  void add_inputs(size_t n);

  // Returns the current number of inputs this clock has.
  size_t n_inputs() const;

  // Returns whether the specified input is current with this clock.
  // Current means that the input timeslot is aligned with the clock's timeslot.
  // Assumes `input_index` < `n_inputs()`.
  bool is_current(size_t input_index);

  // Returns whether the specified input can be updated.
  // Assumes `input_index` < `n_inputs()`.
  bool can_update(size_t input_index);

  // Updates the specified input.
  // Assumes `input_index` < `n_inputs()`.
  // Returns 0 on success;
  //         -EINVAL if the supplied timestamp points to a past timeslot;
  //         -EPERM if the specified input can't be updated (`can_update()`
  //                would return `false`).
  int update(size_t input_index, u64 timestamp);

  // Duration of time slots, in timestamp units.
  double timeslot_duration() const { return timeslot_duration_; }

  // Current timeslot, or nullopt if not yet initialized.
  std::optional<timeslot_t> current_timeslot() const { return current_timeslot_; }

  // Advances this clock's timeslot, if possible.
  // Returns `true` if advanced, `false` otherwise.
  bool advance();

private:
  typedef s16 timeslot_diff_t;

  struct Input {
    std::optional<timeslot_t> timeslot;
  };

  std::vector<Input> inputs_;

  // Divides input timestamps into clock timeslots.
  fast_div divider_;
  // Approximate duration of timeslots in timestamp units.
  double timeslot_duration_{0};

  // This clock's current timeslot.
  std::optional<timeslot_t> current_timeslot_;

  // Returns the earliest timeslot value of all inputs, or nullopt if
  // not all inputs have been updated.
  std::optional<timeslot_t> earliest_input_timeslot();

  // Returns the smallest advance in timeslots of all inputs, or nullopt
  // if not all inputs have been updated.
  // Assumes `current_timeslot_` is initialized.
  std::optional<timeslot_diff_t> min_input_advance();
};
