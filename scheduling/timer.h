/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/expected.h>
#include <util/uv_helpers.h>

#include <uv.h>

#include <chrono>
#include <functional>
#include <stdexcept>
#include <utility>

#include <cassert>

namespace scheduling {

/**
 * An abstraction for libuv's timer.
 *
 * For portability purposes and to avoid library lock-in, this class shouldn't be used directly.
 * Instead, use abstractions like `scheduling/interval_scheduler.h`.
 */
class Timer {
public:
  using TimerPeriod = std::chrono::milliseconds;
  using CallbackType = std::function<void()>;

  /**
   * Constructs a timer to be run on the given event loop, to execute the
   * given callback.
   */
  Timer(uv_loop_t &loop, CallbackType callback);

  Timer(Timer const &) = delete;

  /**
   * If the timer is moved after it has been started, the behavior is undefined.
   */
  Timer(Timer &&rhs);

  /**
   * Dtor.
   */
  ~Timer();

  /**
   * Schedules the callback to be called once after `timeout`.
   */
  Expected<bool, uv_error_t> defer(TimerPeriod timeout);

  /**
   * Schedules the callback to be called every `interval`, with the first call
   * happening after `timeout`.
   */
  Expected<bool, uv_error_t> start(TimerPeriod timeout, TimerPeriod interval);

  /**
   * Stops the timer. If it was initially started with an `interval`, restarts
   * it after `interval` to repeat every `interval`.
   */
  Expected<bool, uv_error_t> restart();

  /**
   * Updates the repeat `interval` of the timer.
   */
  void update(TimerPeriod interval);

  /**
   * Stops the timer.
   */
  Expected<bool, uv_error_t> stop();

  /**
   * Returns the libuv handle for the timer.
   */
  uv_timer_t &native_handle() { return handle_; }

private:
  bool is_active() const;

  static void callback(uv_timer_t *handle);

  uv_timer_t handle_;
  CallbackType callback_;
};

} // namespace scheduling
