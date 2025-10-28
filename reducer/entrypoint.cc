// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include <channel/component.h>
#include <reducer/ingest/component.h>
#include <reducer/matching/component.h>
#include <reducer/reducer.h>
#include <reducer/reducer_config.h>
#include <util/args_parser.h>
#include <util/signal_handler.h>

static int reducer_run(int argc, char **argv)
{
  uv_loop_t loop;
  CHECK_UV(uv_loop_init(&loop));

  ////////////////////////////////////////////////////////////////////////////////
  // Command-line flags.
  // IMPORTANT: do not provide a default value for flags that correspond to
  // ReducerConfig fields, otherwise those defaults will override values set in
  // DEFAULT_REDUCER_CONFIG and those provided in the configuration file.
  //

  cli::ArgsParser parser("OpenTelemetry eBPF Reducer.");

  // Main.
  //
  args::HelpFlag help(*parser, "help", "Display this help menu", {'h', "help"});
  args::ValueFlag<std::string> config_file(*parser, "config_file", "Path to the configuration file", {"c", "config-file"});
  args::Flag print_config(*parser, "print_config", "Print configuration values to stdout", {"print-config"});
  args::ValueFlag<u32> telemetry_port(
      *parser, "port", "TCP port to listen on for incoming connections from collectors", {'p', "port"});
  args::ValueFlag<std::string> metrics_tsdb_format_flag(
      *parser, "prometheus|json", "Format of TSDB data for scraped metrics", {"metrics-tsdb-format"}, "prometheus");

  // Features.
  //
  args::Flag enable_aws_enrichment(
      *parser,
      "enable_aws_enrichment",
      "Enables enrichment using AWS metadata received from the Cloud Collector",
      {"enable-aws-enrichment"});
  args::Flag disable_node_ip_field(
      *parser, "disable_node_ip_field", "Disables the IP addresses field in node spans", {"disable-node-ip-field"});
  args::Flag enable_id_id(*parser, "enable_id_id", "Enables id-id timeseries generation", {"enable-id-id"});
  args::Flag enable_az_id(*parser, "enable_az_id", "Enables az-id timeseries generation", {"enable-az-id"});
  args::Flag enable_flow_logs(*parser, "enable_flow_logs", "Enables exporting metric flow logs", {"enable-flow-logs"});
  args::Flag enable_autonomous_system_ip(
      *parser,
      "enable_autonomous_system_ip",
      "Enables using IP addresses for autonomous systems",
      {"enable-autonomous-system-ip"});
  args::Flag enable_percentile_latencies(
      *parser,
      "enable_percentile_latencies",
      "Enables computation and output of pXX latency timeseries",
      {"enable-percentile-latencies"});

  // Scaling.
  //
  args::ValueFlag<u32> num_ingest_shards(*parser, "num_ingest_shards", "How many ingest shards to run.", {"num-ingest-shards"});
  args::ValueFlag<u32> num_matching_shards(
      *parser, "num_matching_shards", "How many matching shards to run.", {"num-matching-shards"});
  args::ValueFlag<u32> num_aggregation_shards(
      *parser, "num_aggregation_shards", "How many aggregation shards to run.", {"num-aggregation-shards"});
  args::ValueFlag<u32> partitions_per_shard(
      *parser, "count", "How many partitions per aggregation shard to write metrics into.", {"partitions-per-shard"});

  // Prometheus output.
  //
  args::Flag disable_prometheus_metrics(
      *parser, "disable_prometheus_metrics", "Disables prometheus metrics output", {"disable-prometheus-metrics"});
  args::Flag shard_prometheus_metrics(
      *parser, "shard_prometheus_metrics", "Partitions prometheus metrics", {"shard-prometheus-metrics"});
  args::ValueFlag<std::string> prom_bind(*parser, "prometheus_bind", "Bind address for Prometheus", {"prom"});
  args::ValueFlag<u64> scrape_size_limit_bytes(
      *parser, "scrape_size_limit", "Maximum size of a scrape response, in bytes.", {"scrape-size-limit-bytes"});

  // OTLP gRPC output.
  //
  args::Flag enable_otlp_grpc_metrics(
      *parser, "enable_otlp_grpc_metrics", "Enables OTLP gRPC metrics output", {"enable-otlp-grpc-metrics"});
  args::ValueFlag<std::string> otlp_grpc_metrics_address(
      *parser,
      "otlp_grpc_metrics_address",
      "Network address to send OTLP gRPC metrics",
      {"otlp-grpc-metrics-host"},
      "localhost");
  args::ValueFlag<u32> otlp_grpc_metrics_port(
      *parser, "otlp_grpc_metrics_port", "TCP port to send OTLP gRPC metrics", {"otlp-grpc-metrics-port"});
  args::ValueFlag<int> otlp_grpc_batch_size(*parser, "otlp_grpc_batch_size", "", {"otlp-grpc-batch-size"});
  args::Flag enable_otlp_grpc_metric_descriptions(
      *parser,
      "enable_otlp_grpc_metric_descriptions",
      "Enables sending metric descriptions in OTLP gRPC metrics output",
      {"enable-otlp-grpc-metric-descriptions"});

  // Metrics output.
  //
  auto disable_metrics = parser.add_arg<std::string>(
      "disable-metrics",
      "A comma (,) separated list of metrics to disable.\n"
      "A metric group can also be disabled. To do so, specify '<group>.all', where <group> is one of: tcp,udp,dns,http.\n"
      "A value of 'none' can be given to enable all metrics.\n\n"
      "If this argument is not specified, the recommended collection of metrics will be used.\n\n"
      "Example: disable-metrics=http.all,dns.all,udp.drops\n"
      "This example will disable all http metrics, all dns metrics, and the udp.drops metric.",
      {"disable-metrics"});

  auto enable_metrics = parser.add_arg<std::string>(
      "enable-metrics",
      "A comma (,) separated list of metrics to enable.  This list is processed AFTER disable-metrics\n"
      "A metric group can also be enabled. To do so, specify '<group>.all', where <group> is one of: tcp,udp,dns,http.\n"
      "Example: enable-metrics=http.all,dns.all,udp.drops\n"
      "This example will enable all http metrics, all dns metrics, and the udp.drops metric.",
      {"enable-metrics"});

  // Internal stats.
  //
  args::ValueFlag<std::string> internal_prom_bind(
      *parser, "prometheus_bind", "Bind address for Internal Prometheus", {"internal-prom"});
  args::ValueFlag<u64> stats_scrape_size_limit_bytes(
      *parser,
      "stats_scrape_size_limit",
      "Maximum size of internal stats scrape response, in bytes.",
      {"stats-scrape-size-limit-bytes"});

  // Logging and debugging.
  //
  auto index_dump_interval = parser.add_arg<u64>(
      "index-dump-interval",
      "Interval (in seconds) to generate a JSON dump of the span indexes for each core."
      " A value of 0 disables index dumping.");
  parser.new_handler<LogWhitelistHandler<ClientType>>("client-type");
  parser.new_handler<LogWhitelistHandler<NodeResolutionType>>("node-resolution-type");
  parser.new_handler<LogWhitelistHandler<channel::Component>>("channel");
  parser.new_handler<LogWhitelistHandler<reducer::ingest::Component>>("ingest");
  parser.new_handler<LogWhitelistHandler<reducer::matching::Component>>("matching");

  SignalManager signal_manager(loop, "reducer");

  // Parse the command-line, bomb out on error.
  if (auto result = parser.process(argc, argv); !result) {
    return result.error();
  }

  // Initialize minimal signal handler behavior (ignore SIGPIPE, disable core dumps)
  signal_manager.handle();

  reducer::ReducerConfig config = reducer::DEFAULT_REDUCER_CONFIG;

  if (config_file) {
    try {
      reducer::read_config_from_yaml(config, config_file.Get());
    } catch (std::exception const &exc) {
      std::cerr << "Unable to load configuration file: " << exc.what() << std::endl;
      return 1;
    }
  }

#define SET_CONFIG(var, flag)                                                                                                  \
  if (flag) {                                                                                                                  \
    var = flag.Get();                                                                                                          \
  }

  SET_CONFIG(config.telemetry_port, telemetry_port);

  SET_CONFIG(config.num_ingest_shards, num_ingest_shards);
  SET_CONFIG(config.num_matching_shards, num_matching_shards);
  SET_CONFIG(config.num_aggregation_shards, num_aggregation_shards);
  SET_CONFIG(config.partitions_per_shard, partitions_per_shard);

  SET_CONFIG(config.enable_id_id, enable_id_id);
  SET_CONFIG(config.enable_az_id, enable_az_id);
  SET_CONFIG(config.enable_flow_logs, enable_flow_logs);

  SET_CONFIG(config.enable_otlp_grpc_metrics, enable_otlp_grpc_metrics);
  SET_CONFIG(config.otlp_grpc_metrics_address, otlp_grpc_metrics_address);
  SET_CONFIG(config.otlp_grpc_metrics_port, otlp_grpc_metrics_port);
  SET_CONFIG(config.otlp_grpc_batch_size, otlp_grpc_batch_size);
  SET_CONFIG(config.enable_otlp_grpc_metric_descriptions, enable_otlp_grpc_metric_descriptions);

  SET_CONFIG(config.disable_prometheus_metrics, disable_prometheus_metrics);
  SET_CONFIG(config.shard_prometheus_metrics, shard_prometheus_metrics);
  SET_CONFIG(config.prom_bind, prom_bind);
  SET_CONFIG(config.internal_prom_bind, internal_prom_bind);

  SET_CONFIG(config.disable_node_ip_field, disable_node_ip_field);
  SET_CONFIG(config.enable_autonomous_system_ip, enable_autonomous_system_ip);

  SET_CONFIG(config.enable_aws_enrichment, enable_aws_enrichment);
  SET_CONFIG(config.enable_percentile_latencies, enable_percentile_latencies);

  SET_CONFIG(config.disable_metrics, disable_metrics);
  SET_CONFIG(config.enable_metrics, enable_metrics);

  SET_CONFIG(config.index_dump_interval, index_dump_interval);

  SET_CONFIG(config.scrape_size_limit_bytes, scrape_size_limit_bytes);

#undef SET_CONFIG

  config.stats_scrape_size_limit_bytes =
      stats_scrape_size_limit_bytes ? stats_scrape_size_limit_bytes.Get() : config.scrape_size_limit_bytes;

  if (metrics_tsdb_format_flag) {
    if (!enum_from_string(metrics_tsdb_format_flag.Get(), config.scrape_metrics_tsdb_format)) {
      LOG::critical("Unknown TSDB format: {}", metrics_tsdb_format_flag.Get());
      return 1;
    }
  }

  if (config.scrape_metrics_tsdb_format != reducer::TsdbFormat::prometheus &&
      config.scrape_metrics_tsdb_format != reducer::TsdbFormat::json) {
    LOG::critical(
        "Invalid TSDB format for scraped metrics: {}. Supported formats: {}, {}",
        metrics_tsdb_format_flag.Get(),
        reducer::TsdbFormat::prometheus,
        reducer::TsdbFormat::json);
    return 1;
  }

  if (auto val = std::getenv(GEOIP_PATH_VAR); (val != nullptr) && (strlen(val) > 0)) {
    config.geoip_path = val;
  }

  if (print_config) {
    std::cout << config;
  }

  reducer::Reducer reducer(loop, config);
  signal_manager.handle_signals({SIGINT, SIGTERM}, std::bind(&reducer::Reducer::shutdown, &reducer));
  reducer.startup();

  return 0;
}

extern "C" int otn_reducer_main(int argc, const char **argv)
{
  return reducer_run(argc, const_cast<char **>(argv));
}
