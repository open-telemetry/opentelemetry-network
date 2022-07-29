/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scheduling/job.h>
#include <scheduling/timer.h>

#include <functional>

namespace scheduling {

/**
 * A scheduler for jobs that runs on a fixed interval using a timer, supporting a geometric
 * back-off strategy.
 *
 * The job must provide a call operator taking no arguments. If it returns `JobFollowUp`,
 * then that will be used to control follow up runs. If it returns anything else, the return
 * value will be ignored and `JobFollowUp::ok` will be assumed.
 *
 * Job control based on returning the enum `JobFollowUp`:
 *
 *  - ok: proceed normally running the job at the previously specified interval
 *  - stop: stops the timer, no further calls to the job will be made
 *  - backoff: the next iterations of the job will be run on a different interval, according
 *    to the backoff strategy, untill something other than `backoff` is returned.
 *
 * Example:
 *
 *  void main_loop(uv_loop_t &loop) {
 *    IntervalScheduler scheduler(loop, [] {
 *      auto response = http_get("www.foo.bar");
 *
 *      if (!response) {
 *        return JobFollowUp::backoff;
 *      }
 *
 *      process_response(response);
 *
 *      return JobFollowUp::ok;
 *    });
 *
 *    // runs the job every 10 seconds
 *    scheduler.start(10s);
 *  }
 */
class IntervalScheduler {
public:
  using JobType = std::function<JobFollowUp()>;
  using TimerPeriod = Timer::TimerPeriod;

  /**
   * Constructs an interval scheduler on the given event loop, to run the given job.
   */
  explicit IntervalScheduler(uv_loop_t &loop, JobType job);

  /**
   * Runs the job a single time in the future.
   */
  bool defer(TimerPeriod timeout);

  /**
   * Runs the job every `interval` time, starting at `timeout` from now.
   *
   * If `interval` is 0, behaves just like `defer`.
   */
  bool start(TimerPeriod timeout, TimerPeriod interval);

  /**
   * Runs the job every `timeout` time, starting at `timeout` from now.
   */
  bool start(TimerPeriod timeout);

  /**
   * Stops and restarts the job with the pre-configured `timeout`.
   *
   * Requires the job to have been started at some point.
   */
  bool restart();

  /**
   * Stops subsequent job runs.
   */
  bool stop();

  /**
   * Gets a reference to the timer.
   */
  auto &timer() { return timer_; }

private:
  void callback();

  void reset_backoff();

  std::size_t backoff_count_ = 0;
  std::size_t backoff_ratio_ = 1;
  TimerPeriod interval_;
  JobType job_;
  Timer timer_;
  bool started_ = false;

  static constexpr std::size_t BACKOFF_RATIO = 2;
};

} // namespace scheduling
