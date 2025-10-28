/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <uv.h>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>
#include <list>

struct SignalManager {
  explicit SignalManager(::uv_loop_t &loop, std::string_view product);

  void handle();

  void handle_signals(std::initializer_list<int> signal_numbers, std::function<void()> on_signal = {});

  void clear();

private:
  ::uv_loop_t &loop() { return loop_; }

  ::uv_loop_t &loop_;

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
};
