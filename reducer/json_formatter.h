/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "tsdb_formatter.h"

namespace reducer {
// Formatter implementation for JSON time-series format.
//
// Output:
// - prefix: `{"aggregation":"<aggregation>][_<rollup>]","<metric>":<value>,`
// - labels: `"label1"="xxx","label2"="yyy",...,`
// - suffix: `"timestamp":"<timestamp>"}`
//
class JsonTsdbFormatter : public TsdbFormatter {
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
  // Large enough for metric name, aggregation, rollup and value.
  char prefix_buf_[512];
  // Large enough for all labels.
  char labels_buf_[8192];
  // Large enough for ISO timestamp and then some.
  char suffix_buf_[64];

  // Cached labels string, points to labels_buf_ when initialized.
  std::string_view labels_;
  // Cached suffix string, points to suffix_buf_ when initialized.
  std::string_view suffix_;
  // Cached aggregation label, based on aggregation and rollup.
  std::string aggregation_label_;
};

} // namespace reducer
