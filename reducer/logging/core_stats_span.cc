/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core_stats_span.h"

#include "logging_core.h"

#include <reducer/internal_metrics_encoder.h>
#include <reducer/logging/component.h>

#include <reducer/internal_stats.h>
#include <reducer/tsdb_format.h>

#include <util/log.h>
#include <util/log_formatters.h>

namespace reducer::logging {

CoreStatsSpan::CoreStatsSpan() {}

CoreStatsSpan::~CoreStatsSpan() {}

void CoreStatsSpan::span_utilization_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__span_utilization_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  SpanUtilizationStats stats;
  stats.labels.span = msg->span_name;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.metrics.utilization = std::size_t(msg->allocated);
  stats.metrics.utilization_fraction = (double)msg->allocated / msg->pool_size_;
  stats.metrics.utilization_max = std::size_t(msg->max_allocated);

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan::span_utilization_stats: module={} span_name={} allocated={} max_allocated={} pool_size_={} timestamp={} ",
      msg->module,
      msg->span_name,
      msg->allocated,
      msg->max_allocated,
      msg->pool_size_,
      msg->time_ns);
}

void CoreStatsSpan::connection_message_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__connection_message_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  ConnectionMessageStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.connection = std::to_string(msg->conn);
  stats.labels.message = msg->msg_;
  stats.labels.severity = std::to_string(msg->severity_);
  stats.metrics.count = msg->count;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan::connection_message_stats module={} msg_={} shard={} severity_={} conn={} count={} timestamp={}",
      msg->module,
      msg->msg_,
      msg->shard,
      msg->severity_,
      msg->conn,
      msg->count,
      msg->time_ns);
}

void CoreStatsSpan ::connection_message_error_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__connection_message_error_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  ConnectionMessageErrorStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.connection = std::to_string(msg->conn);
  stats.labels.message = msg->msg_;
  stats.labels.error = msg->error;
  stats.metrics.count = msg->count;
  stats.labels.module = msg->module;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan ::connection_message_error_stats: module={} msg_={} shard={} conn={} count={} error={} timestamp={}",
      msg->module,
      msg->msg_,
      msg->shard,
      msg->conn,
      msg->count,
      msg->error,
      msg->time_ns);
}

void CoreStatsSpan::status_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__status_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  StatusStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.program = msg->program;
  stats.labels.version = msg->version;
  stats.metrics.status = std::size_t(msg->status);

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan::status_stats module={} shard={} program={} version={} status={} timestamp={}",
      msg->module,
      msg->shard,
      msg->program,
      msg->version,
      msg->status,
      msg->time_ns);
}

void CoreStatsSpan ::rpc_receive_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__rpc_receive_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  RpcLatencyStats stats;
  stats.labels.module = msg->receiver_app;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.peer = msg->sender_app;
  stats.metrics.max_latency_ns = msg->max_latency_ns;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan::rpc_receive_stats receiver_app={} shard={} sender_app={} max_latency_ns={} timestamp={}",
      msg->receiver_app,
      msg->shard,
      msg->sender_app,
      msg->max_latency_ns,
      msg->time_ns);
}

void CoreStatsSpan ::rpc_write_stalls_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__rpc_write_stalls_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  RpcWriteStallsStats stats;
  stats.labels.module = msg->sender_app;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.peer = msg->receiver_app;
  stats.metrics.stalls = msg->count;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan::rpc_write_stalls_stats sender_app={} shard={} receiver_app={}  count={} timestamp={}",
      msg->sender_app,
      msg->shard,
      msg->receiver_app,
      msg->count,
      msg->time_ns);
}

void CoreStatsSpan ::rpc_write_utilization_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__rpc_write_utilization_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  RpcQueueUtilizationStats stats;
  stats.labels.module = msg->sender_app;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.peer = msg->receiver_app;
  stats.metrics.max_buf_used = msg->max_buf_used;
  stats.metrics.max_buf_util = msg->max_buf_util;
  stats.metrics.max_elem_util = msg->max_elem_util;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan::rpc_write_utilization_stats sender_app={} shard={} receiver_app={}  max_buf_used={} max_buf_util={} max_elem_util={} timestamp={}",
      msg->sender_app,
      msg->shard,
      msg->receiver_app,
      msg->max_buf_used,
      msg->max_buf_util,
      msg->max_elem_util,
      msg->time_ns);
}

void CoreStatsSpan::code_timing_stats(
    ::ebpf_net::logging::weak_refs::core_stats span_ref, u64 timestamp, jsrv_logging__code_timing_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  CodeTimingStats stats;
  stats.labels.name = msg->name;
  stats.labels.filename = msg->filename;
  stats.labels.line = std::to_string(msg->line);
  stats.labels.index = std::to_string(msg->index_string);

  stats.metrics.count = msg->count;
  stats.metrics.avg_ns = msg->avg_ns;
  stats.metrics.min_ns = msg->min_ns;
  stats.metrics.max_ns = msg->max_ns;
  stats.metrics.sum_ns = msg->sum_ns;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "CoreStatsSpan::code_timing_stats name={} filename={} line={}  index={} count={} avg_ns={} min_ns={} max_ns={} sum_ns={}",
      msg->name,
      msg->filename,
      msg->line,
      msg->index_string,
      msg->count,
      msg->avg_ns,
      msg->min_ns,
      msg->max_ns,
      msg->sum_ns,
      msg->time_ns);
}
} // namespace reducer::logging
