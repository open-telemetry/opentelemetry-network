/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "metric_info.h"
#include "publisher.h"
#include "tsdb_format.h"

#include <platform/types.h>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

namespace reducer {

// Formats time-series entries that are to be sent to a TSDB.
//
// Formatted time-series entries are produced by calling the `write` method.
//
// Before calling `write`, time-series parameters are set using
// `set_aggregation`, `set_rollup`, `set_labels` and `set_timestamp`.
//
// Aggregation is a string like `az_az`, `az_role`, `role_role`, etc.
// Rollup is the number of seconds that the time-series is rolled-up to.
//
// This interface reflects the usage pattern where a number of similar
// time-series are formatted in succession, each one differing from others only
// in the metric name and value, while having the same set of labels, same
// timestamp, aggregation name and rollup. The interface allows implementations
// to format and cache portions of the output that doesn't change between calls
// to `write`.
//
class TsdbFormatter {
  friend class OtlpGrpcFormatterTest;

public:
  // Creates an instance for the specified format.
  static std::unique_ptr<TsdbFormatter>
  make(TsdbFormat format, std::optional<std::reference_wrapper<Publisher::WriterPtr>> writer = std::nullopt);

  using value_t = std::variant<u32, u64, double>;
  using rollup_t = std::optional<int>;
  using labels_t = std::map<std::string, std::string>;
  using timestamp_t = std::chrono::nanoseconds;

  virtual ~TsdbFormatter() {}

  // Some formatters may buffer metrics that need to be flushed periodically and before being destructed.
  virtual void flush() {};

  // Assigns the aggregation name (e.g. az_az, role_az, etc.)
  void set_aggregation(std::string_view aggregation);
  // Assigns the rollup count (e.g. 30, 60, ...).
  void set_rollup(rollup_t rollup);
  // Assigns the timestamp of time-series entry.
  void set_timestamp(timestamp_t timestamp);

  // Assigns time-series labels (set of key/value pairs).
  void set_labels(labels_t labels);
  void set_labels(std::initializer_list<std::tuple<std::string_view, std::string_view>> labels);

  // Helper function to set labels from NodeLabels, FlowLabels objects.
  template <typename Labels> void set_labels(Labels const &labels)
  {
    labels_.clear();
    labels.foreach ([this](std::string_view name, std::string_view value) {
      if (!value.empty()) {
        assign_label(name, value);
      };
    });
  }

  // Assigns a specific label.
  void assign_label(std::string_view name, std::string_view value);
  void assign_label(std::string name, std::string value);

  // Removes a specific label.
  void remove_label(std::string_view name);

  // Clears the labels
  void clear_labels();

  // Writes the formatted entry using the provided publisher writer object.
  void write(std::string_view metric_name, value_t value, Publisher::WriterPtr const &writer);

  // Writes the formatted entry using the provided publisher writer object.
  void write(MetricInfo const &metric, value_t value, Publisher::WriterPtr const &writer);

  // Writes the formatted entry as a flow log.
  template <typename TMetrics> void write_flow_log(TMetrics const &metrics)
  {
    format_flow_log(metrics, labels_, labels_changed_, timestamp_, timestamp_changed_);

    aggregation_changed_ = false;
    rollup_changed_ = false;
    labels_changed_ = false;
    timestamp_changed_ = false;
  };

protected:
  // Subclasses implement this function to do the actual formatting.
  virtual void format(
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
      Publisher::WriterPtr const &writer) = 0;

  // Subclasses that support formatting metrics as flow logs implement this function to do the actual formatting.
  virtual void format_flow_log(
      ebpf_net::metrics::tcp_metrics const &tcp_metrics,
      labels_t labels,
      bool labels_changed,
      timestamp_t timestamp,
      bool timestamp_changed) {};

private:
  std::string aggregation_;
  bool aggregation_changed_{false};

  rollup_t rollup_;
  bool rollup_changed_{false};

  labels_t labels_;
  bool labels_changed_{false};

  timestamp_t timestamp_{0};
  bool timestamp_changed_{false};
};

} // namespace reducer
