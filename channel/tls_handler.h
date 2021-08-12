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

#include <channel/network_channel.h>
#include <channel/tls_channel.h>
#include <channel/tls_over_tcp_channel.h>
#include <map>

namespace channel {

class TLSHandler : public NetworkChannel {
public:
  /**
   * c'tor
   * Throws if:
   *   1. connection error on tcp_channel_
   *   2. call to connect_tls throws
   *
   */
  TLSHandler(
      uv_loop_t &loop,
      std::string addr,
      std::string port,
      std::string agent_key = "",
      std::string agent_crt = "",
      std::string server_hostname = "",
      std::optional<config::HttpProxyConfig> proxy = {});

  /**
   * Connects to an endpoint and starts negotiating
   * @param callbacks: the callbacks to use during this connection
   */
  void connect(Callbacks &callbacks) override;

  std::error_code send(const u8 *data, int data_len) override;

  /**
   * disconnects the channel
   */
  void close() override;
  std::error_code flush() override;

  in_addr_t const *connected_address() const override;

  bool is_open() const override { return tls_channel_.is_open(); }

private:
  TLSChannel::Credentials creds_;
  TlsOverTcpChannel tls_channel_;
};

} // namespace channel
