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

#include <channel/callbacks.h>
#include <channel/channel.h>
#include <channel/upstream_connection.h>
#include <config/intake_config.h>

#include <set>

namespace channel {

// ReconnectingChannel manages the connection to a remote server.
//
// Retries connection when network error occurs.
//
// Note that this class is NOT thread safe.
class ReconnectingChannel : public Channel, public Callbacks {
public:
  ReconnectingChannel(config::IntakeConfig intake_config, uv_loop_t &loop, std::size_t buffer_size);
  ~ReconnectingChannel() final;

  // Registers/unregisters a observer.
  //
  // They are expected to use during initialization or clean-up phase.
  void register_pipeline_observer(Callbacks *cb);
  void unregister_pipeline_observer(Callbacks *cb);

  // Enables or disables compression.
  void set_compression(bool enabled);

  // Channel interface
  std::error_code send(const u8 *data, int data_len) override;

  // Callbacks interface.
  u32 received_data(const u8 *data, int data_len) override;
  void on_error(int err) override;
  void on_closed() override;
  void on_connect() override;

  // Starts the connection to remote server.
  //
  // It can only be called once.
  void start_connect();

  BufferedWriter &buffered_writer();

  void close() override;

  // Flushes and sends out any remaining messages in the send buffer.
  std::error_code flush() override;

  enum class State : int { INACTIVE, CONNECTING, CONNECTED, CLOSING, BACKOFF };

  const char *state_string() const;
  State state() const;

  config::IntakeConfig const &intake_config() const { return intake_config_; }

  bool is_open() const override { return upstream_connection_.is_open(); }

private:
  friend void start_timer_cb(uv_timer_t *timer);

  // How long we should wait for the connection to be established, before
  // it times out and reconnects again.
  static constexpr u64 connection_timeout_ms_ = 10000;

  // Starts the connection_timer_ to track if the TCP connection is
  // established within |connection_timeout_ms_|
  void to_connecting_state();

  // Starts the start_timer_, to let the service sleep for certain period
  // of time before start a new connection.
  void to_backoff_state();

  // Closes the TCP connection, and timers.
  void to_closing_state();

  // Stops all active timers
  void stop_all_timers();

  // Returns how much time the system should wait, in microsecond,
  // before it tries to start a new connection.
  u64 get_start_wait_time() const;

  // UV loop that this object runs on.
  uv_loop_t &loop_;

  // Intake endpoint config
  config::IntakeConfig const intake_config_;

  // Handles low-level TLS, TCP connection.
  std::unique_ptr<NetworkChannel> network_channel_;
  UpstreamConnection upstream_connection_;

  // Current state of the pipeline.
  State state_;

  // The timer to clock the waiting period before TCP connection restarts.
  // ([INACTIVE | BACKOFF] -> CONNECTING)
  uv_timer_t start_timer_;

  // The timer to track if connection is establish successfully.
  // (CONNECTING -> CONNECTED)
  uv_timer_t connection_timer_;

  // Observers interested in the status of the pipeline.
  std::set<Callbacks *> pipeline_observers_;

  // Number of bytes this channel has sent, or is about to send, back to
  // flowmill pipeline server.
  u64 num_bytes_sent_ = 0;
};

} // namespace channel
