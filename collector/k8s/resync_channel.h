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
