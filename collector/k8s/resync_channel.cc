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
