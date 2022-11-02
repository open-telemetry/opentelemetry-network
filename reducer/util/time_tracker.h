/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <uv.h>

#include <chrono>

/**
 * This class tracks time between client and server.
 *
 * Things it can do:
 * - calculate clock offset between client and server;
 * - track timeouts from last message received from client.
 */
class TimeTracker {
public:
  using timestamp_t = std::chrono::nanoseconds;

  TimeTracker();

  /**
   * Updates the tracker when a message has been received.
   *
   * @returns: true if the client clock seems sane, false if it differs above
   *   the accepted threshold.
   */
  bool message_received(timestamp_t client_timestamp);

  std::chrono::nanoseconds clock_offset() const;
  std::chrono::nanoseconds time_since_last_message() const;

private:
  inline timestamp_t now() const;

  timestamp_t clock_offset_ = timestamp_t::zero();
  timestamp_t last_message_seen_;
};
