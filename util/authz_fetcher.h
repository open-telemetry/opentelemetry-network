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

/**
 * A component for fetching authz tokens from the `authz` backend service.
 */

#include <config/http_proxy_config.h>
#include <scheduling/timer.h>
#include <util/args_parser.h>
#include <util/authz_token.h>
#include <util/curl_engine.h>
#include <util/expected.h>

#include <curlpp/Options.hpp>

#include <string>
#include <string_view>

#define FLOWMILL_AUTH_KEY_ID_VAR "FLOWMILL_AUTH_KEY_ID"
#define FLOWMILL_AUTH_SECRET_VAR "FLOWMILL_AUTH_SECRET"
#define FLOWMILL_AUTH_SERVER_ENV_VAR "FLOWMILL_AUTHZ_SERVER"

class AuthzFetcher {
public:
  struct AgentKey {
    std::string key_id;
    std::string secret;
  };

  /**
   * Callback must return the time left in the token that's being replaced, or
   * `std::chrono::milliseconds::zero()` if there was no previous token.
   */
  using SuccessCallback = std::function<std::chrono::milliseconds(AuthzToken const &)>;

  /**
   * Constructs the authz token fetcher and automatically fetches the first authz token.
   */
  AuthzFetcher(
      CurlEngine &curl,
      std::string authz_server,
      AgentKey const &agent_key,
      std::string const &agent_id,
      config::HttpProxyConfig const *proxy = nullptr);

  /**
   * Destructor will automatically stop auto-refresh if it has been previously initiated.
   */
  ~AuthzFetcher();

  class ScheduledFetch {
  public:
    ScheduledFetch(
        uv_loop_t &loop,
        std::string const &url,
        std::string const &secret_header,
        std::string const &agent_id_header,
        CurlEngine &curl,
        std::function<void(std::string_view)> on_success,
        std::function<void()> on_network_error,
        config::HttpProxyConfig const *proxy = nullptr);

    void schedule(scheduling::Timer::TimerPeriod timeout);
    void schedule_backoff();

    void stop();

  private:
    std::string buffer_;
    CurlEngine::FetchRequest request_;
    SuccessCallback on_success_;
    scheduling::Timer timer_;
    scheduling::Timer::TimerPeriod backoff_interval_;
    std::size_t backoff_count_ = 0;
  };

  Expected<AuthzToken, std::runtime_error> sync_fetch();

  /**
   * Forces a synchronous refresh of the authz token if it's expired or due to
   * expire within the notice period.
   */
  void sync_refresh();

  /**
   * Schedules auto-refresh of the authz token before expiration.
   *
   * Auto-refresh will continue to run indefinitely until a stoppage is requested.
   *
   * Calls `on_success`, if provided, on every successful refresh.
   */
  void auto_refresh(uv_loop_t &loop, SuccessCallback on_success = {});

  /**
   * Cancels previously initiated auto-refresh.
   */
  void stop_auto_refresh();

  Expected<AuthzToken, std::runtime_error> const &token() const { return token_; }

  /**
   * Fetches the agent key from the environment.
   *
   * NOTE: this function reads environment variables so it's advisable to call it
   * before any thread is created, given that reading/writing to environment
   * variables is not thread safe and we can't control 3rd party libraries.
   */
  static Expected<AgentKey, std::string> read_agent_key();

  static cli::ArgsParser::ArgProxy<std::string> &register_args_parser(cli::ArgsParser &parser);

private:
  CurlEngine &curl_;
  config::HttpProxyConfig const *proxy_;

  std::string const api_url_;
  std::string const secret_header_;
  std::string const agent_id_header_;
  Expected<AuthzToken, std::runtime_error> token_;

  std::optional<ScheduledFetch> scheduled_;
};
