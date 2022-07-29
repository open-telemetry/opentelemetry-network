// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "resync_channel.h"

namespace collector {

ResyncChannel::ResyncChannel(
    const u64 resync, ResyncQueueProducerInterface *resync_queue, std::function<void(void)> &reset_callback)
    : resync_(resync), resync_queue_(resync_queue), reset_callback_(reset_callback)
{}

ResyncChannel::~ResyncChannel()
{
  resync_queue_->producer_unregister(this);
}

void ResyncChannel::reset()
{
  reset_callback_();
}

std::error_code ResyncChannel::send(const u8 *data, int data_len)
{
  auto const error = resync_queue_->producer_send(data, data_len, resync_);
  if (error) {
    reset();
  }
  return error;
}

} // namespace collector
