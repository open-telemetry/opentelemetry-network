/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>
#include <reducer/tsdb_format.h>

#include <optional>
#include <string>

namespace reducer {

// Configuration options for running Reducer.
//
struct ReducerConfig {
  u32 telemetry_port = 0;

  u32 num_ingest_shards = 0;
  u32 num_matching_shards = 0;
  u32 num_aggregation_shards = 0;
  u32 partitions_per_shard = 0;

  bool enable_id_id = false;
  bool enable_az_id = false;
  bool enable_flow_logs = false;

  bool enable_otlp_grpc_metrics = false;
  std::string otlp_grpc_metrics_address;
  u32 otlp_grpc_metrics_port = 0;
  int otlp_grpc_batch_size = 0;
  bool enable_otlp_grpc_metric_descriptions = false;

  bool disable_prometheus_metrics = false;
  bool shard_prometheus_metrics = false;
  std::string prom_bind;
  std::optional<u64> scrape_size_limit_bytes;
  std::string internal_prom_bind;
  std::optional<u64> stats_scrape_size_limit_bytes;
  reducer::TsdbFormat scrape_metrics_tsdb_format;

  bool disable_node_ip_field = false;
  bool enable_autonomous_system_ip = false;

  std::optional<std::string> geoip_path;

  bool enable_aws_enrichment = false;
  bool enable_percentile_latencies = false;

  std::string disable_metrics;
  std::string enable_metrics;

  u64 index_dump_interval = 0;
};

// No defaults defined here; defaults live in Rust layer.

} // namespace reducer

#include "reducer_config.inl"
