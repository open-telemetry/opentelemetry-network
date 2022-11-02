/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "constants.h"
#include "disabled_metrics.h"
#include "metric_info.h"
#include "publisher.h"
#include "stat_info.h"
#include "tsdb_formatter.h"

namespace reducer {
class InternalMetricsEncoder {
public:
  InternalMetricsEncoder(TsdbFormat format, Publisher::WriterPtr &writer, const DisabledMetrics &disabled_metrics)
      : writer_(writer), disabled_metrics_(disabled_metrics)
  {
    formatter_ = TsdbFormatter::make(format, writer);
  }

  void write_metric(
      const EbpfNetMetricInfo &metric,
      TsdbFormatter::labels_t labels,
      TsdbFormatter::value_t value,
      TsdbFormatter::timestamp_t time_ns)
  {
    formatter_->set_labels(labels);
    formatter_->assign_label(std::string_view(kProductIdDimName), std::string_view(kProductIdDimValue));
    formatter_->set_timestamp(time_ns);
    formatter_->write(metric, value, writer_);
  }

  void write_metric(const EbpfNetMetricInfo &metric, TsdbFormatter::value_t value, TsdbFormatter::timestamp_t time_ns)
  {
    formatter_->assign_label(std::string_view(kProductIdDimName), std::string_view(kProductIdDimValue));
    formatter_->set_timestamp(time_ns);
    formatter_->write(metric, value, writer_);
  }

  void add_label(std::string name, std::string value) { formatter_->assign_label(name, value); }

  void remove_label(std::string_view name) { formatter_->remove_label(name); }

  void clear_labels() { formatter_->clear_labels(); }

  void flush() { formatter_->flush(); }

  template <typename STATS> void write_internal_stats(STATS &stats, std::chrono::nanoseconds time_ns)
  {
    clear_labels();

    stats.labels.foreach_label([&](std::string_view name, std::string value) { add_label(std::string(name), value); });

    stats.metrics.foreach_metric([&](EbpfNetMetricInfo metric, std::variant<u32, u64, double> value) {
      if (!disabled_metrics_.is_metric_disabled(metric.metric)) {
        write_metric(metric, value, time_ns);
      }
    });
  }

  template <typename STATS> void write_internal_stats(STATS &stats, u64 time_ns)
  {
    write_internal_stats<STATS>(stats, std::chrono::nanoseconds(time_ns));
  }

private:
  Publisher::WriterPtr &writer_;
  std::unique_ptr<TsdbFormatter> formatter_;
  const DisabledMetrics &disabled_metrics_;
};

} // namespace reducer
