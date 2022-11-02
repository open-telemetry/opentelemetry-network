// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <generated/ebpf_net/logging/span_base.h>

namespace reducer::logging {

class CoreStatsSpan : public ::ebpf_net::logging::CoreStatsSpanBase {
public:
  CoreStatsSpan();
  ~CoreStatsSpan();
  void span_utilization_stats(
      ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__span_utilization_stats *msg);
  void connection_message_stats(
      ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__connection_message_stats *msg);
  void connection_message_error_stats(
      ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__connection_message_error_stats *msg);
  void status_stats(::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__status_stats *msg);
  void
  rpc_receive_stats(::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__rpc_receive_stats *msg);
  void rpc_write_stalls_stats(
      ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__rpc_write_stalls_stats *msg);
  void rpc_write_utilization_stats(
      ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__rpc_write_utilization_stats *msg);
  void
  code_timing_stats(::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__code_timing_stats *msg);
};

}; // namespace reducer::logging
