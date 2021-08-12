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
#include <channel/connection_caretaker.h>
#include <channel/reconnecting_channel.h>
#include <channel/tls_channel.h>
#include <generated/flowmill/aws_collector/index.h>
#include <generated/flowmill/ingest/writer.h>

#include <uv.h>

#include <chrono>
#include <functional>

namespace collector::aws {

class IngestConnection : channel::Callbacks {
public:
  IngestConnection(
      std::string_view hostname,
      ::uv_loop_t &loop,
      std::chrono::milliseconds aws_metadata_timeout,
      std::chrono::milliseconds heartbeat_interval,
      config::IntakeConfig intake_config,
      AuthzFetcher &authz_fetcher,
      std::size_t buffer_size,
      channel::Callbacks &connection_callback,
      std::function<void()> on_authenticated_cb);

  void connect();
  void flush();

  flowmill::ingest::Writer &writer() { return writer_; }

  flowmill::aws_collector::Index &index() { return index_; }

private:
  u32 received_data(const u8 *data, int data_len);
  void on_error(int err);
  void on_closed();
  void on_connect();

  channel::TLSChannel::Initializer tls_guard_;
  std::unique_ptr<CurlEngine> curl_;
  channel::ReconnectingChannel channel_;
  channel::Callbacks &connection_callback_;
  std::unique_ptr<::flowmill::ingest::Encoder> encoder_;
  flowmill::ingest::Writer writer_;
  channel::ConnectionCaretaker caretaker_;
  flowmill::aws_collector::Index index_;
};

} // namespace collector::aws
