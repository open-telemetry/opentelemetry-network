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

#include <uv.h>

#include <collector/k8s/resync_queue_interface.h>

#include <channel/buffered_writer.h>
#include <channel/callbacks.h>
#include <channel/connection_caretaker.h>
#include <config/config_file.h>
#include <generated/flowmill/ingest/writer.h>
#include <util/curl_engine.h>

namespace channel {
class ReconnectingChannel;
} // namespace channel

namespace collector {

class ResyncProcessor : public ::channel::Callbacks {
public:
  // Note, does not take ownership any arguments.
  ResyncProcessor(
      uv_loop_t &loop,
      ResyncQueueConsumerInterface *resync_queue,
      channel::ReconnectingChannel &reconnecting_channel,
      config::ConfigFile const &configuration_data,
      std::string_view hostname,
      AuthzFetcher &auth_fetcher,
      std::chrono::milliseconds aws_metadata_timeout,
      std::chrono::seconds heartbeat_interval,
      std::size_t write_buffer_size);
  ~ResyncProcessor();

  // Polling routines
  void poll();
  void start_poll_timer();
  void stop_poll_timer();

  void on_error(int err) override;
  void on_connect() override;

private:
  void on_authenticated();

  void send_metadata();

  // How many iterations of processing on each poll.
  static constexpr int num_iterations_per_poll_ = 10;

  // How long we should wait to poll again.
  static constexpr int poll_interval_ms_ = 200;

  uv_loop_t &loop_; // not owned
  uv_timer_t poll_timer_;

  ResyncQueueConsumerInterface *resync_queue_; // not owned
  std::unique_ptr<CurlEngine> curl_engine_;

  channel::ReconnectingChannel &reconnecting_channel_; // not owned
  std::unique_ptr<::flowmill::ingest::Encoder> encoder_;
  flowmill::ingest::Writer writer_;
  channel::ConnectionCaretaker caretaker_;

  // The minimal resync count with which the packet can be sent back to
  // pipeline server;
  u64 active_resync_;

  // Whether we have sent at least 1 packet back to pipeline server at current
  // active_resync_ value.
  //
  // This is to optimize the resync logic.
  //
  // If we have sent something back to pipeline server, a reset on the libuv
  // side will cause a reset in gRpc side, and vice versa. However, if no
  // communication has happened yet. Either side can reset without causing other
  // side to follow.
  bool dirty_;

  // How many times we poll.
  u64 poll_count_ = 0;

  // How many messages sent since last resync.
  u64 message_count_ = 0;
}; // class RsyncProcessor

} // namespace collector
