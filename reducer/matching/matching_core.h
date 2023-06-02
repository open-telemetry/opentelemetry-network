/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/core_base.h>

#include <geoip/geoip.h>
#include <reducer/publisher.h>
#include <reducer/rpc_stats.h>
#include <reducer/tsdb_format.h>

#include <generated/ebpf_net/logging/writer.h>
#include <generated/ebpf_net/matching/connection.h>
#include <generated/ebpf_net/matching/index.h>
#include <generated/ebpf_net/matching/protocol.h>
#include <generated/ebpf_net/matching/span_base.h>
#include <generated/ebpf_net/matching/transform_builder.h>

#include <memory>

namespace reducer {
class RpcQueueMatrix;
}

namespace reducer::matching {

// This class implements the 'matching' app (see render definition file).
//
// It receives messages from one or more ingest core(s), matches received
// information to 'flow' spans, sends enriched flows to aggregation core(s) for
// aggreation and output to TSDB.
//
class MatchingCore : public CoreBase<
                         ebpf_net::matching::Index,
                         ebpf_net::matching::Protocol,
                         ebpf_net::matching::Connection,
                         ebpf_net::matching::TransformBuilder> {
public:
  // NOTE: must be called on startup, from main, before any matching cores are
  // created
  static void enable_aws_enrichment(bool enabled);

  // Enables using IP address for autonomous systems.
  static void set_autonomous_system_ip_enabled(bool enabled);
  // Returns whether using IP addresses for autonomous systems is enabled.
  static bool autonomous_system_ip_enabled();

  geoip::database an_db;

  MatchingCore(
      RpcQueueMatrix &ingest_to_matching_queues,
      RpcQueueMatrix &matching_to_aggregation_queues,
      RpcQueueMatrix &matching_to_logging_queues,
      std::optional<std::string> geoip_path,
      size_t shard_num,
      u64 initial_timestamp);

  // Logger instance.
  ::ebpf_net::matching::weak_refs::logger logger();

private:
  // Flag indicating whether IP addresses should be used for autonomous systems.
  static bool autonomous_system_ip_enabled_;

  // Keeper of ingest->this RPC stats.
  RpcReceiverStats ingest_to_matching_stats_;
  // Keeper of this->aggregation RPC stats.
  RpcSenderStats matching_to_aggregation_stats_;
  // Keeper of this->logging RPC stats.
  RpcSenderStats matching_to_logging_stats_;

  // accessor handle for core_worker_internal_metrics span
  ::ebpf_net::matching::auto_handles::core_stats core_stats_;

  // For writing logs to the logging core.
  ::ebpf_net::matching::auto_handles::logger logger_;

  void on_timeslot_complete() override;

  // Sends metrics from the metrics store to the aggregation core.
  void send_metrics_to_aggregation();

  // Outputs internal stats to be scraped by a time-series DB.
  void write_internal_stats() override;
};

} // namespace reducer::matching
