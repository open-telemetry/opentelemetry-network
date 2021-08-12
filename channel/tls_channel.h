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

#include <channel/tcp_channel.h>
#include <exception>
#include <memory>
#include <string>

namespace channel {

class Callbacks;

namespace internal {
class SSLContext;
class TLSShim;
}; // namespace internal

/**
 * A TLS channel
 */
class TLSChannel : public Channel {
public:
  /**
   * A guard-like class to initialize the TLS library.
   *
   * A single Initializer must be alive before instantiating TLSChannels
   */
  class Initializer {
  public:
    /**
     * c'tor. Initialize the TLS library.
     */
    Initializer();

    /**
     * d'tor. Clean up the TLS library (this might be partially implemented)
     */
    ~Initializer();

    /**
     * Index to the exdata that links the SSL CTX to its TLSChannel
     */
    static int channel_index;
  };

  class Credentials {
  public:
    Credentials(std::string client_key, std::string client_crt);

  private:
    friend class TLSChannel;

    const std::string client_key_;
    const std::string client_crt_;
  };

  /**
   * c'tor
   * @param transport: the ciphertext transport
   * @param creds: the credentials to use to establish connection
   * @param server_hostname: hostname for server certificate authentication
   */
  TLSChannel(TCPChannel &transport, Credentials &creds, std::string server_hostname);

  /**
   * d'tor
   */
  virtual ~TLSChannel();

  /**
   * Connects to an endpoint
   * @param callbacks: the callbacks to use during this connection
   */
  void connect(Callbacks &callbacks);

  /**
   * Data has been received from the underlying (e.g., TCP) transport.
   *
   * @returns how many bytes were consumed
   */
  u32 received_data(const u8 *data, u32 data_len);

  /**
   * disconnects the channel
   *
   * @important: this does NOT call the callback's close().
   */
  void close() override;

  /**
   * see @PollChannel::send
   */
  std::error_code send(const u8 *data, int data_len) override;

  /**
   * Accessor for the peer's hostname to verify on the certificate
   */
  const std::string &peer_hostname();

  bool is_open() const override { return tls_shim_ && transport_.is_open(); }

private:
  /**
   * Writes pending data from the transport bio to the underlying Channel
   *
   * @return 0 on success, negative error on failure
   */
  std::error_code flush_transport_bio();

  /**
   * try to finish the handshake
   * @returns:
   *   0 on normal conditions
   *   negative error on failure
   */
  int handshake();

  TCPChannel &transport_;
  Callbacks *callbacks_ = nullptr;
  Credentials &creds_;
  std::string server_hostname_;

  std::unique_ptr<channel::internal::SSLContext> ssl_context_;
  std::unique_ptr<channel::internal::TLSShim> tls_shim_;
};

} /* namespace channel */
