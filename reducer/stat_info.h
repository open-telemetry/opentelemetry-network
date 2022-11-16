// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "metric_info.h"
#include "outbound_stats.h"

#include <string>
#include <string_view>

namespace reducer {

// Information on stats (internal metrics) with EBPF_NET prefixed outbound metrics.
//
struct EbpfNetMetricInfo : public OutboundMetricInfo<EbpfNetMetrics> {
  using OutboundMetricInfo<EbpfNetMetrics>::OutboundMetricInfo;

  static EbpfNetMetricInfo agg_root_truncation;
  static EbpfNetMetricInfo bpf_log;
  static EbpfNetMetricInfo client_handle_pool;
  static EbpfNetMetricInfo client_handle_pool_fraction;
  static EbpfNetMetricInfo clock_offset_ns;
  static EbpfNetMetricInfo codetiming_avg_ns;
  static EbpfNetMetricInfo codetiming_count;
  static EbpfNetMetricInfo codetiming_max_ns;
  static EbpfNetMetricInfo codetiming_min_ns;
  static EbpfNetMetricInfo codetiming_sum_ns;
  static EbpfNetMetricInfo collector_health;
  static EbpfNetMetricInfo collector_log_count;
  static EbpfNetMetricInfo connections;
  static EbpfNetMetricInfo disconnects;
  static EbpfNetMetricInfo entrypoint_info;
  static EbpfNetMetricInfo message;
  static EbpfNetMetricInfo otlp_grpc_bytes_failed;
  static EbpfNetMetricInfo otlp_grpc_bytes_sent;
  static EbpfNetMetricInfo otlp_grpc_metrics_failed;
  static EbpfNetMetricInfo otlp_grpc_metrics_sent;
  static EbpfNetMetricInfo otlp_grpc_requests_failed;
  static EbpfNetMetricInfo otlp_grpc_requests_sent;
  static EbpfNetMetricInfo otlp_grpc_unknown_response_tags;
  static EbpfNetMetricInfo pipeline_agent_connections;
  static EbpfNetMetricInfo pipeline_message_error;
  static EbpfNetMetricInfo prometheus_big_items_dropped;
  static EbpfNetMetricInfo prometheus_bytes_discarded;
  static EbpfNetMetricInfo prometheus_bytes_ingested;
  static EbpfNetMetricInfo prometheus_bytes_written;
  static EbpfNetMetricInfo prometheus_failed_scrapes;
  static EbpfNetMetricInfo rpc_latency_ns;
  static EbpfNetMetricInfo rpc_queue_buf_utilization;
  static EbpfNetMetricInfo rpc_queue_buf_utilization_fraction;
  static EbpfNetMetricInfo rpc_queue_elem_utilization_fraction;
  static EbpfNetMetricInfo rpc_write_stalls;
  static EbpfNetMetricInfo span_utilization;
  static EbpfNetMetricInfo span_utilization_fraction;
  static EbpfNetMetricInfo span_utilization_max;
  static EbpfNetMetricInfo time_since_last_message_ns;
  static EbpfNetMetricInfo up;
};

} // namespace reducer
