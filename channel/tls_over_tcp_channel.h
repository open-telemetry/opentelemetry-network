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
#include <channel/tcp_channel.h>
#include <channel/tls_channel.h>
#include <string>
#include <uv.h>

namespace channel {

class Callbacks;

/**
 * A TLS over TCP channel
 */
class TlsOverTcpChannel : public Channel {
public:
  /**
   * c'tor
   * @param transport: the ciphertext transport
   * @param creds: the credentials to use to establish connection
   * @param server_hostname: hostname for server certificate authentication
   */
  TlsOverTcpChannel(
      uv_loop_t &loop,
      std::string addr,
      std::string port,
      TLSChannel::Credentials &creds,
      std::string server_hostname,
      std::optional<config::HttpProxyConfig> proxy = {});

  /**
   * d'tor
   */
  virtual ~TlsOverTcpChannel();

  /**
   * Connects to an endpoint and starts negotiating
   * @param callbacks: the callbacks to use during this connection
   * @param addr: ip address or hostname
   * @param port: string holding port number
   */
  void connect(Callbacks &callbacks);

  /**
   * @see Channel::send
   */
  std::error_code send(const u8 *data, int data_len) override;

  void close() override;
  std::error_code flush() override;

  TCPChannel const &get_tcp_channel() const { return tcp_; }
  TCPChannel &get_tcp_channel() { return tcp_; }

  TLSChannel const &get_tls_channel() const { return tls_; }
  TLSChannel &get_tls_channel() { return tls_; }

  bool is_open() const override { return tls_.is_open() && tcp_.is_open(); }

private:
  class TcpCallbacks : public channel::Callbacks {
  public:
    TcpCallbacks(TlsOverTcpChannel &parent_channel);
    virtual u32 received_data(const u8 *data, int data_len) override;
    virtual void on_error(int err) override;
    virtual void on_closed() override;
    virtual void on_connect() override;

  private:
    TlsOverTcpChannel &parent_channel_;
  };
  friend TcpCallbacks;

  TcpCallbacks tcp_callbacks_;
  TCPChannel tcp_;
  TLSChannel tls_;
  Callbacks *callbacks_ = nullptr;
};

} /* namespace channel */
