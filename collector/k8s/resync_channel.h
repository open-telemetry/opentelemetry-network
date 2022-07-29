/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>

#include "channel/channel.h"
#include "platform/types.h"
#include "resync_queue_interface.h"

namespace collector {

// ResyncChannel is a wrapper class provided by ResyncQueue to its producer.
//
// ResyncChannel handles the resync counting and resource clean-up.
class ResyncChannel : public ::channel::Channel {
public:
  ResyncChannel(const u64 resync, ResyncQueueProducerInterface *resync_queue, std::function<void(void)> &reset_callback);
  ~ResyncChannel();

  void reset();
  std::error_code send(const u8 *data, int data_len) override;

  bool is_open() const override { return true; }

private:
  // At which Resync generation that this channel is created.
  const u64 resync_;
  ResyncQueueProducerInterface *resync_queue_; // not owned
  std::function<void(void)> reset_callback_;
}; // class ResyncChannel
} // namespace collector
