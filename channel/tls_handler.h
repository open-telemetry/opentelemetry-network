/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
