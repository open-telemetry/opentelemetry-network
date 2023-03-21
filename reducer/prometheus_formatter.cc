// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "prometheus_formatter.h"

#include <util/code_timing.h>
#include <util/time.h>

#include <spdlog/fmt/fmt.h>

#include <ctime>
#include <stdexcept>

namespace reducer {
namespace {

std::string_view prom_format_labels(char *buff_ptr, size_t buff_size, TsdbFormatter::labels_t const &labels)
{
  size_t written = 0;
  auto write = [&written, buff_ptr, buff_size](void const *ptr, size_t len) {
    size_t const n = std::min(len, (buff_size - written));
    if (n > 0) {
      memcpy(buff_ptr + written, ptr, n);
      written += n;
    }
  };

  auto write_str = [write](std::string_view str) { write(str.data(), str.size()); };

  size_t num_labels = 0;
  auto write_label = [&num_labels, write_str](std::string_view name, std::string_view value) {
    if (num_labels++ > 0) {
      write_str(",");
    }
    write_str(name);
    write_str("=\"");
    write_str(value);
    write_str("\"");
  };

  write_str("{");

  for (auto const &[name, value] : labels) {
    std::string label_name_sanitized;
    std::transform(name.begin(), name.end(), std::back_inserter(label_name_sanitized), [](unsigned char c) -> unsigned char {
      return c == '.' ? '_' : c;
    });

    write_label(label_name_sanitized, value);
  }

  write_str("}");

  return std::string_view(buff_ptr, written);
}

template <typename T> std::string_view prom_format_suffix(char *buf_ptr, size_t buf_size, T value, std::string_view timestamp)
{
  auto [end, len] = fmt::format_to_n(buf_ptr, buf_size, " {} {}\n", value, timestamp);
  return std::string_view(buf_ptr, std::min(len, buf_size));
}

} // namespace

void PrometheusFormatter::format(
    MetricInfo const &metric,
    value_t value,
    std::string_view aggregation,
    bool aggregation_changed,
    rollup_t rollup,
    bool rollup_changed,
    labels_t labels,
    bool labels_changed,
    timestamp_t timestamp,
    bool timestamp_changed,
    Publisher::WriterPtr const &writer)
{
  START_TIMING(PrometheusFormatterFormat);
  if (timestamp_changed || timestamp_str_.empty()) {
    timestamp_str_ = std::to_string(integer_time<std::chrono::milliseconds>(timestamp));
  }

  if (aggregation_changed || rollup_changed || labels_changed || labels_.empty()) {
    labels_ = prom_format_labels(labels_buf_, sizeof(labels_buf_), labels);
  }

  auto suffix = std::visit(
      [&](auto &&val) -> std::string_view { return prom_format_suffix(suffix_buf_, sizeof(suffix_buf_), val, timestamp_str_); },
      value);

  std::string metric_name_sanitized;
  std::transform(
      metric.name.begin(), metric.name.end(), std::back_inserter(metric_name_sanitized), [](unsigned char c) -> unsigned char {
        return c == '.' ? '_' : c;
      });
  STOP_TIMING(PrometheusFormatterFormat);

  SCOPED_TIMING(PrometheusFormatterFormatWriterWrite);
  writer->write(metric_name_sanitized, labels_, suffix);
}

} // namespace reducer
