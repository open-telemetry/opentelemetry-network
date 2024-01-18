/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "otlp_grpc_publisher.h"
#include "tsdb_formatter.h"

extern int global_otlp_grpc_batch_size;

namespace reducer {

// Formatter implementation for OTLP gRPC format.
//
// Creates and formats an OTLP gRPC request and passes it to the writer.
//
class OtlpGrpcFormatter : public TsdbFormatter {
public:
  // Enables populating the metric description field in the `v1::Metric` messages.
  static void set_metric_description_field_enabled(bool enabled);
  // Returns whether populating the metric description field is enabled.
  static bool metric_description_field_enabled();

  OtlpGrpcFormatter(Publisher::WriterPtr const &writer);
  virtual ~OtlpGrpcFormatter() override;

  // flush any logs and metrics remaining in logs_request_ and metrics_request_
  virtual void flush() override;

protected:
  // Flag indicating whether populating the metric description field is enabled.
  static bool metric_description_field_enabled_;

  void format(
      MetricInfo const &metric_info,
      value_t value,
      std::string_view aggregation,
      bool aggregation_changed,
      rollup_t rollup,
      bool rollup_changed,
      labels_t labels,
      bool labels_changed,
      timestamp_t timestamp,
      bool timestamp_changed,
      Publisher::WriterPtr const &unused_writer) override;

  // Format tcp_metrics as a flow log.
  void format_flow_log(
      ebpf_net::metrics::tcp_metrics const &tcp_metrics,
      labels_t labels,
      bool labels_changed,
      timestamp_t timestamp,
      bool timestamp_changed) override;

  void send_logs_request();
  void send_metrics_request();

  // A single ExportLogsServiceRequest is used to send logs, batching multiple logs per request, and it is reused to
  // avoid regenerating common portions.
  ExportLogsServiceRequest logs_request_;
  opentelemetry::proto::logs::v1::ScopeLogs *scope_logs_;

  // A single ExportMetricsServiceRequest is used to send metrics, batching multiple metrics per request, and it is reused to
  // avoid regenerating common portions.
  ExportMetricsServiceRequest metrics_request_;
  opentelemetry::proto::metrics::v1::Sum sum_;
  opentelemetry::proto::metrics::v1::Gauge gauge_;
  opentelemetry::proto::metrics::v1::NumberDataPoint data_point_;
  opentelemetry::proto::metrics::v1::ScopeMetrics *scope_metrics_;

  OtlpGrpcPublisher::WriterPtr const &writer_;
};

} // namespace reducer
