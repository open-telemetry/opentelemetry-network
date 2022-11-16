// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>

namespace reducer {
struct SpanStats {
  std::string_view span_name;
  std::size_t utilization;
  double utilization_fraction;
  std::size_t utilization_max;
};

template <typename I, typename P, typename C, typename T>
void CoreBase<I, P, C, T>::write_common_stats(InternalMetricsEncoder &encoder, u64 time_ns)
{
  const auto module = app_name();
  const auto shard = shard_num();

  index_.size_statistics(
      [&](std::string_view span_name, std::size_t allocated, std::size_t max_allocated, std::size_t pool_size) {
        SpanUtilizationStats stats;
        stats.labels.span = span_name;
        stats.labels.module = module;
        stats.labels.shard = std::to_string(shard);
        stats.metrics.utilization = allocated;
        stats.metrics.utilization_fraction = (double)allocated / pool_size;
        stats.metrics.utilization_max = max_allocated;
        encoder.write_internal_stats(stats, time_ns);
      });

  for (size_t conn = 0; conn < rpc_clients_.size(); ++conn) {
    auto &rpc_handler = static_cast<RpcHandler &>(*rpc_clients_[conn].handler);
    auto &connection = rpc_handler.connection;

    connection.message_stats.foreach ([&](std::string_view module, std::string_view msg, int severity, u64 count) {
      ConnectionMessageStats stats;
      stats.labels.module = module;
      stats.labels.shard = std::to_string(shard);
      stats.labels.connection = std::to_string(conn);
      stats.labels.message = msg;
      stats.labels.severity = std::to_string(severity);
      stats.metrics.count = count;
      encoder.write_internal_stats(stats, time_ns);
    });

    connection.message_errors.foreach ([&](std::string_view module, std::string_view msg, std::string_view error, u64 count) {
      ConnectionMessageErrorStats stats;
      stats.labels.module = module;
      stats.labels.shard = std::to_string(shard);
      stats.labels.connection = std::to_string(conn);
      stats.labels.message = msg;
      stats.labels.error = error;
      stats.metrics.count = count;

      encoder.write_internal_stats(stats, time_ns);
    });
  }

  StatusStats stats;
  stats.labels.module = module;
  stats.labels.shard = std::to_string(shard);
  stats.labels.program = kServiceName;
  std::stringstream ss;
  ss << versions::release;
  stats.labels.version = ss.str();
  stats.metrics.status = 1u;
  encoder.write_internal_stats(stats, time_ns);
}

template <typename I, typename P, typename C, typename T>
template <typename CoreStatsHandle>
void CoreBase<I, P, C, T>::write_common_stats_to_logging_core(CoreStatsHandle &internal_metrics, u64 time_ns)
{
  const auto module = app_name();
  const auto shard = shard_num();

  index_.size_statistics(
      [&](std::string_view span_name, std::size_t allocated, std::size_t max_allocated, std::size_t pool_size) {
        internal_metrics.span_utilization_stats(
            jb_blob(span_name), jb_blob(module), shard, allocated, max_allocated, pool_size, time_ns);
      });

  for (size_t conn = 0; conn < rpc_clients_.size(); ++conn) {
    auto &rpc_handler = static_cast<RpcHandler &>(*rpc_clients_[conn].handler);
    auto &connection = rpc_handler.connection;

    connection.message_stats.foreach ([&](std::string_view module, std::string_view msg, int severity, u64 count) {
      internal_metrics.connection_message_stats(jb_blob(module), jb_blob(msg), shard, severity, conn, time_ns, count);
    });

    connection.message_errors.foreach ([&](std::string_view module, std::string_view msg, std::string_view error, u64 count) {
      internal_metrics.connection_message_error_stats(
          jb_blob(module), shard, conn, jb_blob(msg), jb_blob(error), count, time_ns);
    });
  }

  std::stringstream ss;
  ss << versions::release;
  internal_metrics.status_stats(jb_blob(module), shard, jb_blob(std::string(kServiceName)), jb_blob(ss.str()), 1u, time_ns);
}

template <typename I, typename P, typename C, typename T>
void CoreBase<I, P, C, T>::dump_internal_state(std::chrono::milliseconds timestamp)
{
  index_dumper_.dump(app_name(), shard_num(), index_, std::chrono::duration_cast<std::chrono::seconds>(timestamp));
}

} // namespace reducer
