// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "rpc_stats.h"

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <reducer/rpc_queue_matrix.h>

#include <util/element_queue_cpp.h>
#include <util/log_modifiers.h>

namespace reducer {

RpcSenderStats::RpcSenderStats(
    int sender_shard, std::string_view sender_app, std::string_view receiver_app, RpcQueueMatrix &queues)
    : sender_shard_(sender_shard),
      sender_app_(sender_app),
      receiver_app_(receiver_app),
      writers_(queues.make_writers<QueueWriterRef>(sender_shard))
{}

void RpcSenderStats::check_utilization()
{
  for (auto &writer_ref : writers_) {
    ElementQueue const &queue = writer_ref.get().queue();

    u32 buf_used = queue.buf_used();
    u32 elem_count = queue.elem_count();

    double buf_util = buf_used / (double)queue.buf_capacity();
    double elem_util = elem_count / (double)queue.elem_capacity();

    max_buf_used_ = std::max(max_buf_used_, buf_used);
    max_elem_count_ = std::max(max_elem_count_, elem_count);

    max_buf_util_ = std::max(max_buf_util_, buf_util);
    max_elem_util_ = std::max(max_elem_util_, elem_util);
  }
}

void RpcSenderStats::write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns)
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
    encoder.write_internal_stats(stats, time_ns);

    last_num_write_stalls_ = num_write_stalls;
  }

  RpcQueueUtilizationStats stats;
  stats.labels.module = sender_app_;
  stats.labels.shard = std::to_string(sender_shard_);
  stats.labels.peer = receiver_app_;

  stats.metrics.max_buf_used = max_buf_used_;
  stats.metrics.max_buf_util = max_buf_util_;
  stats.metrics.max_elem_util = max_elem_util_;

  encoder.write_internal_stats(stats, time_ns);

  max_buf_used_ = 0;
  max_elem_count_ = 0;

  max_buf_util_ = 0.0;
  max_elem_util_ = 0.0;
}

////////////////////////////////////////////////////////////////////////////////

RpcReceiverStats::RpcReceiverStats(int receiver_shard, std::string_view sender_app, std::string_view receiver_app)
    : receiver_shard_(receiver_shard), sender_app_(sender_app), receiver_app_(receiver_app)
{}

void RpcReceiverStats::record_latency(std::chrono::nanoseconds time_diff)
{
  if (time_diff.count() <= 0) {
    return;
  }

  max_latency_ns_ = std::max(max_latency_ns_, (u64)time_diff.count());
}

void RpcReceiverStats::write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns)
{
  RpcLatencyStats stats;
  stats.labels.module = receiver_app_;
  stats.labels.shard = std::to_string(receiver_shard_);
  stats.labels.peer = sender_app_;
  stats.metrics.max_latency_ns = max_latency_ns_;
  encoder.write_internal_stats(stats, time_ns);

  max_latency_ns_ = 0;
}

} // namespace reducer
