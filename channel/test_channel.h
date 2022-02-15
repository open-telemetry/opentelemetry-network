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

#include <channel/channel.h>
#include <channel/network_channel.h>
#include <common/intake_encoder.h>
#include <scheduling/timer.h>
#include <util/json.h>
#include <util/log.h>

#include <map>
#include <sstream>

#include <uv.h>

namespace channel {

/**
 * A channel intended for use by unit tests.  It implements all Channel methods.  If loop is provided to the constructor then
 * it also implements the NetworkChannel methods.
 */
class TestChannel : public NetworkChannel {
public:
  TestChannel(
      std::optional<std::reference_wrapper<uv_loop_t>> loop = std::nullopt, IntakeEncoder encoder = IntakeEncoder::binary);

  std::error_code send(const u8 *data, int size) override;

  void close() override;
  std::error_code flush() override;

  bool is_open() const override { return true; }

  void connect(Callbacks &callbacks) override;
  in_addr_t const *connected_address() const override;

  u64 get_num_sends();
  u64 get_num_failed_sends();
  std::stringstream &get_ss();

  using MessageCountsType = std::map<std::string, u64>; // map of message name to number sent
  MessageCountsType &get_message_counts();

  using JsonMessagesType = std::vector<nlohmann::json>; // vector of messages sent in JSON format
  JsonMessagesType &get_json_messages();

private:
  std::optional<std::reference_wrapper<uv_loop_t>> loop_;
  std::unique_ptr<scheduling::Timer> fake_connected_cb_timer_;

  u64 num_sends_ = 0;
  u64 num_failed_sends_ = 0;

  IntakeEncoder encoder_;
  std::stringstream ss_;
  MessageCountsType message_counts_;
  JsonMessagesType json_messages_;
};

} // namespace channel
