/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "percentile_latencies.h"

#include <reducer/constants.h>
#include <reducer/disabled_metrics.h>
#include <reducer/publisher.h>
#include <reducer/tsdb_formatter.h>
#include <reducer/write_metrics.h>

#include <generated/ebpf_net/aggregation/weak_refs.h>
#include <generated/ebpf_net/metrics.h>

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace reducer::aggregation {

class FlowLabels;

class TsdbEncoder {
public:
  TsdbEncoder(
      std::vector<Publisher::WriterPtr> &metric_writers,
      TsdbFormat tsdb_format,
      Publisher::WriterPtr &otlp_metric_writer,
      std::chrono::nanoseconds timestamp,
      bool id_id_enabled,
      bool az_id_enabled,
      bool flow_logs_enabled,
      const DisabledMetrics &disabled_metrics,
      std::optional<int> rollup_count = std::nullopt);

  void set_reverse(int reverse) { reverse_ = reverse; }

#define DECLARE_OPERATOR(SPAN, METRICS)                                                                                        \
  void operator()(u64 t, ::ebpf_net::aggregation::weak_refs::SPAN &SPAN, ::ebpf_net::metrics::METRICS &metrics, u64 interval);

#define DECLARE_OPERATORS(METRICS)                                                                                             \
  DECLARE_OPERATOR(agg_root, METRICS)                                                                                          \
  DECLARE_OPERATOR(node_node, METRICS)                                                                                         \
  DECLARE_OPERATOR(az_node, METRICS)                                                                                           \
  DECLARE_OPERATOR(az_az, METRICS)

  DECLARE_OPERATORS(tcp_metrics)
  DECLARE_OPERATORS(udp_metrics)
  DECLARE_OPERATORS(http_metrics)
  DECLARE_OPERATORS(dns_metrics)

#undef DECLARE_OPERATOR
#undef DECLARE_OPERATORS

  void encode_and_write_p_latencies(const PercentileLatencies &plat);

  // Flush formatter(s) which may have metrics buffered.
  void flush();

private:
  // Prometheus (scrape) style publisher writers
  std::vector<Publisher::WriterPtr> &metric_writers_; // empty vector if not enabled
  TsdbFormat tsdb_format_;                            // TsdbFormat of the Prometheus style publisher

  // OTLP (push) style publisher writer
  Publisher::WriterPtr &otlp_metric_writer_; // nullptr if not enabled

  std::chrono::nanoseconds timestamp_;
  bool id_id_enabled_{false};
  bool az_id_enabled_{false};
  bool flow_logs_enabled_{false};
  int reverse_{0};

  const DisabledMetrics &disabled_metrics_;

  // Prometheus style formatter (for TsdbFormat::prometheus and TsdbFormat::json)
  std::unique_ptr<TsdbFormatter> prometheus_formatter_;

  // OTLP gRPC formatter
  std::unique_ptr<TsdbFormatter> otlp_grpc_formatter_;

  // encode_and_write for exporting metrics with Prometheus (scrape) style publisher writers
  template <typename Metrics>
  void
  encode_and_write(Publisher::WriterPtr &writer, std::string_view aggregation, const FlowLabels &labels, Metrics const &metrics)
  {
    prometheus_formatter_->set_aggregation(aggregation);
    prometheus_formatter_->set_labels(labels);
    prometheus_formatter_->assign_label(std::string_view(kProductIdDimName), std::string_view(kProductIdDimValue));
    write_metrics(metrics, writer, *prometheus_formatter_, disabled_metrics_);
  }

  // encode_and_write for exporting metrics with OTLP (push) style publisher writers
  template <typename Metrics>
  void encode_and_write_otlp_grpc(
      Publisher::WriterPtr &writer, std::string_view aggregation, const FlowLabels &labels, Metrics const &metrics)
  {
    otlp_grpc_formatter_->set_aggregation(aggregation);
    otlp_grpc_formatter_->set_labels(labels);
    otlp_grpc_formatter_->assign_label(std::string_view(kProductIdDimName), std::string_view(kProductIdDimValue));
    write_metrics(metrics, writer, *otlp_grpc_formatter_, disabled_metrics_);
  }

  // encode_and_write for exporting metrics as flow logs with OTLP (push) style publisher writers
  template <typename Metrics> void encode_and_write_otlp_grpc_flow_log(const FlowLabels &labels, Metrics const &metrics)
  {
    otlp_grpc_formatter_->set_labels(labels);
    write_flow_log(metrics, *otlp_grpc_formatter_, disabled_metrics_);
  }

  void encode_and_write_p_latencies(const std::string &proto, const PercentileLatencies::LatencyAccumulator &accum);
};

} // namespace reducer::aggregation
