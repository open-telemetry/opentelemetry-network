/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <collector/kernel/perf_reader.h>
#include <platform/platform.h>

class PerfPoller {
public:
  /**
   * c'tor
   */
  PerfPoller(PerfContainer &container);

  /**
   * d'tor
   */
  virtual ~PerfPoller();

  /**
   * poll method, to be implemented by child class.
   *
   * Can use container_->...
   */
  virtual void poll() = 0;

  /**
   * Perform @n_intervals pollings, every @interval_useconds microseconds
   */
  void start(u64 interval_useconds, u64 n_intervals);

protected:
  PerfContainer &container_;
};
