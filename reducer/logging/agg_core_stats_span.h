// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <generated/ebpf_net/logging/span_base.h>

namespace reducer::logging {

class AggCoreStatsSpan : public ::ebpf_net::logging::AggCoreStatsSpanBase {
public:
  AggCoreStatsSpan();
  ~AggCoreStatsSpan();
  void agg_root_truncation_stats(
      ::ebpf_net::logging::weak_refs::agg_core_stats span_ref, u64 timestamp, jsrv_logging__agg_root_truncation_stats *msg);
  void agg_prometheus_bytes_stats(
      ::ebpf_net::logging::weak_refs::agg_core_stats span_ref, u64 timestamp, jsrv_logging__agg_prometheus_bytes_stats *msg);
  void agg_otlp_grpc_stats(
      ::ebpf_net::logging::weak_refs::agg_core_stats span_ref, u64 timestamp, jsrv_logging__agg_otlp_grpc_stats *msg);
};

}; // namespace reducer::logging
