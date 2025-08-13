/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <memory>

class ProbeHandler;
struct render_bpf_bpf;

class ProcessProber {
public:
  ProcessProber(
      ProbeHandler &probe_handler,
      struct render_bpf_bpf *skel,
      std::function<void(void)> periodic_cb,
      std::function<void(std::string)> check_cb);

private:
  /**
   * Iterates over /proc and triggers calls to get_pid_task
   */
  void trigger_get_pid_task(std::function<void(void)> periodic_cb);
};
