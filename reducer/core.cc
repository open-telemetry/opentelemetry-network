// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "core.h"

#include <reducer/constants.h>
#include <reducer/util/thread_ops.h>

#include <platform/userspace-time.h>
#include <util/log_formatters.h>
#include <util/time.h>
#include <util/uv_helpers.h>

#include <math.h>

namespace reducer {

thread_local Core *Core::instance_ = nullptr;

Core::Core(std::string_view app_name, size_t shard_num, u64 initial_timestamp)
    : app_name_(app_name), shard_num_(shard_num), current_timestamp_(initial_timestamp)
{
  CHECK_UV(uv_loop_init(&loop_));

  CHECK_UV(uv_async_init(&loop_, &stop_async_, &on_stop_async));
  stop_async_.data = this;

  CHECK_UV(uv_timer_init(&loop_, &rpc_timer_));
  rpc_timer_.data = this;

  CHECK_UV(uv_timer_init(&loop_, &stats_timer_));
  stats_timer_.data = this;
}

Core::~Core() {}

void Core::set_connection_authenticated()
{
  for (auto &rpc_client : rpc_clients_) {
    rpc_client.handler->set_connection_authenticated();
  }
}

double Core::timeslot_duration() const
{
  return virtual_clock_.timeslot_duration();
}

std::chrono::nanoseconds Core::metrics_timestamp() const
{
  // clock timestamps are in nanoseconds
  u64 timestamp = current_timestamp();

  // fraction of the timeslot left after the current timestamp
  double frac = 1.0 - fmod(timestamp / timeslot_duration(), 1);

  // aligned to end of timeslot boundary
  u64 metrics_timestamp = timestamp + (u64)(frac * timeslot_duration());

  return std::chrono::nanoseconds(metrics_timestamp);
}

void Core::run()
{
  // save thread-local instance
  instance_ = this;

  set_self_thread_name(fmt::format("{}_{}", app_name_, shard_num_)).on_error([this](auto const &error) {
    LOG::warn("unable to set name for {} core thread {}: {}", app_name_, shard_num_, error);
  });

  if (!rpc_clients_.empty()) {
    auto repeat = integer_time<std::chrono::milliseconds>(RPC_HANDLE_TIME);
    CHECK_UV(uv_timer_start(&rpc_timer_, on_rpc_timer, repeat, repeat));
  }

  {
    auto repeat = integer_time<std::chrono::milliseconds>(STATS_PERIOD);
    CHECK_UV(uv_timer_start(&stats_timer_, on_stats_timer, repeat, repeat));
  }

  uv_run(&loop_, UV_RUN_DEFAULT);
  close_uv_loop_cleanly(&loop_);

  done_.Notify();
}

void Core::stop_async()
{
  uv_async_send(&stop_async_);
}

void Core::stop_sync()
{
  stop_async();
  wait_for_shutdown();
}

void Core::wait_for_shutdown()
{
  done_.WaitForNotification();
}

void Core::on_stop_async(uv_async_t *handle)
{
  auto core = reinterpret_cast<Core *>(handle->data);

  uv_stop(&core->loop_);
}

void Core::on_rpc_timer(uv_timer_t *timer)
{
  auto core = reinterpret_cast<Core *>(timer->data);

  bool any_handled = core->handle_rpc();

  if (any_handled) {
    // restart the timer immediately
    // otherwise the timer will again fire after the normal repeat time
    uv_timer_start(timer, timer->timer_cb, 0, timer->repeat);
  }
}

void Core::on_stats_timer(uv_timer_t *timer)
{
  auto core = reinterpret_cast<Core *>(timer->data);

  core->write_internal_stats();
}

bool Core::handle_rpc()
{
  // Current monotonic time.
  u64 time_now = monotonic();

  // Time at which we will stop handling messages in this iteration, to allow
  // other timers to trigger when due.
  const u64 timeout_ns = time_now + integer_time<std::chrono::nanoseconds>(RPC_HANDLE_TIME);

  bool any_handled{false};
  bool timed_out{false};

  // Try going through all RPC clients, starting from the next in line.
  for (size_t i = 0; (i < rpc_clients_.size()) && !timed_out; ++i) {
    const auto rpc_client_index = next_rpc_client_;

    // Since handling of RPC messages can be interrupted by a time-out, we
    // have to make sure that on the next try we start from the right place.
    next_rpc_client_ = (next_rpc_client_ + 1) % rpc_clients_.size();

    auto &rpc_client = rpc_clients_[rpc_client_index];

    if (!virtual_clock_.can_update(rpc_client_index)) {
      continue;
    }

    rpc_client.queue.start_read_batch();

    // process queue elements, but not more than kMaxRpcBatchPerQueue
    for (size_t msg_i = 0; (msg_i < kMaxRpcBatchPerQueue) && virtual_clock_.can_update(rpc_client_index) &&
                           (rpc_client.queue.peek() > 0) && !timed_out;
         msg_i++) {
      char *msg_buf{nullptr};
      int msg_len{0};

      // decode the message timestamp
      u64 msg_timestamp;
      if (rpc_client.queue.peek_value(msg_timestamp) < 0) {
        LOG::critical("could not read timestamp from a message");
        // read the message to remove it from the queue but skip processing it
        (void)rpc_client.queue.read(msg_buf);
        continue;
      }

      // update the virtual clock for this client
      if (int r = virtual_clock_.update(rpc_client_index, msg_timestamp); r < 0) {
        if (r == -EINVAL) {
          LOG::critical(
              "{}-{}: out-of-order message from client {} ({}): current_timestamp={}, msg_timestamp={}",
              app_name(),
              shard_num(),
              rpc_client_index,
              to_string(rpc_client.client_type),
              current_timestamp_,
              msg_timestamp);
          throw std::runtime_error(fmt::format("{}-{}: out-of-order message", app_name(), shard_num()));
        } else {
          throw std::runtime_error(fmt::format("{}-{}: unexpected error: {}", app_name(), shard_num(), r));
        }
      }

      if (virtual_clock_.is_current(rpc_client_index)) {
        // update this core's timestamp
        current_timestamp_ = std::max(current_timestamp_, msg_timestamp);
        // read this message
        msg_len = rpc_client.queue.read(msg_buf);
        // pass the message to the message handler
        rpc_client.handler->handle(current_timestamp_, msg_buf, msg_len);
        // yup
        any_handled = true;
      }

      // update the current time
      time_now = monotonic();
      // check for timeout
      timed_out = (time_now >= timeout_ns);
    }

    rpc_client.queue.finish_read_batch();
  }

  if (virtual_clock_.advance()) {
    on_timeslot_complete();
  }

  return any_handled;
}

void Core::on_timeslot_complete() {}

void Core::write_internal_stats() {}

////////////////////////////////////////////////////////////////////////////////

Core::RpcClient::RpcClient(ElementQueue _queue, std::unique_ptr<IRpcHandler> _handler, ClientType _client_type)
    : queue(std::move(_queue)), handler(std::move(_handler)), client_type(_client_type)
{}

} // namespace reducer
