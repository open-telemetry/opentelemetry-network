// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/signal_handler.h>

#include <util/log.h>
#include <util/log_formatters.h>

#include <cassert>
#include <csignal>
#include <cstdlib>
#include <stdexcept>
#include <sys/resource.h>

// Minimal SignalManager implementation without Breakpad/minidump support.

SignalManager::SignalManager(::uv_loop_t &loop, std::string_view /*product*/) : loop_(loop) {}

void SignalManager::handle()
{
  // Disable core dumps to keep prior behavior consistent.
  struct rlimit const core_dump_limit = {.rlim_cur = 0, .rlim_max = 0};
  ::setrlimit(RLIMIT_CORE, &core_dump_limit);

  // Ignore SIGPIPE as libuv docs recommend for I/O robustness.
  ::signal(SIGPIPE, SIG_IGN);
}

void SignalManager::handle_signals(std::initializer_list<int> signal_numbers, std::function<void()> on_signal)
{
  for (auto const signal_number : signal_numbers) {
    signals_.emplace_back(*this, on_signal, signal_number);
  }
}

void SignalManager::clear()
{
  signals_.clear();
}

////////////////////
// signal_handler //
////////////////////

void SignalManager::SignalHandler::signal_handler(uv_signal_t *handle, int signal_number)
{
  auto &handler = *reinterpret_cast<SignalManager::SignalHandler *>(handle->data);
  assert(signal_number == handler.signal_number());
  handler.on_signal();
}

SignalManager::SignalHandler::SignalHandler(SignalManager &manager, std::function<void()> on_signal, int signal_number)
    : manager_(manager), on_signal_(std::move(on_signal))
{
  if (auto const error = ::uv_signal_init(&manager_.loop(), &handler_)) {
    throw std::runtime_error(fmt::format("Could not init handler for signal {}", signal_number));
  }

  handler_.data = this;

  if (auto const error = ::uv_signal_start(&handler_, signal_handler, signal_number)) {
    throw std::runtime_error(fmt::format("Could not start handler for signal {}", signal_number));
  }
}

static void signal_handle_close_cb(uv_handle_t * /*handle*/)
{
  LOG::debug("Closed a signal handler handle");
}

SignalManager::SignalHandler::~SignalHandler()
{
  ::uv_signal_stop(&handler_);
  ::uv_close(reinterpret_cast<uv_handle_t *>(&handler_), signal_handle_close_cb);
}

void SignalManager::SignalHandler::on_signal()
{
  LOG::info("Caught signal {}", handler_.signum);

  // call on_signal callback
  if (on_signal_) {
    on_signal_();
  }

  manager_.clear();

  ::exit(-handler_.signum);
}
