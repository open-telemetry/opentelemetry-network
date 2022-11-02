// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "tsdb_formatter.h"
#include "json_formatter.h"
#include "otlp_grpc_formatter.h"
#include "prometheus_formatter.h"

#include <reducer/constants.h>

#include <util/time.h>

#include <spdlog/fmt/fmt.h>

#include <ctime>
#include <stdexcept>

namespace reducer {

std::unique_ptr<TsdbFormatter>
TsdbFormatter::make(TsdbFormat format, std::optional<std::reference_wrapper<Publisher::WriterPtr>> writer)
{
  switch (format) {
  case TsdbFormat::prometheus:
    return std::make_unique<PrometheusFormatter>();
  case TsdbFormat::json:
    return std::make_unique<JsonTsdbFormatter>();
  case TsdbFormat::otlp_grpc:
    if (!writer) {
      throw std::invalid_argument("otlp_grpc format requires writer");
    }
    return std::make_unique<OtlpGrpcFormatter>(*writer);
  default:
    throw std::invalid_argument("unsupported format");
  }
}

void TsdbFormatter::set_aggregation(std::string_view aggregation)
{
  assign_label("aggregation", aggregation);
  aggregation_ = aggregation;
  aggregation_changed_ = true;
}

void TsdbFormatter::set_rollup(rollup_t rollup)
{
  rollup_ = rollup;
  rollup_changed_ = true;
}

void TsdbFormatter::set_timestamp(timestamp_t timestamp)
{
  timestamp_ = timestamp;
  timestamp_changed_ = true;
}

void TsdbFormatter::set_labels(labels_t labels)
{
  labels_ = std::move(labels);
  labels_changed_ = true;
}

void TsdbFormatter::set_labels(std::initializer_list<std::tuple<std::string_view, std::string_view>> labels)
{
  labels_.clear();
  for (auto const &[name, value] : labels) {
    assign_label(name, value);
  }
  labels_changed_ = true;
}

void TsdbFormatter::assign_label(std::string_view name, std::string_view value)
{
  assign_label(std::string(name), std::string(value));
}

void TsdbFormatter::assign_label(std::string name, std::string value)
{
  labels_.insert_or_assign(name, value);
  labels_changed_ = true;
}

void TsdbFormatter::remove_label(std::string_view name)
{
  labels_.erase(std::string(name));
  labels_changed_ = true;
}

void TsdbFormatter::clear_labels()
{
  labels_.clear();
  labels_changed_ = true;
}

void TsdbFormatter::write(std::string_view metric_name, value_t value, Publisher::WriterPtr const &writer)
{
  write(MetricInfo{metric_name}, value, writer);
}

void TsdbFormatter::write(MetricInfo const &metric, value_t value, Publisher::WriterPtr const &writer)
{
  format(
      metric,
      value,
      aggregation_,
      aggregation_changed_,
      rollup_,
      rollup_changed_,
      labels_,
      labels_changed_,
      timestamp_,
      timestamp_changed_,
      writer);

  aggregation_changed_ = false;
  rollup_changed_ = false;
  labels_changed_ = false;
  timestamp_changed_ = false;
}

} // namespace reducer
