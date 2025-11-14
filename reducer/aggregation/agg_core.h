/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/aggregation/connection.h>
#include <generated/ebpf_net/aggregation/index.h>
#include <generated/ebpf_net/aggregation/protocol.h>
#include <generated/ebpf_net/aggregation/transform_builder.h>
#include <reducer/core_base.h>

#include <memory>
#include <vector>

// cxx::bridge header for Rust AggregationCore
#include <reducer_aggregation_cxxbridge.h>

namespace reducer {
class RpcQueueMatrix;
}

namespace reducer::aggregation {

class AggCore : public CoreBase<
                    ebpf_net::aggregation::Index,
                    ebpf_net::aggregation::Protocol,
                    ebpf_net::aggregation::Connection,
                    ebpf_net::aggregation::TransformBuilder> {
public:
  // Disables using IP address in node spans.
  static void set_node_ip_field_disabled(bool disabled);
  // Returns whether using IP addresses in node spans is disabled.
  static bool node_ip_field_disabled() { return node_ip_field_disabled_; }

  // Enables generating node-node (id-id) metrics.
  static void set_id_id_enabled(bool enabled);

  // Enables generating az-node metrics.
  static void set_az_id_enabled(bool enabled);

  // Enables generating flow logs from node-node (id-id) metrics.
  static void set_flow_logs_enabled(bool enabled);

  AggCore(
      RpcQueueMatrix &matching_to_aggregation_queues,
      RpcQueueMatrix &aggregation_to_logging_queues,
      size_t shard_num,
      u64 initial_timestamp,
      std::string otlp_endpoint,
      bool disable_node_ip_field);

private:
  // Flag indicating whether using IP addresses in node spans is disabled.
  static bool node_ip_field_disabled_;

  // Flag indicating whether id-id timeseries should be outputted.
  static bool id_id_enabled_;

  // Flag indicating whether az-id timeseries should be outputted.
  static bool az_id_enabled_;

  // Flag indicating whether flow logs should be outputted.
  static bool flow_logs_enabled_;

public:
  // Run the aggregation core using the Rust implementation.
  void run();
  // Stop the Rust core.
  void stop_async();

private:
  // Opaque Rust AggregationCore owned via cxx rust::Box
  rust::Box<reducer_agg::AggregationCore> rust_core_;
};

} // namespace reducer::aggregation
