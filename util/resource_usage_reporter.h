/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scheduling/interval_scheduler.h>
#include <util/resource_usage.h>
#include <util/time.h>

#include <generated/ebpf_net/ingest/writer.h>

class ResourceUsageReporter {
public:
  using duration = std::common_type_t<monotonic_clock::duration, ResourceUsage::duration>;

  ResourceUsageReporter(uv_loop_t &loop, ::ebpf_net::ingest::Writer &writer);

  void start();
  void stop();

  static void report(::ebpf_net::ingest::Writer &writer);

private:
  void collect();

  monotonic_clock::time_point last_check_;
  ::ebpf_net::ingest::Writer &writer_;
  scheduling::IntervalScheduler scheduler_;
};
