/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scheduling/timer.h>
#include <util/args_parser.h>

#include <client/linux/handler/exception_handler.h> /* google breakpad */

#include <uv.h>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

struct SignalManager : cli::ArgsParser::Handler {
  explicit SignalManager(cli::ArgsParser &parser, ::uv_loop_t &loop, std::string_view product);

  void handle() override;

  void handle_signals(std::initializer_list<int> signal_numbers, std::function<void()> on_signal = {});

  void clear();

private:
  ::uv_loop_t &loop() { return loop_; }

  void setup_breakpad();
  void handle_minidump();

  ::uv_loop_t &loop_;

  std::string product_;
  std::string module_name_;
  std::string module_id_;

  cli::ArgsParser::FlagProxy disable_crash_report_;
  std::string minidump_dir_;
  cli::ArgsParser::ArgProxy<std::string> minidump_path_;
  google_breakpad::MinidumpDescriptor breakpad_descriptor_;
  std::optional<google_breakpad::ExceptionHandler> breakpad_exception_handler_;
#ifndef NDEBUG
  cli::ArgsParser::FlagProxy crash_;
  cli::ArgsParser::ArgProxy<std::chrono::seconds::rep> schedule_crash_;
#endif

  struct SignalHandler {
    SignalHandler(SignalManager &manager, std::function<void()> on_signal, int signal_number);

    ~SignalHandler();

    void on_signal();

    int signal_number() const { return handler_.signum; }

    static void signal_handler(uv_signal_t *handle, int signal_number);

  private:
    SignalManager &manager_;
    std::function<void()> on_signal_;
    uv_signal_t handler_;
  };

  std::list<SignalHandler> signals_;

#ifndef NDEBUG
  std::unique_ptr<scheduling::Timer> crash_timer_;
#endif
};
