/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
// Set of interfaces provided by ResyncQueue to its clients.

#include <functional>
#include <memory>

#include "platform/types.h"
#include "util/element_queue_cpp.h"

namespace collector {
class ResyncChannel;

// Interface provided to ResyncQueue' producer (ResyncChannel)
class ResyncQueueProducerInterface {
public:
  ResyncQueueProducerInterface() = default;
  virtual ~ResyncQueueProducerInterface() = default;

  // Sends |data| of |data_len|, returns false if error occurs.
  virtual std::error_code producer_send(const u8 *data, int data_len, const u64 resync) = 0;
  virtual void producer_unregister(ResyncChannel *channel) = 0;
};

// Interface provided to ResyncQueue's consumer (ResyncProcessor)
class ResyncQueueConsumerInterface {
public:
  ResyncQueueConsumerInterface() = default;
  virtual ~ResyncQueueConsumerInterface() = default;

  virtual ElementQueue *consumer_get_queue() = 0;
  virtual u64 consumer_get_last_resync() const = 0;
  virtual void consumer_reset() = 0;
};

// Interface for creating and registering a ResyncChannel
class ResyncChannelFactory {
public:
  ResyncChannelFactory() = default;
  virtual ~ResyncChannelFactory() = default;

  virtual std::unique_ptr<ResyncChannel> new_channel(std::function<void(void)> &reset_callback) = 0;
};

} // namespace collector
