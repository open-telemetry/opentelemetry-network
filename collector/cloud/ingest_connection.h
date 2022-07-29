/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <channel/callbacks.h>
#include <channel/connection_caretaker.h>
#include <channel/reconnecting_channel.h>
#include <generated/flowmill/cloud_collector/index.h>
#include <generated/flowmill/ingest/writer.h>

#include <uv.h>

#include <chrono>
#include <functional>

namespace collector::cloud {

class IngestConnection : channel::Callbacks {
public:
  IngestConnection(
      std::string_view hostname,
      ::uv_loop_t &loop,
      std::chrono::milliseconds aws_metadata_timeout,
      std::chrono::milliseconds heartbeat_interval,
      config::IntakeConfig intake_config,
      std::size_t buffer_size,
      channel::Callbacks &connection_callback,
      std::function<void()> on_connected_cb);

  void connect();
  void flush();

  flowmill::ingest::Writer &writer() { return writer_; }

  flowmill::cloud_collector::Index &index() { return index_; }

private:
  u32 received_data(const u8 *data, int data_len);
  void on_error(int err);
  void on_closed();
  void on_connect();

  std::unique_ptr<CurlEngine> curl_;
  channel::ReconnectingChannel channel_;
  channel::Callbacks &connection_callback_;
  std::unique_ptr<::flowmill::ingest::Encoder> encoder_;
  flowmill::ingest::Writer writer_;
  channel::ConnectionCaretaker caretaker_;
  flowmill::cloud_collector::Index index_;
};

} // namespace collector::cloud
