/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/core_base.h>

#include <reducer/disabled_metrics.h>
#include <reducer/publisher.h>
#include <reducer/rpc_stats.h>

#include <generated/ebpf_net/logging/connection.h>
#include <generated/ebpf_net/logging/index.h>
#include <generated/ebpf_net/logging/protocol.h>
#include <generated/ebpf_net/logging/transform_builder.h>

namespace reducer {
class RpcQueueMatrix;
}

namespace reducer::logging {

// This class implements the 'logging' app (see render definition file).
//
// It receives messages that are signaling error conditions, writes out their
// textual representation to stderr, and publishes their count to a TSDB.
//
class LoggingCore : public CoreBase<
                        ebpf_net::logging::Index,
                        ebpf_net::logging::Protocol,
                        ebpf_net::logging::Connection,
                        ebpf_net::logging::TransformBuilder> {
public:
  LoggingCore(
      RpcQueueMatrix &ingest_to_logging_queues,
      RpcQueueMatrix &matching_to_logging_queues,
      RpcQueueMatrix &aggregation_to_logging_queues,
      Publisher::WriterPtr stats_writer,
      const DisabledMetrics &disabled_metrics,
      size_t shard_num,
      u64 initial_timestamp);

  // For writing internal stats.
  Publisher::WriterPtr stats_writer_;

  InternalMetricsEncoder encoder_;

  // Enables otlp formatted internal metrics.
  static void set_otlp_formatted_internal_metrics_enabled(bool enabled);

  // Get tsdb format for Logging core.
  static TsdbFormat get_tsdb_format();

private:
  // Keeper of ingest->this RPC stats.
  RpcReceiverStats ingest_to_logging_stats_;
  // Keeper of matching->this RPC stats.
  RpcReceiverStats matching_to_logging_stats_;
  // Keeper of aggregation->this RPC stats.
  RpcReceiverStats aggregation_to_logging_stats_;

  ::ebpf_net::logging::auto_handles::logger logger_;

  // Internal metrics using otlp formatting.
  static bool otlp_formatted_internal_metrics_enabled_;

  // Outputs internal stats to be scraped by a time-series DB.
  void write_internal_stats() override;
};

} // namespace reducer::logging
