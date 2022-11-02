/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "tsdb_formatter.h"

namespace reducer {

// Formatter implementation for Prometheus time-series format.
//
// Output:
// - prefix: `<metric_name>`
// - labels: `[_<aggregation>][_<rollup>]{label1="xxx",label2="yyy",...}`
// - suffix: ` <value> <milliseconds>`
//
class PrometheusFormatter : public TsdbFormatter {
protected:
  void format(
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
      Publisher::WriterPtr const &writer) override;

private:
  // Large enough for all labels.
  char labels_buf_[8192];
  // Large enough for value and timestamp.
  char suffix_buf_[64];

  // ought to be plenty. e.g. #TYPE tcp_retrans_az_az_30 gauge
  char prefix_buf_[1024];

  // Cached labels string, points to labels_buf_ when initialized.
  std::string_view labels_;
  // Cached textual representation of timestamp.
  std::string timestamp_str_;
};

} // namespace reducer
