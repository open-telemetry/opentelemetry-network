/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/util/virtual_clock.h>

#include <common/client_type.h>

#include <util/element_queue_cpp.h>
#include <util/fast_div.h>

#include <absl/synchronization/notification.h>
#include <uv.h>

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace reducer {

// Base class for core implementations.
//
class Core {
public:
  static constexpr auto STATS_PERIOD = 10s;

  virtual ~Core();

  // Runs the core's execution loop.
  void run();

  // Stops the core's execution loop. Can be run from any thread.
  void stop_async();
  void stop_sync();
  void wait_for_shutdown();

  // This core's application name.
  std::string_view app_name() const { return app_name_; }
  // Returns this core's shard number.
  size_t shard_num() const { return shard_num_; }

  // Flag the connection as authenticated.
  // Used when this core is receiving messages only from in-process sources.
  void set_connection_authenticated();

  // This core's current timestamp.
  u64 current_timestamp() const { return current_timestamp_; }
  // Duration of time slots, in timestamp units.
  double timeslot_duration() const;

  // Returns the current metrics timestamp, for output to a TSDB.
  std::chrono::nanoseconds metrics_timestamp() const;

protected:
  // Subclasses implement to use concrete render-generated classes.
  //
  struct IRpcHandler {
    virtual ~IRpcHandler() {};

    // Handle a received RPC message.
    virtual void handle(u64 current_timestamp, char *buf, size_t len) = 0;
    // Flag connections as authenticated.
    virtual void set_connection_authenticated() = 0;
  };

  // For each RPC client that is sending messages to this core.
  //
  struct RpcClient {
    // Queue to which the client writes and we read from.
    ElementQueue queue;
    // Handles incoming messages received from this client.
    std::unique_ptr<IRpcHandler> handler;
    // Type of this client.
    ClientType client_type;

    RpcClient(ElementQueue queue, std::unique_ptr<IRpcHandler> handler, ClientType client_type);
  };

  // Clients sending RPC messages to this core.
  std::vector<RpcClient> rpc_clients_;

  // The virtual clock driven by messages from RPC clients.
  VirtualClock virtual_clock_;

  Core(std::string_view app_name, size_t shard_num, u64 initial_timestamp);

private:
  // Core instance belonging to the current thread.
  // Assigned in run().
  static thread_local Core *instance_;

  // This core's application name.
  std::string app_name_;
  // This core's shard number.
  size_t shard_num_;

  // Current timestamp.
  // Initialized to initial_timestamp on construction.
  u64 current_timestamp_;

  // Next RPC client to read from.
  size_t next_rpc_client_{0};

  // Libuv loop object.
  uv_loop_t loop_;
  // Async object used for stopping the loop from another thread.
  uv_async_t stop_async_;
  // Notification triggered when the loop is done running.
  absl::Notification done_;

  // Timer that services reading RPC messages from the queue.
  uv_timer_t rpc_timer_;
  // Timer that services writing internal stats to prometheus.
  uv_timer_t stats_timer_;

  // Callback function for stop_async.
  static void on_stop_async(uv_async_t *handle);
  // RPC timer callback.
  static void on_rpc_timer(uv_timer_t *timer);
  // Internal stats timer callback.
  static void on_stats_timer(uv_timer_t *timer);

  // Reads incoming RPC messages that are enqueued in the RPC queue.
  // Forwards received messages to the RPC handler object to handle.
  // If any messages are handled, returns true. Otherwise returns false.
  // Gets invoked periodically by the RPC timer.
  bool handle_rpc();

  // Called when the current timeslot is complete.
  virtual void on_timeslot_complete();

  // Subclasses implement to output internal stats to be scraped by a
  // time-series DB.
  // Gets invoked periodically by the internal stats timer.
  virtual void write_internal_stats();

  template <class T> friend T &local_core();
};

// Returns the core instance belonging to the current thread.
//
template <class T> T &local_core()
{
  assert(Core::instance_);
  return dynamic_cast<T &>(*Core::instance_);
}

} // namespace reducer
