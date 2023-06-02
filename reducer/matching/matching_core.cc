// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0
#include <config.h>

#include <reducer/matching/component.h>
#include <reducer/matching/matching_core.h>

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <reducer/rpc_queue_matrix.h>

#include <common/constants.h>

#include <generated/ebpf_net/matching/containers.h>
#include <generated/ebpf_net/matching/containers.inl>

#include <platform/userspace-time.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/log_modifiers.h>
#include <util/time.h>

#include <functional>
#include <stdexcept>

namespace reducer::matching {

namespace {

geoip::database make_geoip_db(std::optional<std::string> const &path)
{
  if (!path) {
    // Return a no-op database object.
    return geoip::database(std::nothrow, {});
  }

  return geoip::database(std::nothrow, path->c_str());
}

} // namespace

bool MatchingCore::autonomous_system_ip_enabled_ = false;

bool MatchingCore::autonomous_system_ip_enabled()
{
  return autonomous_system_ip_enabled_;
}

void MatchingCore::set_autonomous_system_ip_enabled(bool enabled)
{
  autonomous_system_ip_enabled_ = enabled;
}

void MatchingCore::enable_aws_enrichment(bool enabled)
{
  FlowSpan::enable_aws_enrichment(enabled);
}

MatchingCore::MatchingCore(
    RpcQueueMatrix &ingest_to_matching_queues,
    RpcQueueMatrix &matching_to_aggregation_queues,
    RpcQueueMatrix &matching_to_logging_queues,
    std::optional<std::string> geoip_path,
    size_t shard_num,
    u64 initial_timestamp)
    : CoreBase(
          "matching",
          shard_num,
          initial_timestamp,
          matching_to_aggregation_queues.make_writers<ebpf_net::aggregation::Writer>(
              shard_num, std::bind(&Core::current_timestamp, this)),
          matching_to_logging_queues.make_writers<ebpf_net::logging::Writer>(
              shard_num, std::bind(&Core::current_timestamp, this))),
      an_db(make_geoip_db(geoip_path)),
      ingest_to_matching_stats_(shard_num, "ingest", "matching"),
      matching_to_aggregation_stats_(shard_num, "matching", "aggregation", matching_to_aggregation_queues),
      matching_to_logging_stats_(shard_num, "matching", "logging", matching_to_logging_queues),
      core_stats_(index_.core_stats.alloc()),
      logger_(index_.logger.alloc())
{
  add_rpc_clients(ingest_to_matching_queues.make_readers(shard_num), ClientType::ingest, ingest_to_matching_stats_);
}

ebpf_net::matching::weak_refs::logger MatchingCore::logger()
{
  return logger_;
}

void MatchingCore::on_timeslot_complete()
{
  send_metrics_to_aggregation();

  matching_to_aggregation_stats_.check_utilization();
  matching_to_logging_stats_.check_utilization();

  index_.send_pulse();
}

void MatchingCore::send_metrics_to_aggregation()
{
  // use one timeslot before the current one
  u64 slot_timestamp = current_timestamp() - (u64)timeslot_duration();

  if (index_.flow.tcp_a_to_b_ready(slot_timestamp)) {
    FlowSpan::send_metrics_to_aggregation(index_.flow, slot_timestamp);
  }
}

void MatchingCore::write_internal_stats()
{
  u64 time_ns = fp_get_time_ns();
  write_common_stats_to_logging_core(core_stats_, time_ns);
  ingest_to_matching_stats_.write_internal_metrics_to_logging_core(core_stats_, time_ns);
  matching_to_aggregation_stats_.write_internal_metrics_to_logging_core(core_stats_, time_ns);
  matching_to_logging_stats_.write_internal_metrics_to_logging_core(core_stats_, time_ns);

  dump_internal_state(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds{time_ns}));
}

} // namespace reducer::matching
