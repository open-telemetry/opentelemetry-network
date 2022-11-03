// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include <reducer/aggregation/agg_core.h>
#include <reducer/constants.h>
#include <reducer/core_type.h>
#include <reducer/disabled_metrics.h>
#include <reducer/ingest/agent_span.h>
#include <reducer/ingest/component.h>
#include <reducer/ingest/ingest_core.h>
#include <reducer/logging/logging_core.h>
#include <reducer/matching/component.h>
#include <reducer/matching/matching_core.h>
#include <reducer/null_publisher.h>
#include <reducer/otlp_grpc_formatter.h>
#include <reducer/otlp_grpc_publisher.h>
#include <reducer/prometheus_publisher.h>
#include <reducer/rpc_queue_matrix.h>
#include <reducer/tsdb_format.h>
#include <reducer/util/index_dumper.h>

#include <channel/component.h>
#include <common/client_type.h>
#include <jitbuf/transformer.h>

#include <geoip/geoip.h>

#include <otlp/otlp_grpc_metrics_client.h>

#include <util/args_parser.h>
#include <util/boot_time.h>
#include <util/debug.h>
#include <util/environment_variables.h>
#include <util/file_ops.h>
#include <util/log.h>
#include <util/signal_handler.h>
#include <util/uv_helpers.h>

#include <spdlog/spdlog.h>

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

int main(int argc, char *argv[])
{
  uv_loop_t loop;
  CHECK_UV(uv_loop_init(&loop));

  ////////////////////////////////////////////////////////////////////////////////
  // Command-line flags.
  //

  cli::ArgsParser parser("OpenTelemetry eBPF Reducer.");

  // Main.
  //
  args::HelpFlag help(*parser, "help", "Display this help menu", {'h', "help"});
  args::ValueFlag<u32> telemetry_port(
      *parser, "port", "TCP port to listen on for incoming connections from collectors", {'p', "port"}, 8000);
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
  args::Flag enable_az_node(*parser, "enable_az_node", "Enables az-node timeseries generation", {"enable-az-node"});
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

  args::ValueFlag<int> otlp_grpc_batch_size_flag(*parser, "otlp_grpc_batch_size", "", {"otlp-grpc-batch-size"}, 1000);

  // Scaling.
  //
  args::ValueFlag<uint32_t> num_ingest_shards_flag(
      *parser, "num_ingest_shards", "How many ingest shards to run.", {"num-ingest-shards"}, 1);
  args::ValueFlag<uint32_t> num_matching_shards_flag(
      *parser, "num_matching_shards", "How many matching shards to run.", {"num-matching-shards"}, 1);
  args::ValueFlag<uint32_t> num_aggregation_shards_flag(
      *parser, "num_aggregation_shards", "How many aggregation shards to run.", {"num-aggregation-shards"}, 1);
  args::ValueFlag<uint32_t> partitions_per_shard(
      *parser, "count", "How many partitions per aggregation shard to write metrics into.", {"partitions-per-shard"}, 1);

  // Prometheus output.
  //
  args::Flag disable_prometheus_metrics(
      *parser, "disable_prometheus_metrics", "Disables prometheus metrics output", {"disable-prometheus-metrics"});
  args::ValueFlag<std::string> prom_bind(*parser, "prometheus_bind", "Bind address for Prometheus", {"prom"}, "127.0.0.1:7010");
  args::ValueFlag<uint32_t> scrape_size_limit_bytes_flag(
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
      *parser, "otlp_grpc_metrics_port", "TCP port to send OTLP gRPC metrics", {"otlp-grpc-metrics-port"}, 4317);
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
      "Example: disable-metrics=http.all;dns.all;udp.drops\n"
      "This example will disable all http metrics, all dns metrics, and the udp.drops metric.",
      {"disable-metrics"},
      "");

  auto enable_metrics = parser.add_arg<std::string>(
      "enable-metrics",
      "A comma (,) separated list of metrics to enable.  This list is processed AFTER disable-metrics\n"
      "A metric group can also be enabled. To do so, specify '<group>.all', where <group> is one of: tcp,udp,dns,http.\n"
      "Example: enable-metrics=http.all;dns.all;udp.drops\n"
      "This example will enable all http metrics, all dns metrics, and the udp.drops metric.",
      {"enable-metrics"},
      "");

  // Internal stats.
  //
  args::ValueFlag<std::string> internal_prom_bind(
      *parser, "prometheus_bind", "Bind address for Internal Prometheus", {"internal-prom"}, "0.0.0.0:7001");
  args::ValueFlag<uint32_t> stats_scrape_size_limit_bytes_flag(
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

  SignalManager &signal_manager = parser.new_handler<SignalManager>(loop, "reducer");

  // Parse the command-line, bomb out on error.
  if (auto result = parser.process(argc, argv); !result) {
    return result.error();
  }

  ////////////////////////////////////////////////////////////////////////////////

  auto const num_aggregation_shards = num_aggregation_shards_flag.Get();
  auto const num_matching_shards = num_matching_shards_flag.Get();
  auto const num_ingest_shards = num_ingest_shards_flag.Get();
  if (disable_node_ip_field.Get()) {
    reducer::aggregation::AggCore::disable_node_ip_field();
  }

  if (enable_id_id.Get()) {
    reducer::aggregation::AggCore::enable_id_id();
  }

  if (enable_az_node.Get()) {
    reducer::aggregation::AggCore::enable_az_node();
  }

  if (enable_autonomous_system_ip.Get()) {
    reducer::matching::MatchingCore::enable_autonomous_system_ip();
  }

  if (enable_otlp_grpc_metrics.Get()) {
    reducer::aggregation::AggCore::enable_otlp_formatted_internal_metrics();
    reducer::logging::LoggingCore::enable_otlp_formatted_internal_metrics();
    reducer::matching::MatchingCore::enable_otlp_formatted_internal_metrics();
    reducer::ingest::IngestCore::enable_otlp_formatted_internal_metrics();
  }

  if (enable_otlp_grpc_metric_descriptions.Get()) {
    reducer::OtlpGrpcFormatter::enable_metric_description_field();
  }

  reducer::TsdbFormat metrics_tsdb_format;
  if (!enum_from_string(metrics_tsdb_format_flag.Get(), metrics_tsdb_format)) {
    LOG::critical("Unknown TSDB format: {}", metrics_tsdb_format_flag.Get());
    return 1;
  } else {
    if (metrics_tsdb_format != reducer::TsdbFormat::prometheus && metrics_tsdb_format != reducer::TsdbFormat::json) {
      LOG::critical(
          "Invalid TSDB format for scraped metrics: {}. Supported formats: {}, {}",
          metrics_tsdb_format_flag.Get(),
          reducer::TsdbFormat::prometheus,
          reducer::TsdbFormat::json);
      return 1;
    }
  }

  if (index_dump_interval) {
    // Index dumping is turned on.
    auto dump_interval = std::chrono::seconds{*index_dump_interval};

    std::filesystem::path dump_dir;
    if (auto data_dir = try_get_env_var(DATA_DIR_VAR); !data_dir.empty()) {
      dump_dir = std::filesystem::path(data_dir) / "dump";
    } else {
      dump_dir = std::filesystem::current_path() / "dump";
    }

    LOG::info("Dumping indexes to {} at {} interval", dump_dir, dump_interval);

    if (!std::filesystem::exists(dump_dir)) {
      std::error_code ec;
      if (!std::filesystem::create_directories(dump_dir, ec)) {
        LOG::critical("Could not create directory {}: {}", dump_dir, ec);
        return 1;
      }
    } else if (!std::filesystem::is_directory(dump_dir)) {
      LOG::critical("{} exists but is not a directory!", dump_dir);
      return 1;
    }

    IndexDumper::set_dump_dir(dump_dir.native());
    IndexDumper::set_cooldown(dump_interval);
  }

  std::optional<std::string> geoip_path;
  if (auto val = std::getenv(GEOIP_PATH_VAR); (val != nullptr) && (strlen(val) > 0)) {
    geoip_path = val;
  }

  global_otlp_grpc_batch_size = otlp_grpc_batch_size_flag.Get();

  /* log reducer version */
  LOG::info("Starting OpenTelemetry eBPF Reducer version {} ({})", versions::release, release_mode_string);

  // If the path to a GeoIP database is defined, try loading the database here
  // and print an error message if it fails.
  // Unfortunately, the database structure is not thread safe and is not
  // cloneable, so each matching shard will have to load it again for itself.
  //
  if (geoip_path) {
    try {
      geoip::database geoip_db(geoip_path->c_str());
      LOG::info("Loaded GeoIP database from '{}'.", *geoip_path);
    } catch (std::exception &exc) {
      LOG::error("Failed to load GeoIP database from '{}': {}.", *geoip_path, exc.what());
      geoip_path.reset(); // so matching shards don't try loading it for naught
    }
  }

  jitbuf::initialize_llvm();

  std::optional<u64> scrape_size_limit_bytes;
  if (scrape_size_limit_bytes_flag) {
    scrape_size_limit_bytes = scrape_size_limit_bytes_flag.Get();
  }

  std::optional<u64> stats_scrape_size_limit_bytes;
  if (stats_scrape_size_limit_bytes_flag) {
    // use the command-line provided value
    stats_scrape_size_limit_bytes = stats_scrape_size_limit_bytes_flag.Get();
  } else {
    // use the overall scrape limit
    stats_scrape_size_limit_bytes = scrape_size_limit_bytes;
  }

  // 2 is one logging core and one for ingest, since ingest core writes stats
  // from just one thread
  const size_t num_stat_writers = num_ingest_shards + 2;
  const size_t num_prom_metric_writers = num_aggregation_shards * partitions_per_shard.Get();

  std::unique_ptr<reducer::Publisher> stats_publisher;

  if (enable_otlp_grpc_metrics.Get()) {
    stats_publisher = std::make_unique<reducer::OtlpGrpcPublisher>(
        num_stat_writers, std::string(otlp_grpc_metrics_address.Get() + ":" + std::to_string(otlp_grpc_metrics_port.Get())));
  } else {
    stats_publisher = std::make_unique<reducer::PrometheusPublisher>(
        reducer::PrometheusPublisher::SINGLE_PORT,
        num_stat_writers,
        internal_prom_bind.Get(),
        1,
        stats_scrape_size_limit_bytes);
  }

  std::unique_ptr<reducer::Publisher> prom_metrics_publisher;
  if (!disable_prometheus_metrics.Get()) {
    prom_metrics_publisher = std::make_unique<reducer::PrometheusPublisher>(
        reducer::PrometheusPublisher::PORT_RANGE,
        num_prom_metric_writers,
        prom_bind.Get(),
        num_prom_metric_writers,
        scrape_size_limit_bytes);
  }

  std::unique_ptr<reducer::Publisher> otlp_metrics_publisher;
  if (enable_otlp_grpc_metrics.Get()) {
    otlp_metrics_publisher = std::make_unique<reducer::OtlpGrpcPublisher>(
        num_aggregation_shards,
        std::string(otlp_grpc_metrics_address.Get() + ":" + std::to_string(otlp_grpc_metrics_port.Get())));
  }

  // index of the next stat writer thread to make a writer for
  size_t stat_writer_num = 0;
  // index of the next prom metrics writer
  size_t prom_metric_writer_num = 0;
  // index of the next otlp metrics writer
  size_t otlp_metric_writer_num = 0;

  reducer::RpcQueueMatrix ingest_to_matching_queues(num_ingest_shards, num_matching_shards);

  reducer::RpcQueueMatrix ingest_to_logging_queues(num_ingest_shards, 1);

  reducer::RpcQueueMatrix matching_to_logging_queues(num_matching_shards, 1);

  reducer::RpcQueueMatrix matching_to_aggregation_queues(num_matching_shards, num_aggregation_shards);

  reducer::RpcQueueMatrix aggregation_to_logging_queues(num_aggregation_shards, 1);

  auto initial_timestamp = monotonic() + get_boot_time();
  reducer::DisabledMetrics disabled_metrics(*disable_metrics, *enable_metrics);

  auto logging_core = std::make_unique<reducer::logging::LoggingCore>(
      ingest_to_logging_queues,
      matching_to_logging_queues,
      aggregation_to_logging_queues,
      stats_publisher->make_writer(stat_writer_num++),
      disabled_metrics,
      /* shard */ 0,
      initial_timestamp);

  // matching, logging and aggregation cores are receiving messages only from
  // other in-process core(s), so authentication is not needed
  logging_core->set_connection_authenticated();

  std::vector<std::unique_ptr<reducer::aggregation::AggCore>> agg_cores;
  agg_cores.reserve(num_aggregation_shards);
  for (size_t shard = 0; shard < num_aggregation_shards; ++shard) {
    std::vector<reducer::Publisher::WriterPtr> prom_metric_writers;
    if (prom_metrics_publisher) {
      prom_metric_writers.reserve(partitions_per_shard.Get());
      for (size_t i = 0; i < partitions_per_shard.Get(); ++i) {
        prom_metric_writers.emplace_back(prom_metrics_publisher->make_writer(prom_metric_writer_num++));
      }
    }

    reducer::Publisher::WriterPtr otlp_metric_writer;
    if (otlp_metrics_publisher) {
      otlp_metric_writer = otlp_metrics_publisher->make_writer(otlp_metric_writer_num++);
    }

    auto agg_core = std::make_unique<reducer::aggregation::AggCore>(
        matching_to_aggregation_queues,
        aggregation_to_logging_queues,
        prom_metrics_publisher,
        std::move(prom_metric_writers),
        otlp_metrics_publisher,
        std::move(otlp_metric_writer),
        enable_percentile_latencies.Get(),
        metrics_tsdb_format,
        disabled_metrics,
        shard,
        initial_timestamp);

    agg_core->set_connection_authenticated();
    agg_cores.push_back(std::move(agg_core));
  }

  reducer::matching::MatchingCore::enable_aws_enrichment(enable_aws_enrichment);

  std::vector<std::unique_ptr<reducer::matching::MatchingCore>> matching_cores;
  matching_cores.reserve(num_matching_shards);

  for (size_t shard = 0; shard < num_matching_shards; ++shard) {
    auto matching_core = std::make_unique<reducer::matching::MatchingCore>(
        ingest_to_matching_queues,
        matching_to_aggregation_queues,
        matching_to_logging_queues,
        geoip_path,
        shard,
        initial_timestamp);
    matching_core->set_connection_authenticated();
    matching_cores.push_back(std::move(matching_core));
  }

  std::vector<reducer::ingest::IngestCore::ShardConfig> ingest_core_shard_config;
  ingest_core_shard_config.reserve(num_ingest_shards);
  for (std::size_t i = 0; i < num_ingest_shards; ++i) {
    ingest_core_shard_config.emplace_back(reducer::ingest::IngestCore::ShardConfig{
        .stats_writer = stats_publisher->make_writer(stat_writer_num++),
    });
  }
  auto ingest_core = std::make_unique<reducer::ingest::IngestCore>(
      ingest_to_logging_queues,
      ingest_to_matching_queues,
      telemetry_port.Get(),
      stats_publisher->make_writer(stat_writer_num++),
      std::move(ingest_core_shard_config));

  // all writers created
  assert(stat_writer_num == num_stat_writers);
  assert(!prom_metrics_publisher || prom_metric_writer_num == num_prom_metric_writers);

  auto terminate = [&]() {
    LOG::info("Stopping ingest core threads...");
    ingest_core->stop_async();

    for (std::size_t i = 0; i < matching_cores.size(); ++i) {
      LOG::info("Stopping matching core thread {}...", i);
      matching_cores[i]->stop_async();
    }

    for (std::size_t i = 0; i < agg_cores.size(); ++i) {
      LOG::info("Stopping aggregation core thread {}...", i);
      agg_cores[i]->stop_async();
    }

    LOG::info("Stopping logging core thread...");
    logging_core->stop_async();

    LOG::info("Stopping main loop...");
    uv_stop(&loop);
  };
  signal_manager.handle_signals({SIGINT, SIGTERM}, terminate);

  std::vector<std::thread> threads;
  threads.reserve(num_matching_shards + num_aggregation_shards + 3);

  // start all threads
  threads.emplace_back(&reducer::logging::LoggingCore::run, logging_core.get());
  for (auto &agg_core : agg_cores) {
    threads.emplace_back(&reducer::aggregation::AggCore::run, agg_core.get());
  }
  for (auto &matching_core : matching_cores) {
    threads.emplace_back(&reducer::matching::MatchingCore::run, matching_core.get());
  }
  threads.emplace_back(&reducer::ingest::IngestCore::run, ingest_core.get());

  uv_run(&loop, UV_RUN_DEFAULT);

  // join all threads
  for (auto &thread : threads) {
    thread.join();
  }

  LOG::info("OpenTelemetry eBPF Reducer exiting");

  return 0;
}
