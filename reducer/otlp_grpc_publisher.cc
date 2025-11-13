// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "otlp_grpc_publisher.h"

#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>

namespace reducer {

OtlpGrpcPublisher::OtlpGrpcPublisher(size_t /*num_writer_threads*/, const std::string &endpoint) : endpoint_(endpoint) {}

OtlpGrpcPublisher::~OtlpGrpcPublisher() {}

Publisher::WriterPtr OtlpGrpcPublisher::make_writer(size_t thread_num)
{
  return std::make_unique<Writer>(thread_num, endpoint_);
}

void OtlpGrpcPublisher::write_internal_stats(InternalMetricsEncoder & /*encoder*/, u64 /*time_ns*/) const {}

////////////////////////////////////////////////////////////////////////////////
// Writer

OtlpGrpcPublisher::Writer::Writer(size_t thread_num, std::string const &endpoint)
    : thread_num_(thread_num), endpoint_(endpoint), publisher_(otlp_publisher_new(::rust::Str(endpoint_)))
{}

OtlpGrpcPublisher::Writer::~Writer() {}

void OtlpGrpcPublisher::Writer::flush()
{
  publisher_->flush();
}

u64 OtlpGrpcPublisher::Writer::bytes_written() const
{
  auto s = publisher_->stats();
  return s.bytes_sent - s.bytes_failed;
}

u64 OtlpGrpcPublisher::Writer::bytes_failed_to_write() const
{
  auto s = publisher_->stats();
  return s.bytes_failed;
}

void OtlpGrpcPublisher::Writer::write_internal_stats(
    InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const
{
  auto s = publisher_->stats();

  OtlpGrpcStats stats;
  stats.labels.shard = std::to_string(shard);
  stats.labels.module = module;

  // Report as metrics client
  stats.labels.client_type = "metrics";
  stats.metrics.bytes_failed = s.bytes_failed;
  stats.metrics.bytes_sent = s.bytes_sent;
  stats.metrics.metrics_failed = s.data_points_failed;
  stats.metrics.metrics_sent = s.data_points_sent;
  stats.metrics.requests_failed = s.requests_failed;
  stats.metrics.requests_sent = s.requests_sent;
  stats.metrics.unknown_response_tags = s.unknown_response_tags;
  encoder.write_internal_stats(stats, time_ns);

  // Report as logs client (mirror metrics for now)
  stats.labels.client_type = "logs";
  encoder.write_internal_stats(stats, time_ns);
}

void OtlpGrpcPublisher::Writer::write_internal_stats_to_logging_core(
    ::ebpf_net::aggregation::auto_handles::agg_core_stats &agg_core_stats,
    u64 time_ns,
    int shard,
    std::string_view module) const
{
  auto s = publisher_->stats();

  agg_core_stats.agg_otlp_grpc_stats(
      jb_blob(module),
      shard,
      jb_blob(std::string_view("metrics")),
      s.bytes_failed,
      s.bytes_sent,
      s.data_points_failed,
      s.data_points_sent,
      s.requests_failed,
      s.requests_sent,
      s.unknown_response_tags,
      time_ns);

  agg_core_stats.agg_otlp_grpc_stats(
      jb_blob(module),
      shard,
      jb_blob(std::string_view("logs")),
      s.bytes_failed,
      s.bytes_sent,
      s.data_points_failed,
      s.data_points_sent,
      s.requests_failed,
      s.requests_sent,
      s.unknown_response_tags,
      time_ns);
}

} // namespace reducer
