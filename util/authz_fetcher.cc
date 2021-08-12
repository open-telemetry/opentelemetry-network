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

#include <util/authz_fetcher.h>
#include <util/jitter.h>
#include <util/restful.h>
#include <util/url.h>
#include <util/utility.h>

#include <util/environment_variables.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/string.h>

#include <spdlog/fmt/fmt.h>

#include <sstream>

#include <cassert>

#define AUTH_K8S_SECRET_NAME "flowmill-k8s-agent-key"
#define AUTH_K8S_NAMESPACE "flowmill"
#define AUTH_K8S_KEY_KEY_ID "flowmill_agent_key_id"
#define AUTH_K8S_KEY_SECRET "flowmill_agent_secret"
#define AUTH_K8S_KEY_AUTH_SERVER "flowmill.services.host"
#define AUTH_FETCH_ERROR_MESSAGE                                                                                               \
  "double check the following items:\n"                                                                                        \
  "- Agent keys are properly configured:\n"                                                                                    \
  "  - in Kubernetes environments, verify that secret `" AUTH_K8S_SECRET_NAME                                                  \
  "` has been set for namespace `" AUTH_K8S_NAMESPACE "` and contains keys `" AUTH_K8S_KEY_KEY_ID                              \
  "` and `" AUTH_K8S_KEY_SECRET "`\n"                                                                                          \
  "  - this container's enironment variables `" FLOWMILL_AUTH_KEY_ID_VAR "` and `" FLOWMILL_AUTH_SECRET_VAR                    \
  "` must contain the agent key\n"                                                                                             \
  "- Flowmill auth server is correctly configured:\n"                                                                          \
  "  - in Kubernetes environments, helm chart's key `" AUTH_K8S_KEY_AUTH_SERVER "`\n"                                          \
  "  - this container's enironment variable `" FLOWMILL_AUTH_SERVER_ENV_VAR "`\n"

#define AGENT_KEY_INSTRUCTIONS                                                                                                 \
  "Please double check that the agent key is not empty, and that a"                                                            \
  " valid key was provided.\n"                                                                                                 \
  "Agent keys can be obtained from the Flowmill UI, through menu"                                                              \
  " option \"Agents\" -> \"Agent Keys\".\n"                                                                                    \
  "Instructions on how to provide the keys can also be obtained from"                                                          \
  " the Flowmill UI, through menu option \"Agents\" -> \"Deployment\"."

#define INVALID_AGENT_KEY_ERROR_MESSAGE                                                                                        \
  "The key supplied in the environment variable " FLOWMILL_AUTH_KEY_ID_VAR                                                     \
  " doesn't look like a valid agent key.\n" AGENT_KEY_INSTRUCTIONS

#define WRONG_TYPE_AGENT_KEY_ERROR_MESSAGE                                                                                     \
  "The key supplied in the environment variable " FLOWMILL_AUTH_KEY_ID_VAR                                                     \
  " doesn't look like an agent key (appears to be an API key"                                                                  \
  " instead).\n" AGENT_KEY_INSTRUCTIONS

// prefix common to agent key IDs
static std::string_view AGENT_KEY_PREFIX = "KFII";

// prefix common to API keys
static std::string_view API_KEY_PREFIX = "KFIA";

// how long before expiration to fetch the next token
static constexpr auto AUTHZ_NOTICE_PERIOD = 10s;

// the jitter to add to authz fetch intervals
// explicitly using milliseconds for a finer grained jitter
static constexpr std::chrono::milliseconds AUTHZ_JITTER = 5s;

// how long to back-off when a request fails
static constexpr scheduling::Timer::TimerPeriod AUTHZ_INITIAL_ERROR_BACKOFF = 200ms;

// how long the longest back-off can be
static constexpr scheduling::Timer::TimerPeriod AUTHZ_LONGEST_ERROR_BACKOFF = 1min;

// geometric ratio to apply to back-off interval on failure
static constexpr unsigned AUTHZ_BACKOFF_GEOMETRIC_RATIO = 2;

// maximum number of tries to fetch initial authz token
static constexpr auto AUTHZ_MAX_INITIAL_FETCH_RETRIES = 9;

AuthzFetcher::AuthzFetcher(
    CurlEngine &curl,
    std::string authz_server,
    AgentKey const &agent_key,
    std::string const &agent_id,
    config::HttpProxyConfig const *proxy)
    : curl_(curl),
      proxy_(proxy),
      api_url_([&] {
        // this is the only place where the API endpoint URL is computed
        auto url = format_url(std::move(authz_server), fmt::format("api/v1/auth/keys/{}", agent_key.key_id));

        if (url.empty()) {
          throw std::invalid_argument("missing authz server hostname");
        }

        return url;
      }()),
      secret_header_(fmt::format("Authorization: Bearer {}", agent_key.secret)),
      // must be prefixed with Grpc-Metadata-*
      agent_id_header_(fmt::format("Grpc-Metadata-Agent-ID: {}", agent_id)),
      token_(sync_fetch())
{
  token_.on_error([&](auto const &error) {
    throw std::runtime_error(fmt::format("{} (Flowmill auth server='{}')\n{}", error, api_url_, AUTH_FETCH_ERROR_MESSAGE));
  });
}

AuthzFetcher::~AuthzFetcher()
{
  stop_auto_refresh();
}

void AuthzFetcher::sync_refresh()
{
  if (!token_ || token_->time_left(AuthzToken::clock::now()) <= AUTHZ_NOTICE_PERIOD) {
    token_ = sync_fetch();
  }
}

AuthzFetcher::ScheduledFetch::ScheduledFetch(
    uv_loop_t &loop,
    std::string const &url,
    std::string const &secret_header,
    std::string const &agent_id_header,
    CurlEngine &curl,
    std::function<void(std::string_view)> on_success,
    std::function<void()> on_error,
    config::HttpProxyConfig const *proxy)
    : request_(
          url,
          [this](const char *data, size_t size) { buffer_.append(data, size); },
          [this, &url, on_success = std::move(on_success), on_error = std::move(on_error)](
              CurlEngineStatus status, long responseCode, std::string_view curlError) {
            std::string content;
            std::swap(buffer_, content);
            if (status != CurlEngineStatus::OK) {
              LOG::error("error [{}] while auto-refreshing authz token from '{}': {}", status, url, curlError);
              on_error();
            } else if (responseCode != 200) {
              LOG::error(
                  "error while auto-refreshing authz token from '{}' (http_status_code={})\n{}",
                  url,
                  responseCode,
                  AUTH_FETCH_ERROR_MESSAGE);
              on_error();
            } else {
              LOG::trace_in(
                  Utility::authz,
                  "successfully fetched new authz token from '{}' at {}",
                  url,
                  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()));
              on_success(content);
            }
          }),
      timer_(loop, [this, &curl] {
        LOG::trace_in(Utility::authz, "scheduling authz fetch");
        curl.schedule_fetch(request_);
      })
{
  if (!request_) {
    LOG::error("AuthzFetcher failed to create a request");
  }
  assert(request_);

  request_.add_header(secret_header.c_str());
  request_.add_header(agent_id_header.c_str());

  if (proxy) {
    LOG::trace_in(Utility::authz, "setting proxy {}:{} for async fetch", proxy->host(), proxy->port());
    request_.http_proxy(proxy->host(), try_integer_from_string<std::uint16_t>(proxy->port().c_str()));
  } else {
    LOG::trace_in(Utility::authz, "not using a proxy for async fetch");
  }
}

void AuthzFetcher::ScheduledFetch::schedule(scheduling::Timer::TimerPeriod timeout)
{
  buffer_.clear();
  if (backoff_count_) {
    LOG::trace_in(
        Utility::authz,
        "successfully fetched authz token after {} back-offs, most recent one of {}",
        backoff_count_,
        backoff_interval_);
  }
  backoff_count_ = 0;
  backoff_interval_ = AUTHZ_INITIAL_ERROR_BACKOFF;
  auto const when = add_jitter(timeout, -AUTHZ_JITTER, AUTHZ_JITTER);
  timer_.defer(when).on_error(
      [when](auto const &error) { LOG::error("failed to schedule next authz token fetch {} from now: {}", when, error); });
}

void AuthzFetcher::ScheduledFetch::schedule_backoff()
{
  auto const backoff = backoff_interval_;
  LOG::error(
      "authz token refresh {}-th back-off after error, trying again in {}",
      backoff_count_,
      std::chrono::duration_cast<std::chrono::seconds>(backoff));

  // bounded geometric back-off on subsequent requests
  ++backoff_count_;
  backoff_interval_ = std::min(backoff_interval_ * AUTHZ_BACKOFF_GEOMETRIC_RATIO, AUTHZ_LONGEST_ERROR_BACKOFF);

  timer_.defer(backoff).on_error([&](auto const &error) {
    LOG::error("failed to schedule next authz token fetch after {}-th back-off of {}: {}", backoff_count_, backoff, error);
  });
}

void AuthzFetcher::ScheduledFetch::stop()
{
  timer_.stop();
}

void AuthzFetcher::auto_refresh(uv_loop_t &loop, SuccessCallback on_success)
{
  assert(!scheduled_);

  auto const calculate_delay = [](Expected<AuthzToken, std::runtime_error> const &token) {
    auto const now = AuthzToken::clock::now().time_since_epoch();
    auto const delay = std::chrono::duration_cast<scheduling::Timer::TimerPeriod>(
        token ? token->expiration() - AUTHZ_NOTICE_PERIOD - now : AUTHZ_INITIAL_ERROR_BACKOFF);

    return delay.count() < 0 ? AUTHZ_INITIAL_ERROR_BACKOFF : delay;
  };

  scheduled_.emplace(
      loop,
      api_url_,
      secret_header_,
      agent_id_header_,
      curl_,
      [this, on_success = std::move(on_success), &calculate_delay](std::string_view body) {
        auto const previous_expiration = token_ ? token_->expiration() : AuthzToken::duration::zero();

        if (token_ = AuthzToken::decode_json(body)) {
          auto const now = std::chrono::system_clock::now().time_since_epoch();
          LOG::trace_in(
              Utility::authz,
              "successfully decoded new authz token issued at {} expiring on {}, {} from now",
              std::chrono::duration_cast<std::chrono::seconds>(token_->issued_at()),
              std::chrono::duration_cast<std::chrono::seconds>(token_->expiration()),
              std::chrono::duration_cast<std::chrono::milliseconds>(token_->expiration() - now));

          auto const time_left = on_success
                                     ? on_success(*token_)
                                     : !previous_expiration.count()
                                           ? std::chrono::milliseconds::zero()
                                           : std::chrono::duration_cast<std::chrono::milliseconds>(previous_expiration - now);

          scheduled_->schedule(calculate_delay(token_));
          // TODO: implement adaptive notice period calculation based on history of `time_left`
          if (time_left.count() < 0) {
            LOG::warn("replaced expired authz token that had been stale for {}", -time_left);
          }
        } else {
          // decode failure, but network request was successfull
          LOG::error(
              "failed to decode authz token, backing off and trying again - error: '{}' - contents: '{}'",
              token_.error(),
              body);
          scheduled_->schedule_backoff();
        }
      },
      [this] { scheduled_->schedule_backoff(); },
      proxy_);

  scheduled_->schedule(calculate_delay(token_));
}

void AuthzFetcher::stop_auto_refresh()
{
  if (!scheduled_.has_value()) {
    return;
  }
  scheduled_->stop();
}

Expected<AuthzToken, std::runtime_error> AuthzFetcher::sync_fetch()
{
  LOG::trace_in(Utility::authz, "fetching authz token from '{}'", api_url_);
  RestfulFetcher fetcher{{secret_header_, agent_id_header_}};

  if (proxy_) {
    LOG::trace_in(Utility::authz, "setting proxy {}:{} for sync fetch", proxy_->host(), proxy_->port());
    fetcher.http_proxy(proxy_->host(), try_integer_from_string<std::uint16_t>(proxy_->port().c_str()));
  } else {
    LOG::trace_in(Utility::authz, "not using a proxy for async fetch");
  }

  return fetcher.sync_fetch<AuthzToken>(
      "initial authz token",
      api_url_,
      AuthzToken::decode_json,
      0ms,
      AUTHZ_MAX_INITIAL_FETCH_RETRIES,
      AUTHZ_INITIAL_ERROR_BACKOFF,
      AUTHZ_LONGEST_ERROR_BACKOFF,
      AUTHZ_BACKOFF_GEOMETRIC_RATIO);
}

Expected<AuthzFetcher::AgentKey, std::string> AuthzFetcher::read_agent_key()
{
  auto key_id = get_env_var(FLOWMILL_AUTH_KEY_ID_VAR);
  auto secret = get_env_var(FLOWMILL_AUTH_SECRET_VAR);

  if (auto key_prefix = key_id.substr(0, AGENT_KEY_PREFIX.size()); key_prefix != AGENT_KEY_PREFIX) {
    if (key_prefix == API_KEY_PREFIX) {
      return {unexpected, WRONG_TYPE_AGENT_KEY_ERROR_MESSAGE};
    } else {
      return {unexpected, INVALID_AGENT_KEY_ERROR_MESSAGE};
    }
  }

  return AgentKey{.key_id = key_id, .secret = secret};
}

namespace {

struct AuthzServerHandler : cli::ArgsParser::Handler {
  AuthzServerHandler(cli::ArgsParser &parser)
      : authz_server(parser.add_arg<std::string>(
            "authz-server",
            "Hostname for the authz server",
            nullptr,
            std::string(try_get_env_var(FLOWMILL_AUTH_SERVER_ENV_VAR, "app.flowmill.com"))))
  {}

  cli::ArgsParser::ArgProxy<std::string> authz_server;
};

} // namespace

cli::ArgsParser::ArgProxy<std::string> &AuthzFetcher::register_args_parser(cli::ArgsParser &parser)
{
  return parser.new_handler<AuthzServerHandler>().authz_server;
}
