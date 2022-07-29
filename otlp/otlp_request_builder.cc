// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "otlp_request_builder.h"

#include <util/overloaded_visitor.h>
#include <util/time.h>

#include <cassert>

namespace otlp_client {
OtlpRequestBuilder::OtlpRequestBuilder()
    : resource_metrics_(request_.add_resource_metrics()),
      scope_metrics_(resource_metrics_->add_scope_metrics()),
      current_metric_(nullptr),
      current_sum_(nullptr)
{}

OtlpRequestBuilder &OtlpRequestBuilder::metric(std::string_view metric_name)
{
  current_metric_ = scope_metrics_->add_metrics();
  current_metric_->set_name(metric_name.data(), metric_name.size());
  current_sum_ = nullptr;
  return *this;
}

OtlpRequestBuilder &
OtlpRequestBuilder::sum(opentelemetry::proto::metrics::v1::AggregationTemporality temporality, bool is_monotonic)
{
  if (current_metric_ == nullptr) {
    // This is a programming error.  You must first allocate a metric via metric()
    assert(false);
    return *this;
  }

  opentelemetry::proto::metrics::v1::Sum sum;
  sum.set_aggregation_temporality(temporality);
  sum.set_is_monotonic(is_monotonic);
  *current_metric_->mutable_sum() = std::move(sum);
  current_metric_type_ = opentelemetry::proto::metrics::v1::Metric::DataCase::kSum;

  return *this;
}
OtlpRequestBuilder &OtlpRequestBuilder::number_data_point(value_t value, labels_t labels, timestamp_t timestamp_ns)
{
  if (current_metric_ == nullptr) {
    // This is a programming error.  You must first allocate a metric via metric()
    assert(false);
    return *this;
  }

  opentelemetry::proto::metrics::v1::NumberDataPoint data_point;
  add_labels(data_point, labels);
  set_timestamp(data_point, timestamp_ns);

  std::visit(
      overloaded_visitor{
          [&](auto val) { data_point.set_as_int(val); },
          [&](double val) { data_point.set_as_double(val); },
      },
      value);

  switch (current_metric_->data_case()) {
  case opentelemetry::proto::metrics::v1::Metric::DataCase::kSum: {
    auto *sum = current_metric_->mutable_sum();
    if (sum == nullptr) {
      // This is a programming error.  You must first allocate a sum via sum()
      assert(false);
      return *this;
    }

    *sum->add_data_points() = std::move(data_point);
  } break;

    // FUTURE: case gauge

  default:
    // Programming error:
    // perhaps an unsupported type for number_data_point?
    assert(false);
    break;
  }

  return *this;
}

void OtlpRequestBuilder::add_labels(opentelemetry::proto::metrics::v1::NumberDataPoint &data_point, const labels_t &labels)
{
  for (auto const &[key, value] : labels) {
    auto attribute = data_point.add_attributes();
    attribute->set_key(key.data(), key.size());
    attribute->mutable_value()->set_string_value(value.data(), value.size());
  }
}

void OtlpRequestBuilder::set_timestamp(opentelemetry::proto::metrics::v1::NumberDataPoint &data_point, timestamp_t timestamp_ns)
{
  data_point.set_time_unix_nano(integer_time<std::chrono::nanoseconds>(timestamp_ns));
}

} // namespace otlp_client