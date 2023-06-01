// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include <reducer/logging/connection_metrics.h>
#include <reducer/logging/logging_core.h>

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <reducer/rpc_queue_matrix.h>

#include <common/constants.h>

#include <platform/userspace-time.h>
#include <util/log.h>
#include <util/log_formatters.h>

#include <absl/container/flat_hash_map.h>

namespace reducer::logging {

bool LoggingCore::otlp_formatted_internal_metrics_enabled_ = false;

void LoggingCore::set_otlp_formatted_internal_metrics_enabled(bool enabled)
{
  otlp_formatted_internal_metrics_enabled_ = enabled;
}

TsdbFormat LoggingCore::get_tsdb_format()
{
  TsdbFormat internal_format =
      otlp_formatted_internal_metrics_enabled_ == true ? TsdbFormat::otlp_grpc : TsdbFormat::prometheus;
  return internal_format;
}

LoggingCore::LoggingCore(
    RpcQueueMatrix &ingest_to_logging_queues,
    RpcQueueMatrix &matching_to_logging_queues,
    RpcQueueMatrix &aggregation_to_logging_queues,
    Publisher::WriterPtr stats_writer,
    const DisabledMetrics &disabled_metrics,
    size_t shard_num,
    u64 initial_timestamp)
    : CoreBase("logging", shard_num, initial_timestamp),
      stats_writer_(std::move(stats_writer)),
      encoder_(get_tsdb_format(), stats_writer_, disabled_metrics),
      ingest_to_logging_stats_(shard_num, "ingest", "logging"),
      matching_to_logging_stats_(shard_num, "matching", "logging"),
      aggregation_to_logging_stats_(shard_num, "aggregation", "logging"),
      logger_(index_.logger.alloc())
{
  // ingest->this
  add_rpc_clients(ingest_to_logging_queues.make_readers(shard_num), ClientType::ingest, ingest_to_logging_stats_);

  // matching->this
  add_rpc_clients(matching_to_logging_queues.make_readers(shard_num), ClientType::matching, matching_to_logging_stats_);

  // aggregation->this
  add_rpc_clients(
      aggregation_to_logging_queues.make_readers(shard_num), ClientType::aggregation, aggregation_to_logging_stats_);
}

void LoggingCore::write_internal_stats()
{
  auto bytes_failed_to_write = stats_writer_->bytes_failed_to_write();
  u64 time_ns = fp_get_time_ns();

  write_common_stats(encoder_, time_ns);

  int const shard = shard_num();
  std::string_view module = "logging";

  ingest_to_logging_stats_.write_internal_stats(encoder_, time_ns);
  matching_to_logging_stats_.write_internal_stats(encoder_, time_ns);
  aggregation_to_logging_stats_.write_internal_stats(encoder_, time_ns);

  stats_writer_->write_internal_stats(encoder_, time_ns, shard, module);

  encoder_.flush();
  stats_writer_->flush();

  if (stats_writer_->bytes_failed_to_write() > bytes_failed_to_write) {
    LOG::error("Logging core failed to publish internal metrics writer stats");
  }
}

} // namespace reducer::logging
