/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "core.h"

#include <reducer/publisher.h>
#include <reducer/rpc_stats.h>
#include <reducer/util/index_dumper.h>

#include <generated/ebpf_net/matching/auto_handles.h>

#include <common/client_type.h>
#include <platform/userspace-time.h>
#include <util/short_string.h>

#include <spdlog/fmt/fmt.h>

namespace reducer {

class InternalMetricsEncoder;

// Helper for implementing core classes.
//
template <typename Index, typename Protocol, typename Connection, typename TransformBuilder> class CoreBase : public Core {
protected:
  struct RpcHandler : public IRpcHandler {
    RpcHandler(
        Index &index,
        TransformBuilder &transform_builder,
        ClientType client_type,
        std::size_t client_index,
        RpcReceiverStats &rpc_receiver_stats)
        : protocol(transform_builder), connection(protocol, index), receiver_stats(rpc_receiver_stats)
    {
      conn_name = fmt::format("inproc-conn-{}", client_index);
    }

    void handle(u64 current_timestamp_ns, char *buf, size_t len) override
    {
      const std::chrono::nanoseconds current_timestamp{current_timestamp_ns};

      auto res = protocol.handle(buf, len);

      if (res.client_timestamp.count() > 0) {
        receiver_stats.record_latency(current_timestamp - res.client_timestamp);
      }
    }

    void set_connection_authenticated() override { connection.on_connection_authenticated(); }

    Protocol protocol;
    Connection connection;
    RpcReceiverStats &receiver_stats;
    short_string<16> conn_name;
  };

  // Transform builder for JIT-ing incoming messages.
  TransformBuilder transform_builder_;
  // Index object belonging to this core.
  Index index_;
  // A rate-limited helper to dump the core's index.
  IndexDumper index_dumper_;

  template <typename... Args>
  CoreBase(std::string_view app_name, size_t shard_num, u64 initial_timestamp, Args &&...args)
      : Core(app_name, shard_num, initial_timestamp), transform_builder_(), index_(std::forward<Args>(args)...)
  {}

  void add_rpc_clients(std::vector<ElementQueue> const &queues, ClientType client_type, RpcReceiverStats &receiver_stats)
  {
    for (auto &queue : queues) {
      const auto client_index = rpc_clients_.size();
      rpc_clients_.emplace_back(
          queue,
          std::make_unique<RpcHandler>(index_, transform_builder_, client_type, client_index, receiver_stats),
          client_type);
    }

    virtual_clock_.add_inputs(queues.size());
  }

  // Writes internal stats common to all core types.
  void write_common_stats(InternalMetricsEncoder &encoder, u64 time_ns);

  template <typename CoreStatsHandle> void write_common_stats_to_logging_core(CoreStatsHandle &internal_metrics, u64 time_ns);

  void dump_internal_state(std::chrono::milliseconds timestamp);
};

} // namespace reducer

#include "core_base.inl"
