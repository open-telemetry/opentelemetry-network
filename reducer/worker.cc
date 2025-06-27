// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/worker.h>

#include <channel/callbacks.h>
#include <channel/tcp_channel.h>
#include <reducer/ingest/component.h>
#include <reducer/util/thread_ops.h>
#include <util/defer.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/uv_helpers.h>

#include <absl/container/node_hash_map.h>
#include <absl/synchronization/mutex.h>
#include <absl/synchronization/notification.h>
#include <uv.h>

#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>
#include <vector>

namespace reducer {

namespace {

// Decorator class for the user-provided Callbacks to provide some Worker
// class state management.
class WorkerCallbacksDecorator : public ::channel::Callbacks {
public:
  WorkerCallbacksDecorator(
      std::unique_ptr<::channel::Callbacks> underlying_callbacks,
      std::function<void()> close_cb,
      ::channel::TCPChannel *const tcp_channel)
      : underlying_callbacks_(std::move(underlying_callbacks)), close_cb_(std::move(close_cb)), tcp_channel_(tcp_channel)
  {}

  ~WorkerCallbacksDecorator() override = default;

  uint32_t received_data(const uint8_t *const data, const int data_len) override
  {
    return underlying_callbacks_->received_data(data, data_len);
  }

  void on_error(int err) override
  {
    underlying_callbacks_->on_error(err);

    // UV_EOF indicates that the connection has closed.
    if (err == UV_EOF) {
      tcp_channel_->close_permanently();
    }
  }

  void on_closed() override
  {
    underlying_callbacks_->on_closed();
    close_cb_();
  }

  void on_connect() override { underlying_callbacks_->on_connect(); }

  ::channel::Callbacks *underlying_callbacks() { return underlying_callbacks_.get(); }

private:
  std::unique_ptr<::channel::Callbacks> underlying_callbacks_;
  const std::function<void()> close_cb_;
  ::channel::TCPChannel *const tcp_channel_;
};

} // namespace

Worker::Worker()
{
  // Initialize the uv loop.
  CHECK_UV(uv_loop_init(&loop_));

  // Initialize the async for opening tcp sockets.
  CHECK_UV(uv_async_init(&loop_, &open_tcp_socks_async_, &Worker::open_tcp_socks_async_cb));
  open_tcp_socks_async_.data = this;

  // Initialize the stopping async.
  CHECK_UV(uv_async_init(&loop_, &stop_async_, &Worker::stop_async_cb));
  stop_async_.data = this;

  // Initialize the visitor async.
  CHECK_UV(uv_async_init(&loop_, &visit_async_, &Worker::visit_async_cb));
  visit_async_.data = this;
}

Worker::~Worker()
{
  stop();
}

void Worker::start(std::size_t thread_num)
{
  // Start the main thread.
  thread_ = std::thread([this, thread_num] {
    set_self_thread_name(fmt::format("ingest_{}", thread_num)).on_error([=](auto const &error) {
      LOG::warn("unable to set name for ingest core worker thread {}: {}", thread_num, error);
    });
    on_thread_start();
    thread_started_.Notify();
    uv_run(&loop_, UV_RUN_DEFAULT);
    close_uv_loop_cleanly(&loop_);
    on_thread_stop();
  });
  thread_started_.WaitForNotification();
  started_ = true;

  LOG::trace_in(ingest::Component::worker, "Worker {:p} started", (void *)this);
}

void Worker::stop()
{
  if (!started_) {
    return;
  }

  CHECK_UV(uv_async_send(&stop_async_));
  thread_.join();
  started_ = false;
}

void Worker::assign(const uv_tcp_t &tcp_conn)
{
  // Verify that start() has been called.
  if (!started_) {
    LOG::critical("Make sure to call Worker::start() before accepting tcp connections");
    std::exit(1);
  }

  // Verify that the socket descriptor the a file descriptor. This should be
  // true on all unix-based OSs, so this should not fail (unless on Windows or
  // something...)
  static_assert(
      std::is_same<uv_os_fd_t, uv_os_sock_t>::value, "The socket descriptor must be the same type as a file descriptor");
  uv_os_sock_t fd;
  CHECK_UV(uv_fileno(reinterpret_cast<const uv_handle_t *>(&tcp_conn), reinterpret_cast<uv_os_fd_t *>(&fd)));

  // Duplicate the file descriptor and add it to the fd queue.
  const uv_os_sock_t fd_dupe = dup(fd);
  {
    absl::MutexLock l(&mu_);
    tcp_sock_fds_.push_back(fd_dupe);
  }

  // Tell the uv loop to open the TCP connection.
  CHECK_UV(uv_async_send(&open_tcp_socks_async_));

  LOG::trace_in(
      ingest::Component::worker, "Worker {:p}: assigned file descriptor {}", (void *)this, reinterpret_cast<int>(fd_dupe));
}

std::shared_ptr<absl::Notification> Worker::visit_thread(std::function<void()> cb)
{
  // Verify that start() has been called.
  if (!started_) {
    LOG::critical("Make sure to call Worker::start() before visiting the thread.");
    std::terminate();
  }

  // Enqueue the visitor.
  const auto done = std::make_shared<absl::Notification>();
  {
    absl::MutexLock l(&mu_);
    visitors_.push_back(Visitor{
        .cb = std::move(cb),
        .done = done,
    });
  }

  // Trigger the async.
  CHECK_UV(uv_async_send(&visit_async_));

  return done;
}

std::shared_ptr<absl::Notification> Worker::visit_callbacks(CallbacksCb cb)
{
  return visit_thread([this, captured_cb = std::move(cb)] {
    for (auto &kv : tcp_channel_to_payload_) {
      auto *const callbacks_decorator = static_cast<WorkerCallbacksDecorator *>(kv.second.callbacks.get());
      captured_cb(callbacks_decorator->underlying_callbacks());
    }
  });
}

std::unique_ptr<channel::Callbacks> Worker::create_callbacks(uv_loop_t &loop, ::channel::TCPChannel * /* unused */)
{
  return std::make_unique<channel::Callbacks>();
}

void Worker::open_tcp_socks_async_cb(uv_async_t *const handle)
{
  auto *const worker = reinterpret_cast<Worker *>(handle->data);

  // Get the pending list of tcp sockets file descriptors.
  std::vector<uv_os_sock_t> tcp_sock_fds;
  {
    absl::MutexLock l(&worker->mu_);
    std::swap(worker->tcp_sock_fds_, tcp_sock_fds);
  }

  // Create a new tcp connection for each socket fd.
  for (const uv_os_sock_t &fd : tcp_sock_fds) {
    TcpPayload payload;

    // Instantiate the TCP channel.
    payload.tcp_channel = std::make_unique<::channel::TCPChannel>(worker->loop_);
    auto *const tcp_channel_ptr = payload.tcp_channel.get();

    // Create the callbacks.
    payload.callbacks = std::make_unique<WorkerCallbacksDecorator>(
        worker->create_callbacks(worker->loop_, tcp_channel_ptr),
        [worker, tcp_channel_ptr] { worker->tcp_channel_to_payload_.erase(tcp_channel_ptr); },
        tcp_channel_ptr);

    // Store the payload.
    TcpPayload *const payload_ptr = &worker->tcp_channel_to_payload_.emplace(tcp_channel_ptr, std::move(payload)).first->second;

    payload_ptr->callbacks->on_connect();

    // Start accepting messages.
    tcp_channel_ptr->open_fd(*payload_ptr->callbacks, fd);
  }
}

void Worker::stop_async_cb(uv_async_t *const handle)
{
  auto *const worker = reinterpret_cast<Worker *>(handle->data);
  uv_stop(&worker->loop_);
}

void Worker::visit_async_cb(uv_async_t *const handle)
{
  auto *const worker = reinterpret_cast<Worker *>(handle->data);
  worker->invoke_visitors();
}

void Worker::invoke_visitors()
{
  // NOTE: calling empty() here outside of a mutex lock is acceptable.
  if (visitors_.empty()) {
    return;
  }

  // Get the list of pending visitors.
  std::vector<Visitor> visitors;
  {
    absl::MutexLock l(&mu_);
    std::swap(visitors, visitors_);
  }

  // Invoke each and notify the corresponding `visit_thread` calls to unblock.
  for (Visitor &visitor : visitors) {
    visitor.cb();
    visitor.done->Notify();
  }
}

} // namespace reducer
