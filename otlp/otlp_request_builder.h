/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <map>
#include <string>
#include <string_view>
#include <variant>

#include <platform/types.h>

#include "otlp_grpc_metrics_client.h"

namespace otlp_client {

// This class builds up an ExportMetricsServiceRequest, ready for sending out along
// a grpc client.
//
// The intended usage is to construct the object, then:
//   assign a metric via metric()
//   determine that metric type via sum()
//   then add as many data points for that metric as necessary through number_data_point()
//   repeat for each metric you need to build.
//
// Example:
//  ExportServiceRequest req =
//    OtlpRequestBuilder()
//    .metric("test_metric1")
//      .sum()
//      .number_data_point(42u, label_map, timestamp)
//      .number_data_point(41.9999, label_map, timestamp)
//    .metric("test_metric2")
//      .sum()
//      ... etc.
class OtlpRequestBuilder {
public:
  using value_t = std::variant<u32, u64, double>;
  using labels_t = std::map<std::string, std::string>;
  using timestamp_t = std::chrono::nanoseconds;

  OtlpRequestBuilder();

  OtlpRequestBuilder(const OtlpRequestBuilder &) = delete;
  OtlpRequestBuilder &operator=(const OtlpRequestBuilder &) = delete;

  OtlpRequestBuilder(OtlpRequestBuilder &&) = delete;
  OtlpRequestBuilder &operator=(const OtlpRequestBuilder &&) = delete;

  OtlpRequestBuilder &metric(std::string_view metric_name);

  OtlpRequestBuilder &
  sum(opentelemetry::proto::metrics::v1::AggregationTemporality temporality =
          opentelemetry::proto::metrics::v1::AggregationTemporality::AGGREGATION_TEMPORALITY_DELTA,
      bool is_monotonic = true);
  // FUTURE: gauge, histogram, etc. as necessary

  OtlpRequestBuilder &number_data_point(value_t value, labels_t labels, timestamp_t timestamp_ns);

  operator ExportMetricsServiceRequest() const { return std::move(request_); }

private:
  ExportMetricsServiceRequest request_;
  opentelemetry::proto::metrics::v1::ResourceMetrics *resource_metrics_;
  opentelemetry::proto::metrics::v1::ScopeMetrics *scope_metrics_;

  opentelemetry::proto::metrics::v1::Metric *current_metric_;
  opentelemetry::proto::metrics::v1::Metric::DataCase current_metric_type_;
  opentelemetry::proto::metrics::v1::Sum *current_sum_;

  void add_labels(opentelemetry::proto::metrics::v1::NumberDataPoint &data_point, const labels_t &labels);
  void set_timestamp(opentelemetry::proto::metrics::v1::NumberDataPoint &data_point, timestamp_t timestamp_ns);
  void add_metric();
};
} // namespace otlp_client