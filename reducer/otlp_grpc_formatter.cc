// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "otlp_grpc_formatter.h"

#include <common/constants.h>
#include <otlp/otlp_util.h>
#include <util/code_timing.h>
#if !NDEBUG
#include <util/error_handling.h>
#endif
#include <util/json.h>
#include <util/overloaded_visitor.h>
#include <util/time.h>

#include <spdlog/fmt/fmt.h>

#include <generated/ebpf_net/metrics.h>

#include <ctime>
#include <stdexcept>

int global_otlp_grpc_batch_size = 1000;

namespace reducer {

bool OtlpGrpcFormatter::metric_description_field_enabled_ = false;

void OtlpGrpcFormatter::set_metric_description_field_enabled(bool enabled)
{
  metric_description_field_enabled_ = enabled;
}

bool OtlpGrpcFormatter::metric_description_field_enabled()
{
  return metric_description_field_enabled_;
}

OtlpGrpcFormatter::OtlpGrpcFormatter(Publisher::WriterPtr const &writer) : writer_(writer)
{
  auto resource_logs = logs_request_.add_resource_logs();
  scope_logs_ = resource_logs->add_scope_logs();

  auto resource_metrics = metrics_request_.add_resource_metrics();
  scope_metrics_ = resource_metrics->add_scope_metrics();

  sum_.set_aggregation_temporality(opentelemetry::proto::metrics::v1::AggregationTemporality::AGGREGATION_TEMPORALITY_DELTA);
  sum_.set_is_monotonic(true);
  sum_.add_data_points();

  gauge_.add_data_points();
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
  START_TIMING(OtlpGrpcFormatterFormatMetric);
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

  DEBUG_ASSUME(!metric_info.name.empty()).else_log("empty metric name");
  bool found_non_ascii = false;
  found_non_ascii |= is_non_ascii(metric_info.name);
  found_non_ascii |= is_non_ascii(metric_info.unit);
  found_non_ascii |= is_non_ascii(metric_info.description);
  for (auto const &[key, value] : labels) {
    if (key.empty()) {
      throw std::invalid_argument(fmt::format("empty label key for metric={}", metric_info.name));
    }
    found_non_ascii |= is_non_ascii(key);
    DEBUG_ASSUME(!(key != "span" && key != "error" && value.empty()))
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
    DEBUG_ASSUME(!found_non_ascii).else_log("OtlpGrpcFormatter detected non-ascii character(s) in metric: {}", metric_string);
  }
#endif

  metric.set_name(metric_info.name.data(), metric_info.name.size());
  metric.set_unit(metric_info.unit.data(), metric_info.unit.size());
  if (metric_description_field_enabled()) {
    metric.set_description(metric_info.description.data(), metric_info.description.size());
  }

  if (labels_changed) {
    SCOPED_TIMING(OtlpGrpcFormatterFormatLabelsChanged);
    data_point_.clear_attributes();
    for (auto const &[key, value] : labels) {
      auto attribute = data_point_.add_attributes();
      attribute->set_key(key.data(), key.size());
      attribute->mutable_value()->set_string_value(value.data(), value.size());
    }
  }

  if (timestamp_changed) {
    // set the start time to the timestamp minus 30 seconds.
    data_point_.set_start_time_unix_nano(integer_time<std::chrono::nanoseconds>(timestamp) - int64_t(30000000000));
    data_point_.set_time_unix_nano(integer_time<std::chrono::nanoseconds>(timestamp));
  }

  std::visit(
      overloaded_visitor{
          [&](auto val) { data_point_.set_as_int(val); },
          [&](double val) { data_point_.set_as_double(val); },
      },
      metric_value);

  if (metric_info.type == MetricTypeSum) {
    *sum_.mutable_data_points(0) = data_point_;
    *metric.mutable_sum() = sum_;
  } else {
    *gauge_.mutable_data_points(0) = data_point_;
    *metric.mutable_gauge() = gauge_;
  }

  *scope_metrics_->add_metrics() = std::move(metric);
  STOP_TIMING(OtlpGrpcFormatterFormatMetric);

  if (scope_metrics_->metrics_size() >= global_otlp_grpc_batch_size) {
    send_metrics_request();
  }
}

void OtlpGrpcFormatter::format_flow_log(
    ebpf_net::metrics::tcp_metrics const &tcp_metrics,
    labels_t labels,
    bool labels_changed,
    timestamp_t timestamp,
    bool timestamp_changed)
{
  START_TIMING(OtlpGrpcFormatterFormatFlowLog);

  opentelemetry::proto::logs::v1::LogRecord log_record;
  log_record.set_time_unix_nano(integer_time<std::chrono::nanoseconds>(timestamp));

  log_record.set_severity_text("INFO");
  log_record.set_severity_number(opentelemetry::proto::logs::v1::SeverityNumber::SEVERITY_NUMBER_INFO);

  double sum_srtt = double(tcp_metrics.sum_srtt) / 8 / 1'000'000; // RTTs are measured in units of 1/8 microseconds.

  auto message = fmt::format(
      "{} {} {} {} {} {} {} {} {} {} {} {} {}",
      labels["source.ip"],
      labels["source.workload.name"],
      labels["dest.ip"],
      labels["dest.workload.name"],
      tcp_metrics.sum_bytes,
      tcp_metrics.active_rtts,
      tcp_metrics.active_sockets,
      tcp_metrics.active_rtts ? sum_srtt / tcp_metrics.active_rtts : 0.0,
      tcp_metrics.sum_delivered,
      tcp_metrics.sum_retrans,
      tcp_metrics.syn_timeouts,
      tcp_metrics.new_sockets,
      tcp_metrics.tcp_resets);
  log_record.mutable_body()->set_string_value(message.data(), message.size());

  *scope_logs_->add_log_records() = std::move(log_record);
  STOP_TIMING(OtlpGrpcFormatterFormatFlowLog);

  if (scope_logs_->log_records_size() >= global_otlp_grpc_batch_size) {
    send_logs_request();
  }
}

void OtlpGrpcFormatter::flush()
{
  {
    SCOPED_TIMING(OtlpGrpcFormatterFlushLogs);
    if (scope_logs_->log_records_size()) {
      send_logs_request();
    }
  }

  {
    SCOPED_TIMING(OtlpGrpcFormatterFlushMetrics);
    if (scope_metrics_->metrics_size()) {
      send_metrics_request();
    }
  }
}

#define DEBUG_OTLP_JSON_PRINT 0

void OtlpGrpcFormatter::send_logs_request()
{
  SCOPED_TIMING(OtlpGrpcFormatterSendLogsRequest);

#if DEBUG_OTLP_JSON_PRINT
  LOG::trace("JSON view of ExportLogsServiceRequest being sent: {}", log_waive(otlp_client::get_request_json(logs_request_)));
#endif

  writer_->write(logs_request_);

  // clear the logs portion of logs_request_, leaving the common portions to reuse
  scope_logs_->clear_log_records();
}

void OtlpGrpcFormatter::send_metrics_request()
{
  SCOPED_TIMING(OtlpGrpcFormatterSendMetricsRequest);

#if DEBUG_OTLP_JSON_PRINT
  LOG::trace(
      "JSON view of ExportMetricsServiceRequest being sent: {}", log_waive(otlp_client::get_request_json(metrics_request_)));
#endif

  writer_->write(metrics_request_);

  // clear the metrics portion of metrics_request_, leaving the common portions to reuse
  scope_metrics_->clear_metrics();
}

} // namespace reducer
