/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <uv.h>

#include <functional>
#include <map>
#include <mutex>

namespace reducer {

// Libuv loop for handling Unix signals.
//
// Use this class instead of handling signals (e.g. SIGINT and SIGTERM) in
// each libuv loop.
//
class SignalHandler {
public:
  using Callback = std::function<void(int)>;

  SignalHandler();
  SignalHandler(const SignalHandler &) = delete;

  // Runs the libuv loop.
  void run();

  // Stops the loop.
  void stop_async();

  // Registers a callback for the specified signal.
  // NOTE: overwrites previously registered callback.
  void handle(int signum, Callback callback);

private:
  // Libuv loop object.
  uv_loop_t loop_;
  // Async object used for stopping the loop from another thread.
  uv_async_t stop_async_;

  // Signal to libuv handler map.
  std::map<int, uv_signal_t> handlers_;
  // Signal to callback map.
  std::map<int, Callback> callbacks_;

  // Mutex object for serializing access to handler and callbacks maps.
  std::mutex mutex_;

  // Callback function for stop_async.
  static void on_stop_async(uv_async_t *handle);

  // Callback function for libuv signal handlers.
  static void on_signal(uv_signal_t *handle, int signum);

  // Called on every registered signal.
  void on_signal(int signum);
};

} // namespace reducer
