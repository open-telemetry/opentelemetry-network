/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utility>

namespace reducer {

template <typename Out> Out &&operator<<(Out &&out, ReducerConfig const &config)
{
  out << "telemetry_port: " << config.telemetry_port << "\n"
      << "num_ingest_shards: " << config.num_ingest_shards << "\n"
      << "num_matching_shards: " << config.num_matching_shards << "\n"
      << "num_aggregation_shards: " << config.num_aggregation_shards << "\n"
      << "partitions_per_shard: " << config.partitions_per_shard << "\n"
      << "enable_id_id: " << config.enable_id_id << "\n"
      << "enable_az_id: " << config.enable_az_id << "\n"
      << "enable_flow_logs: " << config.enable_flow_logs << "\n"
      << "enable_otlp_grpc_metrics: " << config.enable_otlp_grpc_metrics << "\n"
      << "otlp_grpc_metrics_address: " << config.otlp_grpc_metrics_address << "\n"
      << "otlp_grpc_metrics_port: " << config.otlp_grpc_metrics_port << "\n"
      << "otlp_grpc_batch_size: " << config.otlp_grpc_batch_size << "\n"
      << "enable_otlp_grpc_metric_descriptions: " << config.enable_otlp_grpc_metric_descriptions << "\n"
      << "disable_prometheus_metrics: " << config.disable_prometheus_metrics << "\n"
      << "shard_prometheus_metrics: " << config.shard_prometheus_metrics << "\n"
      << "prom_bind: " << config.prom_bind << "\n"
      << "internal_prom_bind: " << config.internal_prom_bind << "\n";

  if (config.scrape_size_limit_bytes) {
    out << "scrape_size_limit_bytes: " << *config.scrape_size_limit_bytes << "\n";
  } else {
    out << "scrape_size_limit_bytes: unlimited\n";
  }

  if (config.stats_scrape_size_limit_bytes) {
    out << "stats_scrape_size_limit_bytes: " << *config.stats_scrape_size_limit_bytes << "\n";
  } else {
    out << "stats_scrape_size_limit_bytes: unlimited\n";
  }

  out << "scrape_metrics_tsdb_format: " << to_string(config.scrape_metrics_tsdb_format) << "\n"
      << "disable_node_ip_field: " << config.disable_node_ip_field << "\n"
      << "enable_autonomous_system_ip: " << config.enable_autonomous_system_ip << "\n"
      << "geoip_path: " << (config.geoip_path ? *config.geoip_path : "none") << "\n"
      << "enable_aws_enrichment: " << config.enable_aws_enrichment << "\n"
      << "enable_percentile_latencies: " << config.enable_percentile_latencies << "\n"
      << "disable_metrics: " << config.disable_metrics << "\n"
      << "enable_metrics: " << config.enable_metrics << "\n"
      << "index_dump_interval: " << config.index_dump_interval << "\n";

  return std::forward<Out>(out);
}

} // namespace reducer
