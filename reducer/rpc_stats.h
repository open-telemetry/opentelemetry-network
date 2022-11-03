/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>

#include <generated/ebpf_net/matching/auto_handles.h>

#include <chrono>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

class ElementQueueWriter;

namespace reducer {

class RpcQueueMatrix;
class InternalMetricsEncoder;

// Collects and writes out stats on RPC queues from sender's standpoint.
//
// Each core that sends RPC messages will have one object of this class
// per receiver application.
//
class RpcSenderStats {
public:
  // Constructs the object.
  // sender_shard -- index of the core this object is used in
  // sender_app -- name of the application this core is running
  // receiver_app -- name of the application the receiver core is running
  //
  RpcSenderStats(int sender_shard, std::string_view sender_app, std::string_view receiver_app, RpcQueueMatrix &queues);

  // Checks utilization of all queues this sender is writing to.
  void check_utilization();

  // Writes stats out.
  void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns);

  //// Writes internal stats to Logging core from different cores.
  template <typename CoreStatsHandle>
  void write_internal_metrics_to_logging_core(CoreStatsHandle &internal_metrics, u64 time_ns);

private:
  int sender_shard_;
  std::string sender_app_;
  std::string receiver_app_;

  // Access to writes this sender is using.
  using QueueWriterRef = std::reference_wrapper<ElementQueueWriter>;
  std::vector<QueueWriterRef> writers_;

  // Last umber of write stalls.
  u64 last_num_write_stalls_{0};

  // Maximum usage of queue buffer, in bytes.
  u32 max_buf_used_{0};
  // Maximum usage of queue elements, in number of elements
  u32 max_elem_count_{0};

  // Maximum ratio of queue buffer usage and capacity.
  double max_buf_util_{0};
  // Maximum ratio of queue elements usage and capacity.
  double max_elem_util_{0};
};

// Collects and writes out stats on RPC queues from receiver's standpoint.
//
// Each core that receives RPC messages will have one object of this class
// per sender application.
//

class RpcReceiverStats {
public:
  // Constructs the object.
  // receiver_shard -- index of the core this object is used in
  // sender_app -- name of the application that the sender core is running
  // receiver_app -- name of the application this core is running
  //
  RpcReceiverStats(int receiver_shard, std::string_view sender_app, std::string_view receiver_app);

  // Records the supplied time differential (latency).
  // This is the difference between the time when a message is send and the time
  // when it was extracted from the queue.
  void record_latency(std::chrono::nanoseconds time_diff);

  // Writes stats out.
  void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns);

  // Writes stats out to Logging core.
  template <typename CoreStatsHandle>
  void write_internal_metrics_to_logging_core(CoreStatsHandle &internal_metrics, u64 time_ns);

private:
  int receiver_shard_;
  std::string sender_app_;
  std::string receiver_app_;

  u64 max_latency_ns_{0};
};

} // namespace reducer

#include "rpc_stats.inl"
