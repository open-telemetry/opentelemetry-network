/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "reducer_config.h"

#include <yaml-cpp/yaml.h>

#include <stdexcept>

namespace reducer {

const ReducerConfig DEFAULT_REDUCER_CONFIG = {
    .telemetry_port = 8080,

    .num_ingest_shards = 1,
    .num_matching_shards = 1,
    .num_aggregation_shards = 1,
    .partitions_per_shard = 1,

    .enable_id_id = false,
    .enable_az_id = false,
    .enable_flow_logs = false,

    .enable_otlp_grpc_metrics = false,
    .otlp_grpc_metrics_address = "localhost",
    .otlp_grpc_metrics_port = 4317,
    .otlp_grpc_batch_size = 1000,
    .enable_otlp_grpc_metric_descriptions = false,

    .disable_prometheus_metrics = false,
    .shard_prometheus_metrics = false,
    .prom_bind = "127.0.0.1:7010",
    .scrape_size_limit_bytes = std::nullopt,
    .internal_prom_bind = "0.0.0.0:7001",
    .stats_scrape_size_limit_bytes = std::nullopt,
    .scrape_metrics_tsdb_format = reducer::TsdbFormat::prometheus,

    .disable_node_ip_field = false,
    .enable_autonomous_system_ip = false,

    .geoip_path = std::nullopt,

    .enable_aws_enrichment = false,
    .enable_percentile_latencies = false,

    .disable_metrics = "",
    .enable_metrics = "",

    .index_dump_interval = 0,
};

namespace {
// Used in LOAD_FIELD macro to support both normal and optional-valued fields.
template <typename T> struct value_type {
  using type = T;
};
template <typename T> struct value_type<std::optional<T>> {
  using type = T;
};
} // namespace

void read_config_from_yaml(ReducerConfig &config, std::string const &path)
{
  if (path.empty()) {
    return;
  }

  YAML::Node yaml = YAML::LoadFile(path);

#define LOAD_FIELD(field)                                                                                                      \
  if (auto value = yaml[#field]) {                                                                                             \
    using field_type = typename value_type<typeof config.field>::type;                                                         \
    config.field = value.as<field_type>();                                                                                     \
  }

  LOAD_FIELD(telemetry_port);

  LOAD_FIELD(num_ingest_shards);
  LOAD_FIELD(num_matching_shards);
  LOAD_FIELD(num_aggregation_shards);
  LOAD_FIELD(partitions_per_shard);

  LOAD_FIELD(enable_id_id);
  LOAD_FIELD(enable_az_id);
  LOAD_FIELD(enable_flow_logs);

  LOAD_FIELD(enable_otlp_grpc_metrics);
  LOAD_FIELD(otlp_grpc_metrics_address);
  LOAD_FIELD(otlp_grpc_metrics_port);
  LOAD_FIELD(otlp_grpc_batch_size);
  LOAD_FIELD(enable_otlp_grpc_metric_descriptions);

  LOAD_FIELD(disable_prometheus_metrics);
  LOAD_FIELD(shard_prometheus_metrics);
  LOAD_FIELD(prom_bind);
  LOAD_FIELD(scrape_size_limit_bytes);
  LOAD_FIELD(internal_prom_bind);
  LOAD_FIELD(stats_scrape_size_limit_bytes);

  if (auto value = yaml["scrape_metrics_tsdb_format"]) {
    auto str_value = value.as<std::string>();
    if (!enum_from_string(str_value, config.scrape_metrics_tsdb_format)) {
      throw std::runtime_error("unknown TSDB format '" + str_value + "'");
    }
  }

  LOAD_FIELD(disable_node_ip_field);
  LOAD_FIELD(enable_autonomous_system_ip);

  LOAD_FIELD(geoip_path);

  LOAD_FIELD(enable_aws_enrichment);
  LOAD_FIELD(enable_percentile_latencies);

  LOAD_FIELD(disable_metrics);
  LOAD_FIELD(enable_metrics);

  LOAD_FIELD(index_dump_interval);

#undef LOAD_FIELD
}

}; // namespace reducer
