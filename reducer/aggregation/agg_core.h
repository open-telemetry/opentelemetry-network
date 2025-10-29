/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/core_base.h>

#include <reducer/aggregation/percentile_latencies.h>
#include <reducer/aggregation/stat_counters.h>

#include <reducer/disabled_metrics.h>
#include <reducer/publisher.h>
#include <reducer/rpc_stats.h>
#include <reducer/tsdb_format.h>

#include <generated/ebpf_net/aggregation/connection.h>
#include <generated/ebpf_net/aggregation/index.h>
#include <generated/ebpf_net/aggregation/protocol.h>
#include <generated/ebpf_net/aggregation/span_base.h>
#include <generated/ebpf_net/aggregation/transform_builder.h>

#include <generated/ebpf_net/logging/writer.h>

#include <memory>
#include <vector>

// cxx::bridge header for Rust AggregationCore
#include <reducer_aggregation_cxxbridge.h>

namespace reducer {
class RpcQueueMatrix;
}

namespace reducer::aggregation {

// This class implements the 'aggregation' app (see render definition file).
//
// It receives messages from one or more matching core(s), performs flow
// aggregation and outputs metrics ready to be scraped by a time-series DB.
//
class AggCore : public CoreBase<
                    ebpf_net::aggregation::Index,
                    ebpf_net::aggregation::Protocol,
                    ebpf_net::aggregation::Connection,
                    ebpf_net::aggregation::TransformBuilder> {
public:
  // Disables using IP address in node spans.
  static void set_node_ip_field_disabled(bool disabled);
  // Returns whether using IP addresses in node spans is disabled.
  static bool node_ip_field_disabled() { return node_ip_field_disabled_; }

  // Enables generating node-node (id-id) metrics.
  static void set_id_id_enabled(bool enabled);

  // Enables generating az-node metrics.
  static void set_az_id_enabled(bool enabled);

  // Enables generating flow logs from node-node (id-id) metrics.
  static void set_flow_logs_enabled(bool enabled);

  // How many top aggregation role/role pairs to show the count for in the
  // pipeline_aggregation_roles internal stat.
  static void count_top_aggregation_roles(size_t k);

  // Internal statistics counters.
  StatCounters stat_counters;

  AggCore(
      RpcQueueMatrix &matching_to_aggregation_queues,
      RpcQueueMatrix &aggregation_to_logging_queues,
      std::unique_ptr<Publisher> &metrics_publisher,
      std::vector<Publisher::WriterPtr> metric_writers,
      std::unique_ptr<Publisher> &otlp_metrics_publisher,
      Publisher::WriterPtr otlp_metric_writer,
      bool enable_percentile_latencies,
      TsdbFormat metrics_tsdb_format,
      reducer::DisabledMetrics disabled_metrics,
      size_t shard_num,
      u64 initial_timestamp);

private:
  // Stores TDigests to compute p90, p95, p99 latencies
  std::unique_ptr<PercentileLatencies> p_latencies_;

  // Publisher for Prometheus (scrape) style external metrics.
  std::unique_ptr<Publisher> &metrics_publisher_;
  // For writing external metrics to a TSDB.
  std::vector<Publisher::WriterPtr> metric_writers_;

  // Publisher for OTLP (push) style external metrics.
  std::unique_ptr<Publisher> &otlp_metrics_publisher_;
  // For writing external metrics to opentelemetry collector via OTLP gRPC.
  Publisher::WriterPtr otlp_metric_writer_;

  // Format of TSDB metrics.
  TsdbFormat metrics_tsdb_format_;

  // Keeper of matching->this RPC stats.
  RpcReceiverStats matching_to_aggregation_stats_;
  // Sender to this->logging RPC stats.
  RpcSenderStats aggregation_to_logging_stats_;

  // allow the user to control which metrics are disabled
  DisabledMetrics disabled_metrics_;

  // accessor handle for core_worker_internal_metrics span
  ::ebpf_net::aggregation::auto_handles::core_stats core_stats_;

  ::ebpf_net::aggregation::auto_handles::agg_core_stats agg_core_stats_;

  // Flag indicating whether using IP addresses in node spans is disabled.
  static bool node_ip_field_disabled_;

  // Flag indicating whether id-id timeseries should be outputted.
  static bool id_id_enabled_;

  // Flag indicating whether az-id timeseries should be outputted.
  static bool az_id_enabled_;

  // Flag indicating whether flow logs should be outputted.
  static bool flow_logs_enabled_;

  void on_timeslot_complete() override;

  // Outputs external metrics.
  void write_metrics();

  // Outputs standard-resolution metrics.
  void write_standard_metrics(u64 t);

  // Outputs internal stats.
  void write_internal_stats() override;

public:
  // Run the aggregation core using the Rust implementation.
  void run();
  // Stop the Rust core.
  void stop_async();

private:
  // Opaque Rust AggregationCore owned via cxx rust::Box
  rust::Box<reducer_agg::AggregationCore> rust_core_;
};

} // namespace reducer::aggregation
