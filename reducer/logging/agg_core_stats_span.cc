/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "agg_core_stats_span.h"

#include "logging_core.h"

#include <reducer/internal_metrics_encoder.h>
#include <reducer/logging/component.h>

#include <reducer/internal_stats.h>
#include <reducer/tsdb_format.h>

#include <util/log.h>
#include <util/log_formatters.h>

namespace reducer::logging {

AggCoreStatsSpan::AggCoreStatsSpan() {}

AggCoreStatsSpan::~AggCoreStatsSpan() {}

void AggCoreStatsSpan::agg_root_truncation_stats(
    ::ebpf_net::logging::weak_refs::agg_core_stats span_ref, u64 timestamp, jsrv_logging__agg_root_truncation_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  AggRootTruncationStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.field = msg->field;
  stats.metrics.count = msg->count;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "AggCoreStatsSpan::agg_root_truncation_stats module={} shard={} field={}  count={} timestamp={}",
      msg->module,
      msg->shard,
      msg->field,
      msg->count,
      msg->time_ns);
}

void AggCoreStatsSpan::agg_prometheus_bytes_stats(
    ::ebpf_net::logging::weak_refs::agg_core_stats span_ref, u64 timestamp, jsrv_logging__agg_prometheus_bytes_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  AggPrometheusBytesStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.metrics.prometheus_bytes_written = msg->prometheus_bytes_written;
  stats.metrics.prometheus_bytes_discarded = msg->prometheus_bytes_discarded;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "AggCoreStatsSpan::agg_prometheus_bytes_stats module={} shard={} metrics_bytes_written={} metrics_bytes_discarded={} timestamp={}",
      msg->module,
      msg->shard,
      msg->prometheus_bytes_written,
      msg->prometheus_bytes_discarded,
      msg->time_ns);
}
} // namespace reducer::logging
