// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "disabled_metrics.h"

#include <algorithm>
#include <cctype>

#include <util/string_view.h>

namespace {

// helper to trim whitespace and enforce lowercase
std::string clean(std::string_view s)
{
  std::string ret(views::trim_ws(s));

  std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c) { return std::tolower(c); });

  return ret;
}

} // namespace

namespace reducer {

DisabledMetrics::DisabledMetrics(std::string_view disabled_metrics_arg, std::string_view enabled_metrics_arg)
{
  constexpr std::string_view NO_METRICS_DISABLED = "none";

  // All internal stats are disabled by default except collector_health.
  // Users can only use enable flags in order to turn internal stat(s).
  disabled_defaults();

  if (disabled_metrics_arg.compare(NO_METRICS_DISABLED) == 0) {
    // nothing is disabled
    return;
  }

  size_t current = 0;
  size_t next = 0;

  while (current < disabled_metrics_arg.size()) {
    next = disabled_metrics_arg.find(",", current);

    if (next == std::string_view::npos) {
      next = disabled_metrics_arg.size();
    }

    std::string metric = clean(disabled_metrics_arg.substr(current, next - current));
    disable_metric(metric);

    current = next + 1;
  }

  current = 0;
  while (current < enabled_metrics_arg.size()) {
    next = enabled_metrics_arg.find(",", current);

    if (next == std::string_view::npos) {
      next = enabled_metrics_arg.size();
    }

    std::string metric = clean(enabled_metrics_arg.substr(current, next - current));
    enable_metric(metric);

    current = next + 1;
  }
}

} // namespace reducer
