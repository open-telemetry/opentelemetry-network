/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <memory>

class ProbeHandler;
namespace ebpf {
class BPFModule;
}

class ProcessProber {
public:
  ProcessProber(
      ProbeHandler &probe_handler,
      ebpf::BPFModule &bpf_module,
      std::function<void(void)> periodic_cb,
      std::function<void(std::string)> check_cb);

private:
  /**
   * Iterates over /proc and triggers calls to get_pid_task
   */
  void trigger_get_pid_task(std::function<void(void)> periodic_cb);
};
