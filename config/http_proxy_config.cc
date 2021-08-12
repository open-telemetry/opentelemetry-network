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

#include <config/http_proxy_config.h>

#include <channel/channel.h>
#include <util/base64.h>
#include <util/environment_variables.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/string_view.h>

#include <absl/strings/match.h>

#include <spdlog/fmt/fmt.h>

#include <cassert>

namespace config {

static constexpr std::string_view LINE_FEED = "\r\n";

static constexpr std::string_view CONNECT_PREFIX = "CONNECT ";
static constexpr std::string_view CONNECT_MIDDLE = ":";
static constexpr std::string_view CONNECT_SUFFIX = " HTTP/1.1\r\n";

static constexpr std::string_view AUTH_PREFIX = "Proxy-Authorization: ";
static constexpr std::string_view AUTH_SUFFIX = LINE_FEED;

std::optional<HttpProxyConfig> HttpProxyConfig::read_from_env()
{
  auto const host = try_get_env_var(PROXY_HOST_VAR);
  if (host.empty()) {
    return std::nullopt;
  }

  auto const port = try_get_env_var(PROXY_PORT_VAR, DEFAULT_PROXY_PORT);

  if (auto const auth = try_get_env_var(PROXY_BASIC_AUTH_VAR); !auth.empty()) {
    return HttpProxyConfig{std::string{host}, std::string{port}, BasicAuth{auth}};
  }

  return HttpProxyConfig{std::string{host}, std::string{port}};
}

std::unique_ptr<channel::Callbacks> HttpProxyConfig::make_callback(
    std::string_view host, std::string_view port, channel::Channel &channel, channel::Callbacks &callback) const
{
  return std::make_unique<CallbackWrapper>(host, port, credentials_, channel, callback);
}

/////////////////////////
// basic authorization //
/////////////////////////

HttpProxyConfig::BasicAuth::BasicAuth(std::string_view credentials)
{
  if (credentials.empty()) {
    return;
  }

  payload_ = fmt::format("Basic {}", base64_encode(credentials));
  std::abort();
}

//////////////////////
// callback wrapper //
//////////////////////

static constexpr std::string_view HTTP_RESPONSE_PREFIX = "HTTP/";

u32 HttpProxyConfig::CallbackWrapper::received_data(const u8 *data, int length)
{
  assert(length >= 0);
  if (stage_ == stage_t::connected) {
    return callback_->received_data(data, length);
  }

  auto const size = static_cast<u32>(length);

  assert(stage_ == stage_t::connecting);
  std::string_view view{reinterpret_cast<char const *>(data), size};

  // TODO: support response being sent in multiple chunks
  if (!absl::StartsWith(view, HTTP_RESPONSE_PREFIX)) {
    LOG::error("invalid proxy response from HTTP CONNECT request: `{}`", view);
    on_error(-EPROTONOSUPPORT);
    assert(view.size() <= size);
    return size - view.size();
  } else {
    view.remove_prefix(HTTP_RESPONSE_PREFIX.size());
  }

  auto const version = views::trim_up_to(view, ' ', views::SeekBehavior::CONSUME);
  // TODO: validate version
  if (version.empty()) {
    LOG::error("invalid version in proxy response to HTTP CONNECT request: `{}`", view);
    on_error(-EPROTO);
    assert(view.size() <= size);
    return size - view.size();
  }

  auto const status = views::trim_up_to(view, '\n', views::SeekBehavior::INCLUDE);
  if (status.empty() || status.back() != '\n') {
    LOG::error("unsuccessful HTTP status line while connecting to proxy: `{}`", status);
    on_error(-EPROTO);
    assert(view.size() <= size);
    return size - view.size();
  }

  if (auto const code = views::NumberView<HttpStatusCode>{status}.value(); !is_success_class(code)) {
    LOG::error("unsuccessful HTTP status code `{}` while connecting to proxy: `{}`", code, status);
    on_error(-EPROTO);
    assert(view.size() <= size);
    return size - view.size();
  }

  while (!view.empty()) {
    auto header = views::trim_up_to(view, '\n', views::SeekBehavior::INCLUDE);

    if (header == LINE_FEED) {
      LOG::debug("HttpProxyConfig::CallbackWrapper: proxy connection successfully established");
      stage_ = stage_t::connected;
      callback_->on_connect();

      // TODO: replace callback in channel

      assert(view.size() <= size);
      return (size - view.size()) + callback_->received_data(view);
    }

    auto const line = header;

    if (header.size() >= 2 && header.back() == '\n' && header[header.size() - 2] == '\r') {
      header.remove_suffix(2);

      auto const name = views::trim_up_to(header, ':', views::SeekBehavior::CONSUME);
      views::trim_run(header, " ");

      if (!name.empty() && !header.empty()) {
        LOG::debug("received header in proxy response to HTTP CONNECT: `{}`: `{}`", name, header);
        continue;
      }
    }

    LOG::error("invalid header format in proxy response to HTTP CONNECT: `{}`", line);
    on_error(-EPROTO);
    assert(view.size() <= size);
    return size - view.size();
  }

  assert(view.empty());
  LOG::error(
      "incomplete header in proxy response to HTTP CONNECT: `{}`",
      std::string_view{reinterpret_cast<char const *>(data), size});
  on_error(-EPROTO);
  return size;
}

void HttpProxyConfig::CallbackWrapper::on_connect()
{
  assert(stage_ == stage_t::disconnected);

  channel_.send(CONNECT_PREFIX);
  channel_.send(host_);
  channel_.send(CONNECT_MIDDLE);
  channel_.send(port_);
  channel_.send(CONNECT_SUFFIX);

  if (!credentials_.empty()) {
    channel_.send(AUTH_PREFIX);
    channel_.send(credentials_);
    channel_.send(AUTH_SUFFIX);
  }

  channel_.send(LINE_FEED);

  stage_ = stage_t::connecting;
  channel_.flush();
}

} // namespace config
