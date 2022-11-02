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
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0, "")                                                                                                            \
  X(pipeline_metric_bytes_discarded, 0x00000001, INTERNAL_PREFIX "pipeline_metric_bytes_discarded")                            \
  X(codetiming_min_ns, 0x000000002, INTERNAL_PREFIX "codetiming_min_ns")                                                       \
  X(entrypoint_info, 0x000000004, INTERNAL_PREFIX "entrypoint_info")                                                           \
  X(otlp_grpc_requests_sent, 0x000000008, INTERNAL_PREFIX "otlp_grpc.requests_sent")                                           \
  X(connections, 0x000000010, INTERNAL_PREFIX "connections")                                                                   \
  X(rpc_queue_elem_utilization_fraction, 0x000000020, INTERNAL_PREFIX "rpc_queue_elem_utilization_fraction")                   \
  X(disconnects, 0x000000040, INTERNAL_PREFIX "rpc_queue_elem_utilization_fraction")                                           \
  X(codetiming_avg_ns, 0x000000080, INTERNAL_PREFIX "codetiming_avg_ns")                                                       \
  X(client_handle_pool, 0x000000100, INTERNAL_PREFIX "client_handle_pool")                                                     \
  X(otlp_grpc_successful_requests, 0x000000200, INTERNAL_PREFIX "otlp_grpc.successful_requests")                               \
  X(span_utilization, 0x000000400, INTERNAL_PREFIX "span_utilization")                                                         \
  X(up, 0x000000800, INTERNAL_PREFIX "up")                                                                                     \
  X(rpc_queue_buf_utilization_fraction, 0x000001000, INTERNAL_PREFIX "rpc_queue_buf_utilization_fraction")                     \
  X(collector_log_count, 0x000002000, INTERNAL_PREFIX "collector_log_count")                                                   \
  X(time_since_last_message_ns, 0x000004000, INTERNAL_PREFIX "time_since_last_message_ns")                                     \
  X(bpf_log, 0x000008000, INTERNAL_PREFIX "bpf_log")                                                                           \
  X(codetiming_count, 0x000010000, INTERNAL_PREFIX "codetiming_count")                                                         \
  X(message, 0x000020000, INTERNAL_PREFIX "message")                                                                           \
  X(otlp_grpc_bytes_sent, 0x000040000, INTERNAL_PREFIX "otlp_grpc.bytes_sent")                                                 \
  X(pipeline_message_error, 0x000080000, INTERNAL_PREFIX "pipeline_message_error")                                             \
  X(pipeline_metric_bytes_written, 0x000100000, INTERNAL_PREFIX "pipeline_metric_bytes_written")                               \
  X(codetiming_max_ns, 0x000200000, INTERNAL_PREFIX "codetiming_max_ns")                                                       \
  X(span_utilization_max, 0x000400000, INTERNAL_PREFIX "span_utilization_max")                                                 \
  X(client_handle_pool_fraction, 0x000800000, INTERNAL_PREFIX "client_handle_pool_fraction")                                   \
  X(span_utilization_fraction, 0x001000000, INTERNAL_PREFIX "span_utilization_fraction")                                       \
  X(rpc_latency_ns, 0x002000000, INTERNAL_PREFIX "rpc_latency_ns")                                                             \
  X(agg_root_truncation, 0x004000000, INTERNAL_PREFIX "agg_root_truncation")                                                   \
  X(clock_offset_ns, 0x008000000, INTERNAL_PREFIX "clock_offset_ns")                                                           \
  X(otlp_grpc_metrics_sent, 0x010000000, INTERNAL_PREFIX "otlp_grpc.metrics_sent")                                             \
  X(otlp_grpc_unknown_response_tags, 0x020000000, INTERNAL_PREFIX "otlp_grpc.unknown_response_tags")                           \
  X(collector_health, 0x040000000, INTERNAL_PREFIX "collector_health")                                                         \
  X(codetiming_sum_ns, 0x080000000, INTERNAL_PREFIX "codetiming_sum_ns")                                                       \
  X(otlp_grpc_failed_requests, 0x1000000000, INTERNAL_PREFIX "otlp_grpc.failed_requests")                                      \
  X(rpc_queue_buf_utilization, 0x2000000000, INTERNAL_PREFIX "rpc_queue_buf_utilization")                                      \
  X(pipeline_agent_connections, 0x4000000000, INTERNAL_PREFIX "pipeline_agent_connections")                                    \
  X(bytes_ingested_by_prometheus, 0x8000000000, INTERNAL_PREFIX "bytes_ingested_by_prometheus")                                \
  X(pipeline_failed_scrapes, 0x10000000000, INTERNAL_PREFIX "pipeline_failed_scrapes")                                         \
  X(big_items_dropped, 0x20000000000, INTERNAL_PREFIX "big_items_dropped")                                                     \
  X(rpc_write_stalls, 0x40000000000, INTERNAL_PREFIX "rpc_write_stalls")                                                       \
  X(all, 0xFFFFFFFFFFF, INTERNAL_PREFIX "all")
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
#undef INTERNAL_PREFIX
