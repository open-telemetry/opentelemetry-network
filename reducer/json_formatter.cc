// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "json_formatter.h"

#include <util/code_timing.h>
#include <util/time.h>

#include <spdlog/fmt/fmt.h>

#include <ctime>
#include <stdexcept>

namespace reducer {
namespace {

template <typename T>
std::string_view
json_format_prefix(char *buf_ptr, size_t buf_size, std::string_view metric, std::string_view aggregation_label, T value)
{
  auto [end, len] =
      fmt::format_to_n(buf_ptr, buf_size, "{{\"aggregation\":\"{}\",\"{}\":{},", aggregation_label, metric, value);
  return std::string_view(buf_ptr, std::min(len, buf_size));
}

std::string_view json_format_labels(char *buff_ptr, size_t buff_size, TsdbFormatter::labels_t const &labels)
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

  auto write_label = [write_str](std::string_view name, std::string_view value) {
    if (value.empty()) {
      return;
    }
    write_str("\"");
    write_str(name);
    write_str("\":\"");
    write_str(value);
    write_str("\",");
  };

  for (auto const &[name, value] : labels) {
    write_label(name, value);
  }

  return std::string_view(buff_ptr, written);
}

std::string_view json_format_suffix(char *buf_ptr, size_t buf_size, TsdbFormatter::timestamp_t timestamp)
{
  using namespace std::chrono;

  time_t time = system_clock::to_time_t(time_point<system_clock>(timestamp));

  struct tm tm_buf;
  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%FT%T", gmtime_r(&time, &tm_buf));

  auto millis = integer_time<milliseconds>(timestamp) - integer_time<seconds>(timestamp) * 1000;

  auto [end, len] = fmt::format_to_n(buf_ptr, buf_size, "\"timestamp\":\"{}.{:03}Z\"}}\n", strftime_buf, millis);
  return std::string_view(buf_ptr, std::min(len, buf_size));
}

} // namespace

void JsonTsdbFormatter::format(
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
  SCOPED_TIMING(JsonTsdbFormatterFormat);

  auto prefix = std::visit(
      [&](auto &&val) -> std::string_view {
        return json_format_prefix(prefix_buf_, sizeof(prefix_buf_), metric.name, aggregation_label_, val);
      },
      value);

  if (labels_changed || labels_.empty()) {
    labels_ = json_format_labels(labels_buf_, sizeof(labels_buf_), labels);
  }

  if (timestamp_changed || suffix_.empty()) {
    suffix_ = json_format_suffix(suffix_buf_, sizeof(suffix_buf_), timestamp);
  }

  SCOPED_TIMING(JsonTsdbFormatterFormatWriterWrite);
  writer->write(prefix, labels_, suffix_);
}

} // namespace reducer
