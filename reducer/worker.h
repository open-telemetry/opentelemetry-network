/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "channel/callbacks.h"
#include "channel/tcp_channel.h"

#include "absl/base/thread_annotations.h"
#include <absl/container/node_hash_map.h>
#include <absl/synchronization/mutex.h>
#include <absl/synchronization/notification.h>

#include <uv.h>

#include <memory>
#include <thread>
#include <vector>

namespace reducer {

// This class is responsible for accepting TCP connections (presumably from a
// libuv-based TCP server) and running them on a different `uv_loop_t`
// instance. This can be used to distribute the work of several TCP connections
// across multiple threads.
class Worker {
public:
  // The size of the internally allocated buffer in which messages are stored.
  static const std::size_t kBufferSize = 64 << 10; // 64 KiB

  Worker();
  virtual ~Worker();

  // Starts the event-processing thread for this worker
  void start(std::size_t thread_num);

  // Stops the currently-running thread, and blocks until the stop has
  // completed.
  void stop();

  // Assigns the provided TCP connection to this worker. This function
  // duplicates the connections file descriptor, and it is the responsibility of
  // the caller to close the original connection. `uv_accept` must already have
  // been called on `tcp_conn` before passing it here.
  // Requires that `start()` was already called.
  void assign(const uv_tcp_t &tcp_conn);

  // Invokes the provided callback in the context of this class's worker thread.
  // Can be used to inspect thread-local values for this class's owned thread.
  // Must not be invoked from within this class's thread itself.
  // Requires that `start()` was already called.
  // This function will run asynchronously, use the returned notification for
  // blocking. For example:
  //
  //   Worker* worker = ...;
  //   worker->visit_thread([] {
  //     /* Do stuff in the worker thread context */
  //   })->WaitForNotification();
  //
  [[nodiscard]] std::shared_ptr<absl::Notification> visit_thread(std::function<void()> cb);

  // Same as above, but invokes the provided callback on the callbacks of each
  // of this class's connections.
  using CallbacksCb = std::function<void(::channel::Callbacks *)>;
  [[nodiscard]] std::shared_ptr<absl::Notification> visit_callbacks(CallbacksCb cb);

  // Invokes all registered visitor callbacks.
  void invoke_visitors();

protected:
  // Invoked when the internal thread has been started or before it stops.
  // Each of these will be invoked within the thread itself.
  virtual void on_thread_start() {}
  virtual void on_thread_stop() {}

  // Returns a set of callbacks to be invoked. Each time a new connection
  // arrives it will be assigned a new set of callbacks provided by this
  // function.
  virtual std::unique_ptr<channel::Callbacks> create_callbacks(uv_loop_t &loop, ::channel::TCPChannel *tcp_channel);

private:
  // Callbacks used by libuv.
  static void open_tcp_socks_async_cb(uv_async_t *handle);
  static void stop_async_cb(uv_async_t *handle);
  static void visit_async_cb(uv_async_t *handle);

  // The contents of the `data` pointer in a uv_tcp_t object.
  struct TcpPayload {
    std::unique_ptr<::channel::TCPChannel> tcp_channel;
    std::unique_ptr<channel::Callbacks> callbacks;
  };

  // The queued entry for `visit_thread` calls.
  struct Visitor {
    std::function<void()> cb;
    std::shared_ptr<absl::Notification> done;
  };

  // The loop that accepts messages on behalf of this worker.
  uv_loop_t loop_;

  // The thread that runs `loop_`.
  std::thread thread_;

  // A mapping of each tcp connection to its payload.
  absl::node_hash_map<::channel::TCPChannel *, TcpPayload> tcp_channel_to_payload_;

  // Various fields that deal with the queueing and processing of
  // newly-assigned TCP sockets.
  uv_async_t open_tcp_socks_async_;
  std::vector<uv_os_sock_t> tcp_sock_fds_ ABSL_GUARDED_BY(mu_);
  mutable absl::Mutex mu_;

  // The queue of visitors.
  uv_async_t visit_async_;
  std::vector<Visitor> visitors_ ABSL_GUARDED_BY(mu_);

  // The async used to stop this thread.
  uv_async_t stop_async_;

  // Notification to determine when the worker thread has begun.
  bool started_ = false;
  absl::Notification thread_started_;
};

} // namespace reducer
