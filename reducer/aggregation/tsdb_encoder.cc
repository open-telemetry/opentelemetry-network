// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "labels.h"
#include "tsdb_encoder.h"

#include <generated/ebpf_net/aggregation/index.h>

#include <util/log.h>
#include <util/time.h>

#include <spdlog/fmt/fmt.h>

namespace reducer::aggregation {

TsdbEncoder::TsdbEncoder(
    std::vector<Publisher::WriterPtr> &metric_writers,
    TsdbFormat tsdb_format,
    Publisher::WriterPtr &otlp_metric_writer,
    std::chrono::nanoseconds timestamp,
    bool id_id_enabled,
    const DisabledMetrics &disabled_metrics,
    std::optional<int> rollup_count)
    : metric_writers_(metric_writers),
      tsdb_format_(tsdb_format),
      otlp_metric_writer_(otlp_metric_writer),
      timestamp_(timestamp),
      id_id_enabled_(id_id_enabled),
      disabled_metrics_(disabled_metrics)
{
  if (!metric_writers.empty()) {
    switch (tsdb_format_) {
    case TsdbFormat::prometheus:
    case TsdbFormat::json:
      prometheus_formatter_ = TsdbFormatter::make(tsdb_format_);
      break;
    case TsdbFormat::otlp_grpc:
      throw std::invalid_argument("invalid format, otlp_grpc, for prometheus formatter");
      break;
    }
    prometheus_formatter_->set_rollup(rollup_count);
    prometheus_formatter_->set_timestamp(timestamp);
  }

  if (otlp_metric_writer) {
    otlp_grpc_formatter_ = TsdbFormatter::make(TsdbFormat::otlp_grpc, otlp_metric_writer);
    otlp_grpc_formatter_->set_rollup(rollup_count);
    otlp_grpc_formatter_->set_timestamp(timestamp);
  }
}

void TsdbEncoder::encode_and_write_p_latencies(const PercentileLatencies &plat)
{
  if (metric_writers_.empty()) {
    // Prometheus style metrics exporting is disabled.
    return;
  }
  prometheus_formatter_->set_aggregation(plat.aggregation_name());

  encode_and_write_p_latencies("tcp", plat.tcp());
  encode_and_write_p_latencies("dns", plat.dns());
  encode_and_write_p_latencies("http", plat.http());
}

void TsdbEncoder::encode_and_write_p_latencies(const std::string &proto, const PercentileLatencies::LatencyAccumulator &accum)
{
  const std::string metric_name_p90 = proto + "_latency_p90";
  const std::string metric_name_p95 = proto + "_latency_p95";
  const std::string metric_name_p99 = proto + "_latency_p99";
  const std::string metric_name_max = proto + "_latency_max";

  // TODO: to evenly distribute output to all available writers, we should choose a writer
  //       based on the labels' hash value.
  auto &metric_writer = metric_writers_[0];

  for (const auto &l : accum.get_p_latencies()) {
    prometheus_formatter_->set_labels(l.key);
    prometheus_formatter_->write(metric_name_p90, l.p90, metric_writer);
    prometheus_formatter_->write(metric_name_p95, l.p95, metric_writer);
    prometheus_formatter_->write(metric_name_p99, l.p99, metric_writer);
  }

  for (const auto &[key, max_latency] : accum.get_max_latencies()) {
    prometheus_formatter_->set_labels(key);
    prometheus_formatter_->write(metric_name_max, max_latency, metric_writer);
  }
}

void TsdbEncoder::flush()
{
  if (prometheus_formatter_) {
    prometheus_formatter_->flush();
  }
  if (otlp_grpc_formatter_) {
    otlp_grpc_formatter_->flush();
  }
}

/////////////////////////////////////////////////////////////
// Operators

// TCP
#define METRICS tcp_metrics
#define A_B_UPDATE tcp_a_to_b_update
#define B_A_UPDATE tcp_b_to_a_update
#include "tsdb_encoder.inl"
#undef METRICS
#undef A_B_UPDATE
#undef B_A_UPDATE

// UDP
#define METRICS udp_metrics
#define A_B_UPDATE udp_a_to_b_update
#define B_A_UPDATE udp_b_to_a_update
#include "tsdb_encoder.inl"
#undef METRICS
#undef A_B_UPDATE
#undef B_A_UPDATE

// HTTP
#define METRICS http_metrics
#define A_B_UPDATE http_a_to_b_update
#define B_A_UPDATE http_b_to_a_update
#include "tsdb_encoder.inl"
#undef METRICS
#undef A_B_UPDATE
#undef B_A_UPDATE

// DNS
#define METRICS dns_metrics
#define A_B_UPDATE dns_a_to_b_update
#define B_A_UPDATE dns_b_to_a_update
#include "tsdb_encoder.inl"
#undef METRICS
#undef A_B_UPDATE
#undef B_A_UPDATE

} // namespace reducer::aggregation
