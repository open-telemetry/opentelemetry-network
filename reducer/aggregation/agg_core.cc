// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "agg_core.h"
#include <reducer/constants.h>
#include <reducer/otlp_grpc_formatter.h>
#include <reducer/rpc_queue_matrix.h>

#include <common/constants.h>

#include <platform/userspace-time.h>
#include <util/log.h>
#include <util/time.h>

#include <stdexcept>

#include <reducer/util/thread_ops.h>

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
    size_t shard_num,
    u64 initial_timestamp,
    std::string otlp_endpoint,
    bool disable_node_ip_field)
    : CoreBase(
          "aggregation",
          shard_num,
          initial_timestamp,
          aggregation_to_logging_queues.make_writers<ebpf_net::logging::Writer>(
              shard_num, std::bind(&Core::current_timestamp, this))),
      rust_core_([&] {
        auto readers = matching_to_aggregation_queues.make_readers(shard_num);
        std::vector<reducer_agg::EqView> eqs;
        eqs.reserve(readers.size());
        for (auto &q : readers) {
          reducer_agg::EqView v;
          v.data = reinterpret_cast<uint8_t *>(q.shared);
          v.n_elems = q.elem_mask + 1;
          v.buf_len = q.buf_mask + 1;
          eqs.push_back(v);
        }
        return reducer_agg::aggregation_core_new(
            eqs,
            static_cast<uint32_t>(shard_num),
            id_id_enabled_,
            az_id_enabled_,
            otlp_endpoint,
            disable_node_ip_field,
            reducer::OtlpGrpcFormatter::metric_description_field_enabled());
      }())
{}

// Note: C++ internal metric emission paths are not used by the Rust core.

void AggCore::run()
{
  set_self_thread_name(fmt::format("{}_{}", app_name(), shard_num())).on_error([this](auto const &error) {
    LOG::warn("unable to set name for {} core thread {}: {}", app_name(), shard_num(), error);
  });

  // Delegate to Rust AggregationCore until stop
  rust_core_->aggregation_core_run();
}

void AggCore::stop_async()
{
  // Cooperative stop for the Rust core
  rust_core_->aggregation_core_stop();
}

} // namespace reducer::aggregation
