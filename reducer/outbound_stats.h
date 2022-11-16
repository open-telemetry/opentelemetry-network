// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <platform/types.h>

#include <util/enum.h>

// This file describes our outgoing internal stats that are either hosted for
// prometheus to scrape, or sent over OTLP.
//

// A value of unknown is used mainly for string to enum conversions
// and denotes the conversion failed.
//

#define INTERNAL_PREFIX "ebpf_net."
#define ENUM_NAMESPACE reducer
#define ENUM_NAME EbpfNetMetrics
#define ENUM_TYPE std::uint64_t
// clang-format off
#define ENUM_ELEMENTS(X)                                                \
  X(unknown, 0, "")                                                     \
  X(agg_root_truncation,                 0x0000'0000'0000'0001, INTERNAL_PREFIX "agg_root_truncation") \
  X(bpf_log,                             0x0000'0000'0000'0002, INTERNAL_PREFIX "bpf_log") \
  X(client_handle_pool,                  0x0000'0000'0000'0004, INTERNAL_PREFIX "client_handle_pool") \
  X(client_handle_pool_fraction,         0x0000'0000'0000'0008, INTERNAL_PREFIX "client_handle_pool_fraction") \
  X(clock_offset_ns,                     0x0000'0000'0000'0010, INTERNAL_PREFIX "clock_offset_ns") \
  X(codetiming_avg_ns,                   0x0000'0000'0000'0020, INTERNAL_PREFIX "codetiming_avg_ns") \
  X(codetiming_count,                    0x0000'0000'0000'0040, INTERNAL_PREFIX "codetiming_count") \
  X(codetiming_max_ns,                   0x0000'0000'0000'0080, INTERNAL_PREFIX "codetiming_max_ns") \
  X(codetiming_min_ns,                   0x0000'0000'0000'0100, INTERNAL_PREFIX "codetiming_min_ns") \
  X(codetiming_sum_ns,                   0x0000'0000'0000'0200, INTERNAL_PREFIX "codetiming_sum_ns") \
  X(collector_health,                    0x0000'0000'0000'0400, INTERNAL_PREFIX "collector_health") \
  X(collector_log_count,                 0x0000'0000'0000'0800, INTERNAL_PREFIX "collector_log_count") \
  X(connections,                         0x0000'0000'0000'1000, INTERNAL_PREFIX "connections") \
  X(disconnects,                         0x0000'0000'0000'2000, INTERNAL_PREFIX "disconnects") \
  X(entrypoint_info,                     0x0000'0000'0000'4000, INTERNAL_PREFIX "entrypoint_info") \
  X(message,                             0x0000'0000'0000'8000, INTERNAL_PREFIX "message") \
  X(otlp_grpc_bytes_failed,              0x0000'0000'0001'0000, INTERNAL_PREFIX "otlp_grpc.bytes_failed") \
  X(otlp_grpc_bytes_sent,                0x0000'0000'0002'0000, INTERNAL_PREFIX "otlp_grpc.bytes_sent") \
  X(otlp_grpc_metrics_failed,            0x0000'0000'0004'0000, INTERNAL_PREFIX "otlp_grpc.metrics_failed") \
  X(otlp_grpc_metrics_sent,              0x0000'0000'0008'0000, INTERNAL_PREFIX "otlp_grpc.metrics_sent") \
  X(otlp_grpc_requests_failed,           0x0000'0000'0010'0000, INTERNAL_PREFIX "otlp_grpc.requests_failed") \
  X(otlp_grpc_requests_sent,             0x0000'0000'0020'0000, INTERNAL_PREFIX "otlp_grpc.requests_sent") \
  X(otlp_grpc_unknown_response_tags,     0x0000'0000'0040'0000, INTERNAL_PREFIX "otlp_grpc.unknown_response_tags") \
  X(pipeline_agent_connections,          0x0000'0000'0080'0000, INTERNAL_PREFIX "pipeline_agent_connections") \
  X(pipeline_message_error,              0x0000'0000'0100'0000, INTERNAL_PREFIX "pipeline_message_error") \
  X(prometheus_big_items_dropped,        0x0000'0000'0200'0000, INTERNAL_PREFIX "prometheus.big_items_dropped") \
  X(prometheus_bytes_discarded,          0x0000'0000'0400'0000, INTERNAL_PREFIX "prometheus.bytes_discarded") \
  X(prometheus_bytes_ingested,           0x0000'0000'0800'0000, INTERNAL_PREFIX "prometheus.bytes_ingested") \
  X(prometheus_bytes_written,            0x0000'0000'1000'0000, INTERNAL_PREFIX "prometheus.bytes_written") \
  X(prometheus_failed_scrapes,           0x0000'0000'2000'0000, INTERNAL_PREFIX "prometheus.failed_scrapes") \
  X(rpc_latency_ns,                      0x0000'0000'4000'0000, INTERNAL_PREFIX "rpc_latency_ns") \
  X(rpc_queue_buf_utilization,           0x0000'0000'8000'0000, INTERNAL_PREFIX "rpc_queue_buf_utilization") \
  X(rpc_queue_buf_utilization_fraction,  0x0000'0001'0000'0000, INTERNAL_PREFIX "rpc_queue_buf_utilization_fraction") \
  X(rpc_queue_elem_utilization_fraction, 0x0000'0002'0000'0000, INTERNAL_PREFIX "rpc_queue_elem_utilization_fraction") \
  X(rpc_write_stalls,                    0x0000'0004'0000'0000, INTERNAL_PREFIX "rpc_write_stalls") \
  X(span_utilization,                    0x0000'0008'0000'0000, INTERNAL_PREFIX "span_utilization") \
  X(span_utilization_fraction,           0x0000'0010'0000'0000, INTERNAL_PREFIX "span_utilization_fraction") \
  X(span_utilization_max,                0x0000'0020'0000'0000, INTERNAL_PREFIX "span_utilization_max") \
  X(time_since_last_message_ns,          0x0000'0040'0000'0000, INTERNAL_PREFIX "time_since_last_message_ns") \
  X(up,                                  0x0000'0080'0000'0000, INTERNAL_PREFIX "up") \
  X(all,                                 0xFFFF'FFFF'FFFF'FFFF, INTERNAL_PREFIX "all")
// clang-format on
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
#undef INTERNAL_PREFIX
