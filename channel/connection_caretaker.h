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
#include <channel/channel.h>
#include <common/client_type.h>
#include <config/config_file.h>
#include <generated/flowmill/ingest/writer.h>
#include <util/authz_fetcher.h>
#include <util/aws_instance_metadata.h>
#include <util/curl_engine.h>
#include <util/gcp_instance_metadata.h>

#include <uv.h>

#include <chrono>
#include <functional>

namespace channel {

// ConnectionCaretaker handles common tasks of agent->server connection.
//
// Current implementation does followings:
//   1. Sends back initial metadata, including agent version, api key &
//      configuration labels.
//   2. Sends back heartbeat signal to server periodically.
//
// This class is NOT thread-safe.
class ConnectionCaretaker {
public:
  using config_data_map = std::unordered_map<std::string, std::string>;

  // Constructor
  //
  // |config_data|: Configuration labels, read from a yaml file.
  // |loop|: Libuv event loop.
  // |channel|: Underline channel connecting agent and server.
  // |heartbeat_interval|: How often a heartbeat signal is sent back to server.
  // |flush_cb|: Callback to flush any downstream buffer.
  ConnectionCaretaker(
      std::string_view hostname,
      ClientType client_type,
      AuthzFetcher &authz_fetcher,
      config::ConfigFile::LabelsMap const &config_data,
      uv_loop_t *loop,
      flowmill::ingest::Writer &writer,
      std::chrono::milliseconds metadata_timeout,
      std::chrono::milliseconds heartbeat_interval,
      std::function<void()> flush_cb,
      std::function<void(bool)> set_compression_cb,
      std::function<void()> on_authenticated_cb);

  ~ConnectionCaretaker();

  // Note, this function triggers metadata to be sent back, and starts
  // heartbeat signal.
  void set_connected();

  // Note, this function stops heartbeat timer implicitly
  void set_disconnected();

  // Sends one heartbeat. It is public so that timer callback can use it.
  void send_heartbeat();

  // Forces a synchronous refresh of the authz token if it's expired or due to
  // expire within the notice period.
  void refresh_authz_token();

private:
  // Sends following information:
  //   agent verstion, api_key (including tenant) and any config labels.
  // TODO: Send agent type as well.
  void send_metadata_header();
  void start_heartbeat();
  void stop_heartbeat();

  void flush();

  std::string_view const hostname_;
  ClientType const client_type_;

  AuthzFetcher &authz_fetcher_;
  const config::ConfigFile::LabelsMap config_data_;

  uv_loop_t *loop_ = nullptr; // not owned

  std::optional<AwsMetadata> aws_metadata_;
  std::optional<GcpInstanceMetadata> gcp_metadata_;

  const std::chrono::milliseconds heartbeat_interval_;

  std::function<void()> flush_cb_;
  std::function<void(bool)> set_compression_cb_;
  std::function<void()> on_authenticated_cb_;

  uv_timer_t heartbeat_timer_;

  flowmill::ingest::Writer &writer_;
};

} // namespace channel
