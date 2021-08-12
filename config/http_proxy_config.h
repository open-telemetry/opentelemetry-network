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

#include <channel/callbacks.h>
#include <common/http_status_code.h>
#include <util/expected.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

#include <cstdint>

namespace channel {
class Channel;
} // namespace channel

namespace config {

class HttpProxyConfig {
  // environment variable names used by `read_from_env()`
  static constexpr auto PROXY_HOST_VAR = "FLOWMILL_PROXY_HOST";
  static constexpr auto PROXY_PORT_VAR = "FLOWMILL_PROXY_PORT";
  static constexpr auto PROXY_BASIC_AUTH_VAR = "FLOWMILL_PROXY_BASIC_AUTH";
  static constexpr std::string_view DEFAULT_PROXY_PORT = "1080";

public:
  struct Auth {
  };

  struct BasicAuth : Auth {
    explicit BasicAuth(std::string_view credentials);

    inline std::string_view payload() const { return payload_; }

  private:
    std::string payload_;
  };

  struct CallbackWrapper : channel::Callbacks {
    explicit CallbackWrapper(
        std::string_view host,
        std::string_view port,
        std::string_view credentials,
        channel::Channel &channel,
        channel::Callbacks &callback)
        : host_(host), port_(port), credentials_(credentials), channel_(channel), callback_(&callback)
    {}

    u32 received_data(const u8 *data, int length) override;

    void on_error(int error) override { return callback_->on_error(error); }

    void on_closed() override { return callback_->on_closed(); }

    void on_connect() override;

    enum class stage_t : std::uint8_t {
      disconnected = 0,
      connecting = 1,
      connected = 2,
    };

    stage_t stage() const { return stage_; }

  private:
    std::string_view host_;
    std::string_view port_;
    std::string_view credentials_;
    channel::Channel &channel_;
    channel::Callbacks *callback_;
    stage_t stage_ = stage_t::disconnected;
  };

  /**
   * host: the host to connect to through the http proxy
   * port: the port to connect to through the http proxy
   */
  explicit HttpProxyConfig(std::string host, std::string port) : host_(std::move(host)), port_(std::move(port)) {}

  /**
   * host: the host to connect to through the http proxy
   * port: the port to connect to through the http proxy
   * credentials: the credentials to use for the `Proxy-Authorization` header
   */
  template <
      typename Credentials,
      typename = std::enable_if_t<std::is_base_of_v<HttpProxyConfig::Auth, std::decay_t<Credentials>>>>
  explicit HttpProxyConfig(std::string host, std::string port, Credentials &&credentials = {})
      : host_(std::move(host)), port_(std::move(port)), credentials_(credentials.payload())
  {}

  std::unique_ptr<channel::Callbacks>
  make_callback(std::string_view host, std::string_view port, channel::Channel &channel, channel::Callbacks &callback) const;

  inline std::string const &host() const { return host_; }
  inline std::string const &port() const { return port_; }

  /**
   * Reads proxy configuration from existing environment variables.
   *
   * NOTE: this function reads environment variables so it's advisable to call it
   * before any thread is created, given that reading/writing to environment
   * variables is not thread safe and we can't control 3rd party libraries.
   */
  static std::optional<HttpProxyConfig> read_from_env();

private:
  std::string host_;
  std::string port_;
  std::string credentials_;
};

} // namespace config
