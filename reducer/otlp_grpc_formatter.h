/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "otlp_grpc_publisher.h"
#include "tsdb_formatter.h"

#include <otlp_export_cxxbridge.h>

// Retained for config compatibility; not used by Rust exporter.
extern int global_otlp_grpc_batch_size;

namespace reducer {

// TsdbFormatter that calls into Rust OTLP exporter via cxx bridge
class OtlpGrpcFormatter : public TsdbFormatter {
public:
  static void set_metric_description_field_enabled(bool enabled);
  static bool metric_description_field_enabled();

  explicit OtlpGrpcFormatter(Publisher::WriterPtr const &writer);
  virtual ~OtlpGrpcFormatter() override;

  virtual void flush() override;

protected:
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

  void format_flow_log(
      ebpf_net::metrics::tcp_metrics const &tcp_metrics,
      labels_t labels,
      bool labels_changed,
      timestamp_t timestamp,
      bool timestamp_changed) override;

private:
  // Cached labels to avoid rebuilding when unchanged
  ::rust::Vec<::Label> labels_cache_;

  // Reference to the concrete writer that owns the Rust publisher
  OtlpGrpcPublisher::Writer *writer_;

  static bool metric_description_field_enabled_;

  void rebuild_labels(labels_t const &labels);
};

} // namespace reducer
