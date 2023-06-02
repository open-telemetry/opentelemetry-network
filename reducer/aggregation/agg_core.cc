// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "agg_core.h"
#include "tsdb_encoder.h"

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <reducer/rpc_queue_matrix.h>

#include <common/constants.h>

#include <generated/ebpf_net/aggregation/containers.h>
#include <generated/ebpf_net/aggregation/containers.inl>

#include <platform/userspace-time.h>
#include <util/code_timing.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/time.h>

#include <stdexcept>

namespace reducer::aggregation {

bool AggCore::node_ip_field_disabled_ = false;

void AggCore::set_node_ip_field_disabled(bool disabled)
{
  node_ip_field_disabled_ = disabled;
}

bool AggCore::id_id_enabled_ = false;
bool AggCore::az_id_enabled_ = false;
bool AggCore::flow_logs_enabled_ = false;

void AggCore::set_id_id_enabled(bool enabled)
{
  id_id_enabled_ = enabled;
}

void AggCore::set_az_id_enabled(bool enabled)
{
  az_id_enabled_ = enabled;
}

void AggCore::set_flow_logs_enabled(bool enabled)
{
  flow_logs_enabled_ = enabled;
}

AggCore::AggCore(
    RpcQueueMatrix &matching_to_aggregation_queues,
    RpcQueueMatrix &aggregation_to_logging_queues,
    std::unique_ptr<Publisher> &metrics_publisher,
    std::vector<Publisher::WriterPtr> metric_writers,
    std::unique_ptr<Publisher> &otlp_metrics_publisher,
    Publisher::WriterPtr otlp_metric_writer,
    bool enable_percentile_latencies,
    TsdbFormat metrics_tsdb_format,
    DisabledMetrics disabled_metrics,
    size_t shard_num,
    u64 initial_timestamp)
    : CoreBase(
          "aggregation",
          shard_num,
          initial_timestamp,
          aggregation_to_logging_queues.make_writers<ebpf_net::logging::Writer>(
              shard_num, std::bind(&Core::current_timestamp, this))),
      metrics_publisher_(metrics_publisher),
      metric_writers_(std::move(metric_writers)),
      otlp_metrics_publisher_(otlp_metrics_publisher),
      otlp_metric_writer_(std::move(otlp_metric_writer)),
      metrics_tsdb_format_(metrics_tsdb_format),
      matching_to_aggregation_stats_(shard_num, "matching", "aggregation"),
      aggregation_to_logging_stats_(shard_num, "aggregation", "logging", aggregation_to_logging_queues),
      disabled_metrics_(disabled_metrics),
      core_stats_(index_.core_stats.alloc()),
      agg_core_stats_(index_.agg_core_stats.alloc())
{
  add_rpc_clients(matching_to_aggregation_queues.make_readers(shard_num), ClientType::matching, matching_to_aggregation_stats_);

  if (enable_percentile_latencies)
    p_latencies_ = std::make_unique<PercentileLatencies>();
}

void AggCore::on_timeslot_complete()
{
  write_metrics();
  aggregation_to_logging_stats_.check_utilization();
  index_.send_pulse();
}

void AggCore::write_metrics()
{
  u64 t = current_timestamp();

  write_standard_metrics(t);

  for (auto &metric_writer : metric_writers_) {
    metric_writer->flush();
  }

  if (otlp_metric_writer_) {
    otlp_metric_writer_->flush();
  }
}

void AggCore::write_standard_metrics(u64 t)
{
  s16 relative_timeslot = index_.agg_root.tcp_a_to_b.relative_timeslot(t);

  if (relative_timeslot <= 0) {
    // Not ready.
    return;
  }

  SCOPED_TIMING(AggCoreWriteStandardMetrics);

  double slot_duration = index_.agg_root.tcp_a_to_b.slot_duration();
  // timestamp, in nanoseconds, within the metric slot
  u64 metric_timestamp = t - (u64)(relative_timeslot * slot_duration);
  // fraction of the slot remaining after the timestamp
  double frac = 1.0 - fmod(metric_timestamp / slot_duration, 1);
  // align to slot boundary
  metric_timestamp += (u64)(frac * slot_duration);

  TsdbEncoder encoder(
      metric_writers_,
      metrics_tsdb_format_,
      otlp_metric_writer_,
      std::chrono::nanoseconds(metric_timestamp),
      id_id_enabled_,
      az_id_enabled_,
      flow_logs_enabled_,
      disabled_metrics_);

  auto az_az_writer = [&encoder, this](auto &&...args) {
    encoder(args...);
    if (this->p_latencies_ != nullptr) {
      this->p_latencies_->operator()(args...);
    }
  };

#define WRITE_METRICS(A_B, B_A)                                                                                                \
  index_.agg_root.A_B##_foreach(t, encoder);                                                                                   \
  encoder.set_reverse(1);                                                                                                      \
  index_.agg_root.B_A##_foreach(t, encoder);                                                                                   \
  encoder.set_reverse(0);                                                                                                      \
                                                                                                                               \
  index_.node_node.A_B##_foreach(t, encoder);                                                                                  \
  encoder.set_reverse(1);                                                                                                      \
  index_.node_node.B_A##_foreach(t, encoder);                                                                                  \
  encoder.set_reverse(0);                                                                                                      \
                                                                                                                               \
  index_.az_node.A_B##_foreach(t, encoder);                                                                                    \
  encoder.set_reverse(1);                                                                                                      \
  index_.az_node.B_A##_foreach(t, encoder);                                                                                    \
  encoder.set_reverse(0);                                                                                                      \
                                                                                                                               \
  index_.az_az.A_B##_foreach(t, az_az_writer);

  WRITE_METRICS(tcp_a_to_b, tcp_b_to_a);
  WRITE_METRICS(udp_a_to_b, udp_b_to_a);
  WRITE_METRICS(http_a_to_b, http_b_to_a);
  WRITE_METRICS(dns_a_to_b, dns_b_to_a);
#undef WRITE_METRICS

  // write pXX latencies
  if (p_latencies_ != nullptr) {
    encoder.encode_and_write_p_latencies(*p_latencies_);
  }

  encoder.flush();
}

void AggCore::write_internal_stats()
{
  SCOPED_TIMING(AggCoreWriteInternalStats);

  u64 time_ns = fp_get_time_ns();
  int const shard = shard_num();
  std::string_view module = "aggregation";

  write_common_stats_to_logging_core(core_stats_, time_ns);

  stat_counters.foreach_field_truncation([&](std::string_view field, std::size_t count, std::size_t) {
    agg_core_stats_.agg_root_truncation_stats(jb_blob(module), shard, jb_blob(field), count, time_ns);
  });

  u64 prometheus_bytes_written = 0;
  u64 prometheus_bytes_discarded = 0;
  for (auto &metric_writer : metric_writers_) {
    prometheus_bytes_written += metric_writer->bytes_written();
    prometheus_bytes_discarded += metric_writer->bytes_failed_to_write();
  }

  agg_core_stats_.agg_prometheus_bytes_stats(
      jb_blob(module), shard, prometheus_bytes_written, prometheus_bytes_discarded, time_ns);

  matching_to_aggregation_stats_.write_internal_metrics_to_logging_core(core_stats_, time_ns);
  aggregation_to_logging_stats_.write_internal_metrics_to_logging_core(core_stats_, time_ns);

  if (otlp_metric_writer_) {
    otlp_metric_writer_->write_internal_stats_to_logging_core(agg_core_stats_, time_ns, shard, module);
  }

#if ENABLE_CODE_TIMING
  code_timing_registry_.visit(
      [&](std::string_view name, std::string_view filename, int line, u64 index, data::Gauge<u64> &data) {
        core_stats_.code_timing_stats(
            jb_blob(name),
            jb_blob(filename),
            line,
            index,
            data.count(),
            data.average<u64>(),
            data.min(),
            data.max(),
            data.sum(),
            time_ns);

        data.reset(); // reset each interval because metrics are being sent as AGGREGATION_TEMPORALITY_DELTA
      });
#endif // ENABLE_CODE_TIMING

  dump_internal_state(std::chrono::milliseconds{time_ns / (1000 * 1000)});
}

} // namespace reducer::aggregation
