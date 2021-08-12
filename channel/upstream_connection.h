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

#include <channel/buffered_writer.h>
#include <channel/callbacks.h>
#include <channel/double_write_channel.h>
#include <channel/lz4_channel.h>
#include <channel/network_channel.h>
#include <platform/platform.h>

namespace channel {

class UpstreamConnection : public NetworkChannel {
public:
  UpstreamConnection(
      std::size_t buffer_size, bool allow_compression, NetworkChannel &primary_channel, Channel *secondary_channel = nullptr);

  /**
   * Connects to an endpoint and starts negotiating
   * @param callbacks: the callbacks to use during this connection
   * @param addr: ip address or hostname
   * @param port: string holding port number
   */
  void connect(Callbacks &callbacks) override;

  std::error_code send(const u8 *data, int data_len) override;

  /**
   * Flushes the internal buffers.
   */
  std::error_code flush() override;

  /**
   * disconnects the channel
   */
  void close() override;

  /**
   * Enables/disables compression.
   */
  void set_compression(bool enabled);

  BufferedWriter &buffered_writer();

  in_addr_t const *connected_address() const override;

  bool is_open() const override { return primary_channel_.is_open(); }

private:
  NetworkChannel &primary_channel_;
  Lz4Channel lz4_channel_;
  bool allow_compression_;
  DoubleWriteChannel double_write_channel_;
  BufferedWriter buffered_writer_;
};

} // namespace channel
