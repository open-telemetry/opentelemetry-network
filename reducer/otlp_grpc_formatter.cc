// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "otlp_grpc_formatter.h"

#include <common/constants.h>
#include <otlp/otlp_grpc_metrics_client.h>
#include <otlp/otlp_util.h>
#include <util/code_timing.h>
#if !NDEBUG
#include <util/error_handling.h>
#endif
#include <util/json.h>
#include <util/overloaded_visitor.h>
#include <util/time.h>

#include <spdlog/fmt/fmt.h>

#include <ctime>
#include <stdexcept>

int global_otlp_grpc_batch_size = 1000;

namespace reducer {

bool OtlpGrpcFormatter::metric_description_field_enabled_ = false;

void OtlpGrpcFormatter::enable_metric_description_field()
{
  metric_description_field_enabled_ = true;
}

bool OtlpGrpcFormatter::metric_description_field_enabled()
{
  return metric_description_field_enabled_;
}

OtlpGrpcFormatter::OtlpGrpcFormatter(Publisher::WriterPtr const &writer) : writer_(writer)
{
  auto resource_metrics = request_.add_resource_metrics();
  scope_metrics_ = resource_metrics->add_scope_metrics();

  sum_.set_aggregation_temporality(opentelemetry::proto::metrics::v1::AggregationTemporality::AGGREGATION_TEMPORALITY_DELTA);
  sum_.set_is_monotonic(true);
  sum_.add_data_points();
}

OtlpGrpcFormatter::~OtlpGrpcFormatter()
{
  flush();
}

void OtlpGrpcFormatter::format(
    MetricInfo const &metric_info,
    value_t metric_value,
    std::string_view aggregation,
    bool aggregation_changed,
    rollup_t rollup,
    bool rollup_changed,
    labels_t labels,
    bool labels_changed,
    timestamp_t timestamp,
    bool timestamp_changed,
    Publisher::WriterPtr const &unused_writer)
{
  START_TIMING(OtlpGrpcFormatterFormat);
  opentelemetry::proto::metrics::v1::Metric metric;

#if !NDEBUG
  // Determine if string contains anything besides ASCII printable characters
  auto is_non_ascii = [](const std::string &str) {
    return std::any_of(str.begin(), str.end(), [](char ch) {
      auto uch = static_cast<unsigned char>(ch);
      return uch < 32 || uch > 127;
    });
  };

  auto print_ascii = [&](const std::string &str) { return is_non_ascii(str) ? "<ERROR_NON_ASCII>" : str; };

  ASSUME(!metric_info.name.empty()).else_log("empty metric name");
  bool found_non_ascii = false;
  found_non_ascii |= is_non_ascii(metric_info.name);
  found_non_ascii |= is_non_ascii(metric_info.unit);
  found_non_ascii |= is_non_ascii(metric_info.description);
  for (auto const &[key, value] : labels) {
    if (key.empty()) {
      throw std::invalid_argument(fmt::format("empty label key for metric={}", metric_info.name));
    }
    found_non_ascii |= is_non_ascii(key);
    ASSUME(!(key != "span" && key != "error" && value.empty()))
        .else_log("empty label value for metric={} label={}", metric_info.name, key);
    found_non_ascii |= is_non_ascii(value);
  }

  if (found_non_ascii) {
    std::string metric_string = fmt::format("name=\"{}\"", print_ascii(metric_info.name));
    if (!metric_info.unit.empty()) {
      metric_string += fmt::format(" unit=\"{}\"", print_ascii(metric_info.unit));
    }
    if (!metric_info.description.empty()) {
      metric_string += fmt::format(" description=\"{}\"", print_ascii(metric_info.description));
    }
    if (!labels.empty()) {
      metric_string += " labels:{";
      bool first_label = true;
      for (auto const &[key, value] : labels) {
        if (!first_label) {
          metric_string += ",";
        }
        metric_string += fmt::format(" key=\"{}\",value=\"{}\"", print_ascii(key), print_ascii(value));
        first_label = false;
      }
      metric_string += " }";
    }
    ASSUME(!found_non_ascii).else_log("OtlpGrpcFormatter detected non-ascii character(s) in metric: {}", metric_string);
  }
#endif

  metric.set_name(metric_info.name.data(), metric_info.name.size());
  metric.set_unit(metric_info.unit.data(), metric_info.unit.size());
  if (metric_description_field_enabled()) {
    metric.set_description(metric_info.description.data(), metric_info.description.size());
  }

  auto data_point = sum_.mutable_data_points(0);

  if (labels_changed) {
    SCOPED_TIMING(OtlpGrpcFormatterFormatLabelsChanged);
    data_point->clear_attributes();
    for (auto const &[key, value] : labels) {
      auto attribute = data_point->add_attributes();
      attribute->set_key(key.data(), key.size());
      attribute->mutable_value()->set_string_value(value.data(), value.size());
    }
  }

  if (timestamp_changed) {
    data_point->set_time_unix_nano(integer_time<std::chrono::nanoseconds>(timestamp));
  }

  std::visit(
      overloaded_visitor{
          [&](auto val) { data_point->set_as_int(val); },
          [&](double val) { data_point->set_as_double(val); },
      },
      metric_value);

  *metric.mutable_sum() = sum_;

  *scope_metrics_->add_metrics() = std::move(metric);
  STOP_TIMING(OtlpGrpcFormatterFormat);

  if (scope_metrics_->metrics_size() >= global_otlp_grpc_batch_size) {
    send_request();
  }
}

void OtlpGrpcFormatter::flush()
{
  SCOPED_TIMING(OtlpGrpcFormatterFlush);
  if (scope_metrics_->metrics_size()) {
    send_request();
  }
}

void OtlpGrpcFormatter::send_request()
{
  SCOPED_TIMING(OtlpGrpcFormatterSendRequest);

#define DEBUG_OTLP_JSON_PRINT 0
#if DEBUG_OTLP_JSON_PRINT
  LOG::trace("JSON view of ExportMetricsServiceRequest being sent: {}", log_waive(otlp_client::get_request_json(request_)));
#endif

  writer_->write(request_);

  // clear the metrics portion of request_, leaving the common portions to reuse
  scope_metrics_->clear_metrics();
}

} // namespace reducer
