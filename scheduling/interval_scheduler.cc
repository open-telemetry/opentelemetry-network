// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <scheduling/interval_scheduler.h>

#include <util/log.h>

#include <utility>

#include <cassert>

namespace scheduling {

IntervalScheduler::IntervalScheduler(uv_loop_t &loop, JobType job)
    : job_(std::move(job)), timer_(loop, std::bind(&IntervalScheduler::callback, this))
{}

bool IntervalScheduler::defer(TimerPeriod timeout)
{
  return start(timeout, TimerPeriod::zero());
}

bool IntervalScheduler::start(TimerPeriod timeout, TimerPeriod interval)
{
  reset_backoff();
  interval_ = interval;

  if (auto result = timer_.defer(timeout); !result) {
    LOG::error("unable to start interval scheduler: {}", result.error());
    return false;
  }

  return started_ = true;
}

bool IntervalScheduler::start(TimerPeriod timeout)
{
  return start(timeout, timeout);
}

bool IntervalScheduler::restart()
{
  if (auto result = timer_.restart(); !result) {
    LOG::error("unable to restart interval scheduler: {}", result.error());
    return false;
  }

  return started_ = true;
}

bool IntervalScheduler::stop()
{
  if (!started_) {
    return true;
  }

  if (auto result = timer_.stop(); !result) {
    LOG::error("unable to stop interval scheduler: {}", result.error());
    return false;
  }

  started_ = false;

  return true;
}

void IntervalScheduler::callback()
{
  switch (job_()) {
  case JobFollowUp::ok: {
    if (backoff_count_) {
      reset_backoff();
    }

    if (interval_ != TimerPeriod::zero()) {
      timer_.defer(interval_);
    }
    break;
  }

  case JobFollowUp::stop: {
    stop();
    break;
  }

  case JobFollowUp::backoff: {
    ++backoff_count_;
    backoff_ratio_ *= BACKOFF_RATIO;

    auto const interval = interval_ * backoff_ratio_;
    timer_.defer(interval);
    break;
  }

  default:
    assert(false);
    break;
  }
}

void IntervalScheduler::reset_backoff()
{
  backoff_count_ = 0;
  backoff_ratio_ = 1;
}

} // namespace scheduling
