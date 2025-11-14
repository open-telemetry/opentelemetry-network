// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include <reducer/constants.h>
#include <reducer/core_type.h>
#include <reducer/disabled_metrics.h>
#include <reducer/ingest/agent_span.h>
#include <reducer/ingest/component.h>
#include <reducer/matching/component.h>
#include <reducer/null_publisher.h>
#include <reducer/otlp_grpc_formatter.h>
#include <reducer/otlp_grpc_publisher.h>
#include <reducer/prometheus_publisher.h>
#include <reducer/reducer.h>
#include <reducer/reducer_config.h>
#include <reducer/util/index_dumper.h>

#include <channel/component.h>
#include <common/client_type.h>

#include <geoip/geoip.h>

#include <util/boot_time.h>
#include <util/debug.h>
#include <util/environment_variables.h>
#include <util/error_handling.h>
#include <util/file_ops.h>
#include <util/log.h>
#include <util/uv_helpers.h>

#include <spdlog/fmt/chrono.h>
#include <spdlog/spdlog.h>

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

namespace reducer {

Reducer::Reducer(uv_loop_t &loop, ReducerConfig &config)
    : loop_(loop),
      config_(config),
      ingest_to_matching_queues_(config_.num_ingest_shards, config_.num_matching_shards),
      ingest_to_logging_queues_(config_.num_ingest_shards, 1),
      matching_to_logging_queues_(config_.num_matching_shards, 1),
      matching_to_aggregation_queues_(config_.num_matching_shards, config_.num_aggregation_shards),
      aggregation_to_logging_queues_(config_.num_aggregation_shards, 1)
{}

void Reducer::startup()
{
  LOG::info("Starting OpenTelemetry eBPF Reducer version {} ({})", versions::release, release_mode_string);

  init_config();
  init_cores();
  start_threads();

  uv_run(&loop_, UV_RUN_DEFAULT);
  close_uv_loop_cleanly(&loop_);

  for (auto &thread : threads_) {
    thread.join();
  }

  LOG::info("OpenTelemetry eBPF Reducer exiting");
}

void Reducer::shutdown()
{
  LOG::info("Stopping ingest core threads...");
  ingest_core_->stop_async();

  for (std::size_t i = 0; i < matching_cores_.size(); ++i) {
    LOG::info("Stopping matching core thread {}...", i);
    matching_cores_[i]->stop_async();
  }

  for (std::size_t i = 0; i < agg_cores_.size(); ++i) {
    LOG::info("Stopping aggregation core thread {}...", i);
    agg_cores_[i]->stop_async();
  }

  LOG::info("Stopping logging core thread...");
  logging_core_->stop_async();

  LOG::info("Stopping main loop...");
  uv_stop(&loop_);
}

void Reducer::init_config()
{
  reducer::aggregation::AggCore::set_id_id_enabled(config_.enable_id_id);
  reducer::aggregation::AggCore::set_az_id_enabled(config_.enable_az_id);
  reducer::aggregation::AggCore::set_flow_logs_enabled(config_.enable_flow_logs);

  reducer::logging::LoggingCore::set_otlp_formatted_internal_metrics_enabled(config_.enable_otlp_grpc_metrics);
  reducer::OtlpGrpcFormatter::set_metric_description_field_enabled(config_.enable_otlp_grpc_metric_descriptions);

  reducer::aggregation::AggCore::set_node_ip_field_disabled(config_.disable_node_ip_field);
  reducer::matching::MatchingCore::set_autonomous_system_ip_enabled(config_.enable_autonomous_system_ip);

  // If the path to a GeoIP database is defined, try loading the database here
  // and print an error message if it fails.
  // Unfortunately, the database structure is not thread safe and is not
  // cloneable, so each matching shard will have to load it again for itself.
  //
  if (config_.geoip_path) {
    try {
      geoip::database geoip_db(config_.geoip_path->c_str());
      LOG::info("Loaded GeoIP database from '{}'.", *config_.geoip_path);
    } catch (std::exception &exc) {
      LOG::error("Failed to load GeoIP database from '{}': {}.", *config_.geoip_path, exc.what());
      config_.geoip_path.reset(); // so matching shards don't try loading it for naught
    }
  }

  if (config_.index_dump_interval) {
    // Index dumping is turned on.
    auto dump_interval = std::chrono::seconds{config_.index_dump_interval};

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
        exit(1);
      }
    } else if (!std::filesystem::is_directory(dump_dir)) {
      LOG::critical("{} exists but is not a directory!", dump_dir);
      exit(1);
    }

    IndexDumper::set_dump_dir(dump_dir.native());
    IndexDumper::set_cooldown(dump_interval);
  } else {
    IndexDumper::set_dump_dir("");
    IndexDumper::set_cooldown(0s);
  }
}

void Reducer::init_cores()
{
  const size_t num_stat_writers = 1; // one for the logging core

  if (config_.enable_otlp_grpc_metrics) {
    stats_publisher_ = std::make_unique<reducer::OtlpGrpcPublisher>(
        num_stat_writers,
        std::string(config_.otlp_grpc_metrics_address + ":" + std::to_string(config_.otlp_grpc_metrics_port)));
  } else {
    stats_publisher_ = std::make_unique<reducer::PrometheusPublisher>(
        reducer::PrometheusPublisher::SINGLE_PORT,
        num_stat_writers,
        config_.internal_prom_bind,
        1,
        config_.stats_scrape_size_limit_bytes);
  }

  // index of the next stat writer thread to make a writer for
  size_t stat_writer_num = 0;

  auto initial_timestamp = monotonic() + get_boot_time();

  reducer::DisabledMetrics disabled_metrics(config_.disable_metrics, config_.enable_metrics);
  logging_core_ = std::make_unique<reducer::logging::LoggingCore>(
      ingest_to_logging_queues_,
      matching_to_logging_queues_,
      aggregation_to_logging_queues_,
      stats_publisher_->make_writer(stat_writer_num++),
      disabled_metrics,
      /* shard */ 0,
      initial_timestamp);

  // matching, logging and aggregation cores are receiving messages only from
  // other in-process core(s), so authentication is not needed
  logging_core_->set_connection_authenticated();

  agg_cores_.reserve(config_.num_aggregation_shards);
  for (size_t shard = 0; shard < config_.num_aggregation_shards; ++shard) {
    std::string otlp_endpoint;
    if (config_.enable_otlp_grpc_metrics) {
      otlp_endpoint = std::string(config_.otlp_grpc_metrics_address + ":" + std::to_string(config_.otlp_grpc_metrics_port));
    }

    auto agg_core = std::make_unique<reducer::aggregation::AggCore>(
        matching_to_aggregation_queues_,
        aggregation_to_logging_queues_,
        shard,
        initial_timestamp,
        otlp_endpoint,
        config_.disable_node_ip_field);

    agg_core->set_connection_authenticated();
    agg_cores_.push_back(std::move(agg_core));
  }

  reducer::matching::MatchingCore::enable_aws_enrichment(config_.enable_aws_enrichment);

  matching_cores_.reserve(config_.num_matching_shards);
  for (size_t shard = 0; shard < config_.num_matching_shards; ++shard) {
    auto matching_core = std::make_unique<reducer::matching::MatchingCore>(
        ingest_to_matching_queues_,
        matching_to_aggregation_queues_,
        matching_to_logging_queues_,
        config_.geoip_path,
        shard,
        initial_timestamp);
    matching_core->set_connection_authenticated();
    matching_cores_.push_back(std::move(matching_core));
  }

  ingest_core_ = std::make_unique<reducer::ingest::IngestCore>(
      ingest_to_logging_queues_, ingest_to_matching_queues_, config_.telemetry_port);

  // all writers created
  ASSUME(stat_writer_num == num_stat_writers);
}

void Reducer::start_threads()
{
  threads_.reserve(config_.num_matching_shards + config_.num_aggregation_shards + 3);

  // start all threads
  threads_.emplace_back(&reducer::logging::LoggingCore::run, logging_core_.get());
  for (auto &agg_core : agg_cores_) {
    threads_.emplace_back(&reducer::aggregation::AggCore::run, agg_core.get());
  }
  for (auto &matching_core : matching_cores_) {
    threads_.emplace_back(&reducer::matching::MatchingCore::run, matching_core.get());
  }
  threads_.emplace_back(&reducer::ingest::IngestCore::run, ingest_core_.get());
}

} // namespace reducer
