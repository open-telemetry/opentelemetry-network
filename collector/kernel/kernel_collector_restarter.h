/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/platform.h>

class KernelCollector;

/**
 * Helper class to facilitate restarting KernelCollector, i.e. when BufferedPoller detects lost BPF samples (PERF_RECORD_LOST).
 * If KernelCollector startup has completed, then a restart request will be processed immediately.
 * If KernelCollector startup has not completed, then the processing of a restart request will be deferred until after
 * KernelCollector startup has completed.
 * If a restart is already in progress, subsequent restart requests will be ignored.
 */
class KernelCollectorRestarter {
  friend class KernelCollector;

public:
  KernelCollectorRestarter(KernelCollector &collector);
  void startup_completed();
  void request_restart();
  void reset();

private:
  void check_restart();

  bool startup_completed_;
  bool restart_requested_;
  bool restart_in_progress_;

  KernelCollector &collector_;
};
