/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "absl/base/thread_annotations.h"
#include <reducer/ingest/ingest_worker.h>
#include <reducer/load_balancer.h>
#include <reducer/prometheus_handler.h>

#include <generated/ebpf_net/ingest/index.h>

#include <uv.h>

#include <cstddef>

namespace reducer::ingest {

// TcpServer is responsible for opening a TCP server port, accepting
// connecitons, and assigning them to workers for processing. It transitively
// uses all the Index objects used in the first stage of the pipeline.
class TcpServer {
public:
  struct Stats {
    Stats() : connection_counter(0), disconnect_counter(0) {}

    u64 connection_counter;
    u64 disconnect_counter;
  };

  // Arguments:
  // * loop - The loop on which the port will be opneed.
  // * telemetry_port - The port the tcp connection will listen on.
  // * localhsot - If true, connects to 127.0.0.1, otherwise uses 0.0.0.0
  // * workers - The ingest workers owned by this class. This constructor
  //    will overwrite the close callback used by these workers.
  TcpServer(uv_loop_t &loop, u32 telemetry_port, bool localhost, std::vector<std::unique_ptr<IngestWorker>> workers);
  ~TcpServer();

  // Returns various stats related to connects/disconnects, etc.
  Stats get_stats();

  // Calls `cb` on the Index objects owned by the workers of this class (the
  // int argument is the index of the worker).
  // These will be executed in parallel. If `block` is true, this function will
  // block until each callback has finished, otherwise it will return early
  // while the callbacks run in the background.
  using IndexCb = std::function<void(int, ::ebpf_net::ingest::Index *)>;
  void visit_indexes(const IndexCb &cb, bool block);

  // Same as above, but for NpmConnection instances. The int parameter
  // is still like IngestWorker index, like above, not the NpmConnection
  // index.
  using ConnectionCb = std::function<void(int, NpmConnection *)>;
  void visit_connections(const ConnectionCb &cb, bool block);

  // Same as above, but for channels.
  using ChannelCb = std::function<void(int, channel::TCPChannel *, std::chrono::nanoseconds)>;
  void visit_channels(const ChannelCb &cb, bool block);

  // Same as above, but for RpcSenderStats.
  using RpcStatsCb = std::function<void(int, RpcSenderStats &)>;
  void visit_rpc_stats(const RpcStatsCb &cb, bool block);

  std::size_t workers_count() const { return workers_.size(); }

  // Global accessor for the TcpSever. Used by classes who want use the
  // `visit_*` functions but do not have direct access to a `TcpSever` instance
  // for whatever reason (e.g. render-instantiated `*Span` classes).
  // It is the caller's responsibility to make sure that `instance` has already
  // been set to a valid value before using it.
  struct Singleton {
  private:
    // Classes are given whitelisted access. Global variables are generally an
    // antipattern, and therefore this list should only be extended if there is
    // no other reasonable alternative.

    // Classes that set `instance`.
    friend class IngestCore;
    friend class TestServer; // Used in agent_span_pod_test.cc

    // Classes that read `instance`.
    friend class AgentSpan;

    TcpServer *instance = nullptr;
  };

  static Singleton *singleton();

private:
  // libuv callback invoked when a new conneciton arrives.
  static void on_new_connection_cb(uv_stream_t *stream, int status);

  // Basically same as above, but as member function.
  void on_new_connection();

  // Callback invoked when a connection on `worker` has been closed.
  void on_connection_close(IngestWorker *worker);

  // Internal visitor implementation.
  using WorkerVisitCb = std::function<std::shared_ptr<absl::Notification>(int, IngestWorker *)>;
  void visit_internal(const WorkerVisitCb &cb, bool block);

  uv_loop_t &loop_;
  uv_tcp_t server_;

  std::vector<std::unique_ptr<IngestWorker>> workers_;
  std::unique_ptr<LoadBalancer<Worker *>> worker_balancer_;

  Stats stats_ ABSL_GUARDED_BY(stats_mu_);
  mutable absl::Mutex stats_mu_;
};

} /* namespace reducer::ingest */
