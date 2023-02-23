/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.pb.h>
#include <opentelemetry/proto/common/v1/common.pb.h>
#include <opentelemetry/proto/metrics/v1/metrics.pb.h>

class MetricsServiceImpl final : public opentelemetry::proto::collector::metrics::v1::MetricsService::Service {
public:
  using Sinks = std::vector<std::ostream *>;

  explicit MetricsServiceImpl(Sinks sinks);

  grpc::Status Export(
      grpc::ServerContext *context,
      const opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest *request,
      opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse *response) override;

private:
  Sinks sinks_;

  void write_resource_metrics(const opentelemetry::proto::metrics::v1::ResourceMetrics &rm);
  void write_scope_metrics(const opentelemetry::proto::metrics::v1::ScopeMetrics &sm);
  void write_metric(const opentelemetry::proto::metrics::v1::Metric &m);
  void write_sum(const std::string &name, const opentelemetry::proto::metrics::v1::Sum &sum);
  void write_gauge(const std::string &name, const opentelemetry::proto::metrics::v1::Gauge &gauge);
  void write_number_data_point(const std::string &name, const opentelemetry::proto::metrics::v1::NumberDataPoint &dp);
  void write_key_value(const opentelemetry::proto::common::v1::KeyValue &v);
  void write_any_value(const opentelemetry::proto::common::v1::AnyValue &v);

  template <typename T> void write(T out)
  {
    for (auto sink : sinks_) {
      (*sink) << out;
    }
  }

  void flush();
};
