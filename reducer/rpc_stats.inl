// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <reducer/rpc_queue_matrix.h>

#include <util/element_queue_cpp.h>
#include <util/log_modifiers.h>

namespace reducer {

//// Writes internal stats to Logging core from different cores.
template <typename CoreStatsHandle>
void RpcSenderStats::write_internal_metrics_to_logging_core(CoreStatsHandle &internal_metrics, u64 time_ns)
{
  u64 num_write_stalls{0};

  for (auto &writer_ref : writers_) {
    num_write_stalls += writer_ref.get().num_write_stalls();
  }

  if (num_write_stalls > last_num_write_stalls_) {
    u64 count = num_write_stalls - last_num_write_stalls_;

    RpcWriteStallsStats stats;
    stats.labels.module = sender_app_;
    stats.labels.shard = std::to_string(sender_shard_);
    stats.labels.peer = receiver_app_;
    stats.metrics.stalls = count;
    internal_metrics.rpc_write_stalls_stats(jb_blob(sender_app_), sender_shard_, jb_blob(receiver_app_), count, time_ns);

    last_num_write_stalls_ = num_write_stalls;
  }

  RpcQueueUtilizationStats stats;
  stats.labels.module = sender_app_;
  stats.labels.shard = std::to_string(sender_shard_);
  stats.labels.peer = receiver_app_;

  stats.metrics.max_buf_used = max_buf_used_;
  stats.metrics.max_buf_util = max_buf_util_;
  stats.metrics.max_elem_util = max_elem_util_;

  internal_metrics.rpc_write_utilization_stats(
      jb_blob(sender_app_), sender_shard_, jb_blob(receiver_app_), max_buf_used_, max_buf_util_, max_elem_util_, time_ns);

  max_buf_used_ = 0;
  max_elem_count_ = 0;

  max_buf_util_ = 0.0;
  max_elem_util_ = 0.0;
}

//// Writes internal stats to Logging core from different cores.
template <typename CoreStatsHandle>
void RpcReceiverStats::write_internal_metrics_to_logging_core(CoreStatsHandle &internal_metrics, u64 time_ns)
{
  RpcLatencyStats stats;
  stats.labels.module = receiver_app_;
  stats.labels.shard = std::to_string(receiver_shard_);
  stats.labels.peer = sender_app_;
  stats.metrics.max_latency_ns = max_latency_ns_;
  internal_metrics.rpc_receive_stats(jb_blob(receiver_app_), receiver_shard_, jb_blob(sender_app_), max_latency_ns_, time_ns);

  max_latency_ns_ = 0;
}

} // namespace reducer