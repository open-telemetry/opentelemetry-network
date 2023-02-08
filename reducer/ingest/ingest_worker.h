/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "npm_connection.h"

#include <reducer/rpc_stats.h>
#include <reducer/worker.h>

#include <generated/ebpf_net/ingest/index.h>
#include <generated/ebpf_net/ingest/span_base.h>
#include <generated/ebpf_net/logging/writer.h>

#include <channel/callbacks.h>

#include <util/log.h>
#include <util/lz4_decompressor.h>

#include <absl/time/time.h>
#include <uv.h>

#include <memory>
#include <optional>

namespace reducer {
class RpcQueueMatrix;
}

namespace reducer::ingest {

// IngestWorker is responsible to reading npm_connection messages via TCP connections
// from the agent and forwarding them to the various spans in the reducer.
//
class IngestWorker : public Worker {
public:
  using OnCloseCallback = std::function<void()>;

  // Arguments:
  // - index - The ingest index that will be owned by this class.
  // Calling this constructor will set the `local_index()` value.
  IngestWorker(RpcQueueMatrix &ingest_to_logging_queues, RpcQueueMatrix &ingest_to_matching_queues, u32 shard_num);
  ~IngestWorker() override;

  // Registers a callback that will be invoked everytime a TCP connection
  // is closed. Overwrites the previous callback if this function has already
  // been called.
  void register_close_callback(OnCloseCallback on_close_cb);

  // The set of callbacks invoked when data arrives over a TCP connection.
  // There will be an instance of this class for every established connection.
  // When `received_data` is invoked, `local_connection` will be overwritten by
  // the NpmConnection instance owned by this class.
  class Callbacks : public ::channel::Callbacks {
  public:
    Callbacks(IngestWorker *worker, channel::TCPChannel *tcp_channel);
    ~Callbacks() override;

    uint32_t received_data(const u8 *data, int data_len) override;
    void on_error(int err) override;
    void on_closed() override;

  private:
    friend class IngestWorker;

    // Internal data processing callback. If successful, return the number of
    // bytes processed. If an error occurred, returns nullopt (which will cause
    // the connection to close).
    std::optional<uint32_t> received_data_internal(const u8 *data, int data_len);

    IngestWorker *worker_;
    channel::TCPChannel *channel_;
    Lz4Decompressor decompressor_;

    std::unique_ptr<NpmConnection> connection_;

    bool decompressor_active_ = false;
    bool first_message_seen_ = false;
    std::chrono::nanoseconds last_message_seen_;
  };

  // Invokes the provided callback in this worker's thread, allowing one to
  // access the contained `Index` without worry about multithreading issues.
  // See `Worker::visit_thread` for usage notes.
  using IndexCb = std::function<void(::ebpf_net::ingest::Index *)>;
  [[nodiscard]] std::shared_ptr<absl::Notification> visit_index(IndexCb cb);

  // Same as above, but runs `cb` on each of this class's connections'
  // `NpmConnection` objects.
  using ConnectionCb = std::function<void(NpmConnection *)>;
  [[nodiscard]] std::shared_ptr<absl::Notification> visit_connections(ConnectionCb cb);

  // Same as above, but runs `cb` on each of this class's channel.
  using ChannelCb = std::function<void(channel::TCPChannel *, std::chrono::nanoseconds)>;
  [[nodiscard]] std::shared_ptr<absl::Notification> visit_channels(ChannelCb cb);

  // Same as above, but runs `b` on worker's RpcSenderStats.
  using RpcStatsCb = std::function<void(RpcSenderStats &)>;
  std::shared_ptr<absl::Notification> visit_rpc_stats(RpcStatsCb cb);

protected:
  void on_thread_start() override;
  void on_thread_stop() override;

  std::unique_ptr<::channel::Callbacks> create_callbacks(uv_loop_t &loop, ::channel::TCPChannel *tcp_channel) override;

private:
  OnCloseCallback on_close_cb_;
  RpcSenderStats ingest_to_logging_stats_;
  RpcSenderStats ingest_to_matching_stats_;
  std::unique_ptr<::ebpf_net::ingest::Index> index_;
  ::ebpf_net::ingest::auto_handles::logger logger_;
  ::ebpf_net::ingest::auto_handles::core_stats core_stats_;
  ::ebpf_net::ingest::auto_handles::ingest_core_stats ingest_core_stats_;

  friend class Callbacks;
};

} // namespace reducer::ingest
