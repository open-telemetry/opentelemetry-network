// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include <channel/component.h>
#include <common/client_type.h>
#include <reducer/constants.h>
#include <reducer/ingest/component.h>
#include <reducer/matching/component.h>
#include <reducer/reducer.h>
#include <reducer/reducer_config.h>
#include <util/log.h>
#include <util/log_whitelist.h>
#include <util/signal_handler.h>

#include <algorithm>
#include <cctype>
#include <reducer_entrypoint_cxxbridge.h>

namespace {

// Map cxx bridge TsdbFormat to native
static inline reducer::TsdbFormat map_tsdb_format(reducer_cfg::TsdbFormat f)
{
  switch (f) {
  case reducer_cfg::TsdbFormat::prometheus:
    return reducer::TsdbFormat::prometheus;
  case reducer_cfg::TsdbFormat::json:
    return reducer::TsdbFormat::json;
  case reducer_cfg::TsdbFormat::otlp_grpc:
    return reducer::TsdbFormat::otlp_grpc;
  }
  return reducer::TsdbFormat::prometheus;
}

// Trim helper
static inline std::string trim(std::string s)
{
  auto not_space = [](int ch) { return !std::isspace(ch); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
  s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
  return s;
}

template <typename Enum> bool parse_and_set_whitelist(std::string const &value)
{
  if (value.empty()) {
    return true;
  }
  // '*' => enable all for this whitelist
  if (value == "*") {
    log_whitelist_all<Enum>();
    return true;
  }

  std::list<Enum> enums;
  std::list<std::string> errors;
  std::size_t start = 0;
  while (start <= value.size()) {
    std::size_t comma = value.find(',', start);
    auto token = trim(value.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
    if (!token.empty()) {
      Enum e{};
      if (enum_from_string(token, e)) {
        enums.push_back(e);
      } else {
        errors.push_back(token);
      }
    }
    if (comma == std::string::npos) {
      break;
    }
    start = comma + 1;
  }

  if (!errors.empty()) {
    std::string err;
    for (auto const &t : errors) {
      if (!err.empty())
        err += ",";
      err += t;
    }
    LOG::error("Unable to enable requested logs for whitelist: {}", err);
    return false;
  }

  if (!enums.empty()) {
    set_log_whitelist<Enum>(std::move(enums));
  }
  return true;
}

static reducer::ReducerConfig from_ffi(reducer_cfg::ReducerConfig const &in)
{
  reducer::ReducerConfig out;
  out.telemetry_port = in.telemetry_port;

  out.num_ingest_shards = in.num_ingest_shards;
  out.num_matching_shards = in.num_matching_shards;
  out.num_aggregation_shards = in.num_aggregation_shards;
  out.partitions_per_shard = in.partitions_per_shard;

  out.enable_id_id = in.enable_id_id;
  out.enable_az_id = in.enable_az_id;
  out.enable_flow_logs = in.enable_flow_logs;

  out.enable_otlp_grpc_metrics = in.enable_otlp_grpc_metrics;
  out.otlp_grpc_metrics_address = std::string(in.otlp_grpc_metrics_address);
  out.otlp_grpc_metrics_port = in.otlp_grpc_metrics_port;
  out.otlp_grpc_batch_size = in.otlp_grpc_batch_size;
  out.enable_otlp_grpc_metric_descriptions = in.enable_otlp_grpc_metric_descriptions;

  out.disable_prometheus_metrics = in.disable_prometheus_metrics;
  out.shard_prometheus_metrics = in.shard_prometheus_metrics;
  out.prom_bind = std::string(in.prom_bind);
  if (in.scrape_size_limit_bytes == 0) {
    out.scrape_size_limit_bytes = std::nullopt;
  } else {
    out.scrape_size_limit_bytes = in.scrape_size_limit_bytes;
  }
  out.internal_prom_bind = std::string(in.internal_prom_bind);
  if (in.stats_scrape_size_limit_bytes == 0) {
    out.stats_scrape_size_limit_bytes = std::nullopt;
  } else {
    out.stats_scrape_size_limit_bytes = in.stats_scrape_size_limit_bytes;
  }
  out.scrape_metrics_tsdb_format = map_tsdb_format(in.scrape_metrics_tsdb_format);

  out.disable_node_ip_field = in.disable_node_ip_field;
  out.enable_autonomous_system_ip = in.enable_autonomous_system_ip;

  if (!in.geoip_path.empty()) {
    out.geoip_path = std::string(in.geoip_path);
  } else {
    out.geoip_path = std::nullopt;
  }

  out.enable_aws_enrichment = in.enable_aws_enrichment;
  out.enable_percentile_latencies = in.enable_percentile_latencies;

  out.disable_metrics = std::string(in.disable_metrics);
  out.enable_metrics = std::string(in.enable_metrics);

  out.index_dump_interval = in.index_dump_interval;

  return out;
}

} // namespace

// Thin C++ entrypoint: accepts final config from Rust and runs the reducer.
namespace reducer_cfg {
int otn_reducer_main_with_config(reducer_cfg::ReducerConfig const &cfg)
{
  uv_loop_t loop;
  CHECK_UV(uv_loop_init(&loop));

  SignalManager signal_manager(loop, "reducer");
  // Initialize minimal signal handler behavior (ignore SIGPIPE, disable core dumps)
  signal_manager.handle();

  // Apply logging whitelist configuration
  if (cfg.log_whitelist_all) {
    log_whitelist_all_globally();
  }
  (void)parse_and_set_whitelist<ClientType>(std::string(cfg.log_whitelist_client_type));
  (void)parse_and_set_whitelist<NodeResolutionType>(std::string(cfg.log_whitelist_node_resolution_type));
  (void)parse_and_set_whitelist<channel::Component>(std::string(cfg.log_whitelist_channel));
  (void)parse_and_set_whitelist<reducer::ingest::Component>(std::string(cfg.log_whitelist_ingest));
  (void)parse_and_set_whitelist<reducer::matching::Component>(std::string(cfg.log_whitelist_matching));

  // Convert config
  reducer::ReducerConfig config = from_ffi(cfg);

  // Validate TSDB format for scraped metrics: only prometheus/json are supported here
  if (config.scrape_metrics_tsdb_format != reducer::TsdbFormat::prometheus &&
      config.scrape_metrics_tsdb_format != reducer::TsdbFormat::json) {
    LOG::critical(
        "Invalid TSDB format for scraped metrics: {}. Supported formats: {}, {}",
        to_string(config.scrape_metrics_tsdb_format),
        reducer::TsdbFormat::prometheus,
        reducer::TsdbFormat::json);
    return 1;
  }

  reducer::Reducer reducer(loop, config);
  signal_manager.handle_signals({SIGINT, SIGTERM}, std::bind(&reducer::Reducer::shutdown, &reducer));
  reducer.startup();

  return 0;
}

void otn_init_logging(bool log_console, bool no_log_file)
{
  auto const log_file = std::string(LOG::log_file_path());
  LOG::init(log_console, no_log_file ? nullptr : &log_file);
}

void otn_set_log_level(int level_code)
{
  switch (level_code) {
  case 0:
    spdlog::set_level(spdlog::level::trace);
    break;
  case 1:
    spdlog::set_level(spdlog::level::debug);
    break;
  case 2:
    spdlog::set_level(spdlog::level::info);
    break;
  case 3:
    spdlog::set_level(spdlog::level::warn);
    break;
  case 4:
    spdlog::set_level(spdlog::level::err);
    break;
  case 5:
    spdlog::set_level(spdlog::level::critical);
    break;
  default:
    break; // no-op on unknown
  }
}
} // namespace reducer_cfg
