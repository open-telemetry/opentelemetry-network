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

#include <scheduling/timer.h>

namespace scheduling {

Timer::Timer(uv_loop_t &loop, CallbackType callback) : callback_(std::move(callback))
{
  handle_.data = this;

  if (auto const error = ::uv_timer_init(&loop, &handle_)) {
    throw std::runtime_error(::uv_strerror(error));
  }
}

Timer::Timer(Timer &&rhs) : handle_(std::move(rhs.handle_)), callback_(std::move(rhs.callback_))
{
  handle_.data = this;
  rhs.handle_.data = nullptr;
}

Timer::~Timer()
{
  if (!handle_.data) {
    return;
  }

  if (is_active()) {
    stop();
  }
  uv_close(reinterpret_cast<uv_handle_t *>(&handle_), nullptr);
}

Expected<bool, uv_error_t> Timer::defer(TimerPeriod timeout)
{
  return start(timeout, TimerPeriod::zero());
}

Expected<bool, uv_error_t> Timer::start(TimerPeriod timeout, TimerPeriod interval)
{
  assert(!is_active());

  if (auto const error = uv_timer_start(&handle_, callback, timeout.count(), interval.count())) {
    return {unexpected, uv_error_t(error)};
  }

  return true;
}

Expected<bool, uv_error_t> Timer::restart()
{
  assert(is_active());

  if (auto const error = uv_timer_again(&handle_)) {
    return {unexpected, uv_error_t(error)};
  }

  return true;
}

void Timer::update(TimerPeriod interval)
{
  uv_timer_set_repeat(&handle_, interval.count());
}

Expected<bool, uv_error_t> Timer::stop()
{
  assert(is_active());

  if (auto const error = uv_timer_stop(&handle_)) {
    return {unexpected, uv_error_t(error)};
  }

  return true;
}

bool Timer::is_active() const
{
  return uv_is_active(reinterpret_cast<uv_handle_t const *>(&handle_));
}

void Timer::callback(uv_timer_t *handle)
{
  reinterpret_cast<Timer *>(handle->data)->callback_();
};

} // namespace scheduling
