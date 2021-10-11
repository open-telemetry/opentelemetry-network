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

#include <channel/component.h>
#include <channel/reconnecting_channel.h>

#include <util/log.h>

#include <stdexcept>

namespace channel {
namespace {
// Callbacks passed to libuv
void connection_timer_cb(uv_timer_t *timer)
{
  ReconnectingChannel *channel = (ReconnectingChannel *)(timer->data);

  LOG::error("ReconnectingChannel: Connection timeout.");
  channel->on_error(UV_ETIMEDOUT);
}
} // namespace

void start_timer_cb(uv_timer_t *timer)
{
  ReconnectingChannel *channel = (ReconnectingChannel *)(timer->data);
  channel->to_connecting_state();
}

ReconnectingChannel::ReconnectingChannel(config::IntakeConfig intake_config, uv_loop_t &loop, std::size_t buffer_size)
    : loop_(loop),
      intake_config_(std::move(intake_config)),
      network_channel_(intake_config_.make_channel(loop)),
      upstream_connection_(buffer_size, intake_config_.allow_compression(), *network_channel_),
      state_(State::INACTIVE)
{
  int res = uv_timer_init(&loop_, &start_timer_);
  if (res != 0) {
    LOG::error("ReconnectingChannel: Cannot init start_timer");
  }
  start_timer_.data = this;

  res = uv_timer_init(&loop_, &connection_timer_);
  if (res != 0) {
    LOG::error("ReconnectingChannel: Cannot init connection_timer");
  }
  connection_timer_.data = this;
}

ReconnectingChannel::~ReconnectingChannel()
{
  close();
  uv_close((uv_handle_t *)&connection_timer_, NULL);
  uv_close((uv_handle_t *)&start_timer_, NULL);
}

void ReconnectingChannel::register_pipeline_observer(Callbacks *observer)
{
  pipeline_observers_.insert(observer);
}

void ReconnectingChannel::unregister_pipeline_observer(Callbacks *observer)
{
  pipeline_observers_.erase(observer);
}

//// Callbacks interface ////
u32 ReconnectingChannel::received_data(const u8 *data, int data_len)
{
  // Do nothing for now.
  return data_len;
}

void ReconnectingChannel::on_error(int err)
{
  LOG::trace_in(Component::reconnecting_channel, "ReconnectingChannel: on_error(). prev_state: {}", state_string());
  LOG::warn("ReconnectingChannel: Connection error: {}", uv_err_name(err));
  for (auto *observer : pipeline_observers_) {
    observer->on_error(err);
  }

  to_closing_state();
}

void ReconnectingChannel::on_closed()
{
  LOG::trace_in(Component::reconnecting_channel, "ReconnectingChannel: on_closed(). State: {}", state_string());

  for (auto *observer : pipeline_observers_) {
    observer->on_closed();
  }

  to_backoff_state();
}

void ReconnectingChannel::on_connect()
{
  LOG::trace_in(Component::reconnecting_channel, "ReconnectingChannel: on_connect(). State: {}", state_string());
  LOG::info("ReconnectingChannel: Remote connection established.");

  num_bytes_sent_ = 0;
  set_compression(false);

  stop_all_timers();
  assert(state_ == State::CONNECTING);
  state_ = State::CONNECTED;

  for (auto *observer : pipeline_observers_) {
    observer->on_connect();
  }
}

void ReconnectingChannel::set_compression(bool enabled)
{
  upstream_connection_.set_compression(enabled);
}

//// Channel interface ////
std::error_code ReconnectingChannel::send(const u8 *data, int data_len)
{
  if (state_ != State::CONNECTED) {
    LOG::trace_in(Component::reconnecting_channel, "ReconnectingChannel: Attempt to send when the channel is NOT connected.");
    return std::make_error_code(std::errc::not_connected);
  }

  auto &buffered_writer = upstream_connection_.buffered_writer();

  num_bytes_sent_ += data_len;
  LOG::trace_in(
      Component::reconnecting_channel,
      "Sending ReconnectingChannel: {} bytes. {} bytes sent in total",
      data_len,
      num_bytes_sent_);

  auto buffer = buffered_writer.start_write(data_len);
  if (!buffer) {
    LOG::error("ReconnectingChannel: buffered writer overflow: {}", buffer.error());
    return buffer.error();
  }
  memcpy(*buffer, data, data_len);
  buffered_writer.finish_write();
  return {};
}

BufferedWriter &ReconnectingChannel::buffered_writer()
{
  return upstream_connection_.buffered_writer();
}

void ReconnectingChannel::close()
{
  state_ = State::INACTIVE;
  stop_all_timers();
  upstream_connection_.close();
}

std::error_code ReconnectingChannel::flush()
{
  return upstream_connection_.flush();
}

u64 ReconnectingChannel::get_start_wait_time() const
{
  // TODO: better back-off mechanism here.
  return 1000;
}

void ReconnectingChannel::to_connecting_state()
{
  LOG::trace_in(Component::reconnecting_channel, "ReconnectingChannel: to_connecting_state(). State: {}", state_string());

  state_ = State::CONNECTING;

  try {
    upstream_connection_.connect(*this);
  } catch (std::exception &e) {
    LOG::warn(
        "ReconnectingChannel: Connection attempt failed; will backoff "
        "and retry. Error: {}",
        e.what());
    to_backoff_state();
    return;
  }
  stop_all_timers();
  int res = uv_timer_start(&connection_timer_, connection_timer_cb, connection_timeout_ms_, 0);

  if (res != 0) {
    LOG::error("ReconnectingChannel: Cannot start connection_timer {}", uv_err_name(res));
  }
}

void ReconnectingChannel::to_backoff_state()
{
  LOG::trace_in(Component::reconnecting_channel, "ReconnectingChannel: start_timer(). State: {}", state_string());
  state_ = State::BACKOFF;

  stop_all_timers();
  int res = uv_timer_start(&start_timer_, start_timer_cb, get_start_wait_time(), 0);

  if (res != 0) {
    LOG::error("ReconnectingChannel: Cannot start start_timer {}", uv_err_name(res));
  }
}

void ReconnectingChannel::start_connect()
{
  LOG::trace_in(Component::reconnecting_channel, "ReconnectingChannel: start_connect(). State: {}", state_string());

  assert(state_ == State::INACTIVE);

  to_connecting_state();
}

void ReconnectingChannel::to_closing_state()
{
  state_ = State::CLOSING;
  stop_all_timers();
  try {
    upstream_connection_.close();
  } catch (std::exception &e) {
    LOG::warn("ReconnectingChannel: Cannot close connection: {}", e.what());
  }
  //  to_backoff_state();
}

const char *ReconnectingChannel::state_string() const
{
  switch (state_) {
  case State::INACTIVE:
    return "INACTIVE";
  case State::CONNECTING:
    return "CONNECTING";
  case State::CONNECTED:
    return "CONNECTED";
  case State::CLOSING:
    return "CLOSING";
  case State::BACKOFF:
    return "BACKOFF";
  }

  // Make compiler happy.
  // TODO: remove this line if we switch to clang++
  return "UNKNOWN";
}

ReconnectingChannel::State ReconnectingChannel::state() const
{
  return state_;
}

void ReconnectingChannel::stop_all_timers()
{
  uv_timer_stop(&connection_timer_);
  uv_timer_stop(&start_timer_);
}

} // namespace channel
