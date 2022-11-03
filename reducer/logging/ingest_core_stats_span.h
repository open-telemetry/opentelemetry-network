// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <generated/ebpf_net/logging/span_base.h>

namespace reducer::logging {

class IngestCoreStatsSpan : public ::ebpf_net::logging::IngestCoreStatsSpanBase {
public:
  IngestCoreStatsSpan();
  ~IngestCoreStatsSpan();
  void client_handle_pool_stats(
      ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__client_handle_pool_stats *msg);
  void agent_connection_message_stats(
      ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref,
      u64 timestamp,
      jsrv_logging__agent_connection_message_stats *msg);
  void agent_connection_message_error_stats(
      ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref,
      u64 timestamp,
      jsrv_logging__agent_connection_message_error_stats *msg);
  void connection_stats(
      ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__connection_stats *msg);
  void collector_log_stats(
      ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__collector_log_stats *msg);
  void entry_point_stats(
      ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__entry_point_stats *msg);
  void server_stats(::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__server_stats *msg);
  void collector_health_stats(
      ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__collector_health_stats *msg);
  void
  bpf_log_stats(::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__bpf_log_stats *msg);
};

}; // namespace reducer::logging