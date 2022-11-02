// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "signal_handler.h"
#include "thread_ops.h"

#include <util/log.h>
#include <util/log_formatters.h>
#include <util/uv_helpers.h>

#include <signal.h>
#include <stdexcept>

namespace reducer {

SignalHandler::SignalHandler()
{
  CHECK_UV(uv_loop_init(&loop_));

  CHECK_UV(uv_async_init(&loop_, &stop_async_, &on_stop_async));

  stop_async_.data = this;
}

void SignalHandler::run()
{
  set_self_thread_name("signal_handler").on_error([](auto const &error) {
    LOG::warn("unable to set name for signal handler thread: {}", error);
  });
  uv_run(&loop_, UV_RUN_DEFAULT);
}

void SignalHandler::stop_async()
{
  CHECK_UV(uv_async_send(&stop_async_));
}

void SignalHandler::handle(int signum, Callback callback)
{
  std::lock_guard<std::mutex> lock(mutex_);

  uv_signal_t &handler = handlers_[signum];

  CHECK_UV(uv_signal_init(&loop_, &handler));

  handler.data = this;

  CHECK_UV(uv_signal_start(&handler, on_signal, signum));

  callbacks_[signum] = callback;
}

void SignalHandler::on_stop_async(uv_async_t *handle)
{
  auto obj = reinterpret_cast<SignalHandler *>(handle->data);
  uv_stop(&obj->loop_);
}

void SignalHandler::on_signal(uv_signal_t *handle, int signum)
{
  auto obj = reinterpret_cast<SignalHandler *>(handle->data);
  obj->on_signal(signum);
}

void SignalHandler::on_signal(int signum)
{
  LOG::info("Caught signal {}", signum);

  Callback callback;

  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (auto it = callbacks_.find(signum); it != callbacks_.end()) {
      callback = callbacks_[signum];
    }
  }

  if (callback) {
    callback(signum);
  }
}

} // namespace reducer
