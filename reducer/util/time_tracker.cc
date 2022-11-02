// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <platform/userspace-time.h>
#include <reducer/util/time_tracker.h>

#include <cmath>

using namespace std::literals::chrono_literals;

constexpr auto ACCEPTED_CLOCK_DIFFERENCE = 10s;

TimeTracker::TimeTracker() : last_message_seen_(now()) {}

inline TimeTracker::timestamp_t TimeTracker::now() const
{
  return std::chrono::nanoseconds(fp_get_time_ns());
}

bool TimeTracker::message_received(timestamp_t client_timestamp)
{
  auto const timestamp = now();

  // keeps track of server time of last received message
  last_message_seen_ = timestamp;

  // computes clock offset between client and server
  if (client_timestamp.count()) {
    static_assert(std::is_signed_v<timestamp_t::rep>);
    timestamp_t const diff = timestamp - client_timestamp;

    // TODO: IMPLEMENT MOVING AVERAGE TO USE LAST n OFFSETS RATHER THAN LAST
    // TODO: ACCOUNT FOR ROUND-TRIP TIME
    // TODO: DITCH SAMPLES / DISCONNECT CLIENT WHEN OFFSET IS ABOVE A THRESHOLD
    clock_offset_ = timestamp_t(std::abs(diff.count()));

    if (clock_offset_ >= ACCEPTED_CLOCK_DIFFERENCE) {
      return false;
    }
  }

  return true;
}

std::chrono::nanoseconds TimeTracker::clock_offset() const
{
  return clock_offset_;
}

std::chrono::nanoseconds TimeTracker::time_since_last_message() const
{
  return now() - last_message_seen_;
}
