/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scheduling/interval_scheduler.h>
#include <util/resource_usage.h>
#include <util/time.h>

#include <generated/flowmill/ingest/writer.h>

class ResourceUsageReporter {
public:
  using duration = std::common_type_t<monotonic_clock::duration, ResourceUsage::duration>;

  ResourceUsageReporter(uv_loop_t &loop, ::flowmill::ingest::Writer &writer);

  void start();
  void stop();

  static void report(::flowmill::ingest::Writer &writer);

private:
  void collect();

  monotonic_clock::time_point last_check_;
  ::flowmill::ingest::Writer &writer_;
  scheduling::IntervalScheduler scheduler_;
};
