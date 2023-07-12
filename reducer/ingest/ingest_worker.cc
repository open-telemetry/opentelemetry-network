// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "ingest_worker.h"
#include "npm_connection.h"
#include "shared_state.h"

#include <reducer/rpc_queue_matrix.h>
#include <reducer/worker.h>

#include <generated/ebpf_net/ingest/index.h>
#include <generated/ebpf_net/ingest/modifiers.h>

#include <channel/callbacks.h>

#include <platform/userspace-time.h>

#include <util/boot_time.h>
#include <util/error_handling.h>
#include <util/log.h>

#include <absl/time/time.h>

#include <uv.h>

#include <memory>
#include <optional>

namespace reducer::ingest {

IngestWorker::IngestWorker(RpcQueueMatrix &ingest_to_logging_queues, RpcQueueMatrix &ingest_to_matching_queues, u32 shard_num)
    : ingest_to_logging_stats_(shard_num, "ingest", "logging", ingest_to_logging_queues),
      ingest_to_matching_stats_(shard_num, "ingest", "matching", ingest_to_matching_queues),
      index_(std::make_unique<ebpf_net::ingest::Index>(
          ingest_to_logging_queues.make_writers<ebpf_net::logging::Writer>(shard_num, monotonic, get_boot_time()),
          ingest_to_matching_queues.make_writers<ebpf_net::matching::Writer>(shard_num, monotonic, get_boot_time()))),
      logger_(index_->logger.alloc()),
      core_stats_(index_->core_stats.alloc()),
      ingest_core_stats_(index_->ingest_core_stats.alloc())
{}

IngestWorker::~IngestWorker() {}

void IngestWorker::register_close_callback(OnCloseCallback on_close_cb)
{
  on_close_cb_ = std::move(on_close_cb);
}

std::shared_ptr<absl::Notification> IngestWorker::visit_index(IndexCb cb)
{
  return visit_thread([this, captured_cb = std::move(cb)] { captured_cb(index_.get()); });
}

std::shared_ptr<absl::Notification> IngestWorker::visit_connections(ConnectionCb cb)
{
  return visit_callbacks([this, captured_cb = std::move(cb)](::channel::Callbacks *const callbacks) {
    auto *const ingest_callbacks = static_cast<IngestWorker::Callbacks *>(callbacks);
    captured_cb(ingest_callbacks->connection_.get());
  });
}

std::shared_ptr<absl::Notification> IngestWorker::visit_channels(ChannelCb cb)
{
  return visit_callbacks([this, captured_cb = std::move(cb)](::channel::Callbacks *const callbacks) {
    auto *const ingest_callbacks = static_cast<IngestWorker::Callbacks *>(callbacks);
    captured_cb(ingest_callbacks->channel_, ingest_callbacks->last_message_seen_);
  });
}

std::shared_ptr<absl::Notification> IngestWorker::visit_rpc_stats(RpcStatsCb cb)
{
  return visit_thread([this, captured_cb = std::move(cb)] {
    captured_cb(ingest_to_logging_stats_);
    captured_cb(ingest_to_matching_stats_);
  });
}

void IngestWorker::on_thread_start()
{
  set_local_index(index_.get());
  set_local_logger(&logger_);
  set_local_core_stats_handle(&core_stats_);
  set_local_ingest_core_stats_handle(&ingest_core_stats_);
}

void IngestWorker::on_thread_stop()
{
  set_local_index(nullptr);
  set_local_logger(nullptr);
  set_local_core_stats_handle(nullptr);
  set_local_ingest_core_stats_handle(nullptr);
  set_local_connection(nullptr);
}

std::unique_ptr<::channel::Callbacks> IngestWorker::create_callbacks(uv_loop_t &loop, ::channel::TCPChannel *const tcp_channel)
{
  return std::make_unique<IngestWorker::Callbacks>(this, tcp_channel);
}

IngestWorker::Callbacks::Callbacks(IngestWorker *worker, channel::TCPChannel *channel)
    : worker_(worker), channel_(channel), decompressor_(Worker::kBufferSize)
{
  assert(local_index() == worker_->index_.get());

  connection_ = std::make_unique<NpmConnection>(*worker_->index_);

  last_message_seen_ = std::chrono::nanoseconds(fp_get_time_ns());
}

IngestWorker::Callbacks::~Callbacks() {}

uint32_t IngestWorker::Callbacks::received_data(const u8 *data, int data_len)
{
  const u8 *begin = data;
  const u8 *const end = data + data_len;
  u16 count = 0;

  // Process the raw data as-is if the decompressor is not active.
  while (!decompressor_active_) {
    const std::optional<uint32_t> bytes_consumed = received_data_internal(begin, end - begin);
    if (!bytes_consumed.has_value()) {
      // If nullopt, then close the channel.
      channel_->close_permanently();
      return 0;
    }

    if (*bytes_consumed == 0) {
      // Not enough data received.
      return 0;
    }

    // Increment the begin pointer by the bytes consumed.
    ASSUME(static_cast<int>(*bytes_consumed) <= (end - begin));
    begin += *bytes_consumed;
    ++count;

    if (begin == end) {
      // Everything has been consumed.
      return data_len;
    }
  }

  // If this loop is reached, either this is
  // 1) The first message received (and whether or not compression is being
  //    used is being negotiated).
  // 2) This is a subsequent data message, and it is compressed.
  size_t consumed_len = 0;
  do {
    const size_t res = decompressor_.process(begin, end - begin, &consumed_len);

    // Check if decompression failed.
    if (res != 0) {
      local_logger().ingest_decompression_error(
          static_cast<u8>(connection_->client_type()),
          jb_blob(connection_->client_hostname()),
          jb_blob(std::string_view(LZ4F_getErrorName(res))));
      channel_->close_permanently();
      return 0;
    }

    // Process the decompressed data.
    begin += consumed_len;

    ASSUME(decompressor_active_);
    const std::optional<uint32_t> consumed_uncompressed =
        received_data_internal(decompressor_.output_buf(), decompressor_.output_buf_size());

    // An error occurred, close.
    if (!consumed_uncompressed) {
      channel_->close_permanently();
      return 0;
    }

    // Remove the handled bytes from decompression buffer.
    decompressor_.discard(*consumed_uncompressed);
    ++count;

    // * if we weren't able to decompress any bytes, can exit -- another
    //   iteration will not make progress.
    // * if we were able to decompress everything, no need for another
    //   iteration.
    // * if we were able to decompress, regardless of whether handle_multiple
    //   processed bytes or not, another call to the decompressor might make
    //   more bytes available, so if there are more compressed bytes, we should
    //   make another iteration.
  } while ((consumed_len > 0) && (begin < end));

  worker_->ingest_to_logging_stats_.check_utilization();
  worker_->ingest_to_matching_stats_.check_utilization();
  worker_->invoke_visitors();

  return end - data;
}

std::optional<uint32_t> IngestWorker::Callbacks::received_data_internal(const u8 *const data, const int data_len)
{
  last_message_seen_ = std::chrono::nanoseconds(fp_get_time_ns());

  auto *ft_conn = connection_.get();

  try {
    set_local_connection(ft_conn);
    const int res = first_message_seen_ ? ft_conn->handle_multiple((const char *)data, data_len)
                                        : ft_conn->handle((const char *)data, data_len);

    // Check if the received data buffer is too small for the message type.
    // (but do not close).
    if (res == -EAGAIN) {
      return 0;
    }

    // Process other types of errors.
    if (res < 0) {
      local_logger().ingest_processing_error(
          static_cast<u8>(ft_conn->client_type()),
          jb_blob(ft_conn->client_hostname()),
          jb_blob(std::string_view(strerror(res))));
      return std::nullopt;
    }

    // If this is the first message, turn on decompression of further messages.
    if (!first_message_seen_) {
      decompressor_active_ = true;
      first_message_seen_ = true;
    }

    return res;
  } catch (const std::exception &e) {
    // Catch any thrown errors.
    local_logger().ingest_processing_error(
        static_cast<u8>(ft_conn->client_type()), jb_blob(ft_conn->client_hostname()), jb_blob(std::string_view(e.what())));
    return std::nullopt;
  }
}

void IngestWorker::Callbacks::on_error(const int err)
{
  const ClientType client_type = connection_->client_type();
  const std::string_view client_hostname = connection_->client_hostname();

  if (err == UV_EOF) {
    // Client disconnected. Print message if it is a collector client.
    if ((client_type != ClientType::liveness_probe) && (client_type != ClientType::readiness_probe)) {
      LOG::info("Connection closed from {} collector at '{}'", client_type, client_hostname);
    }
  } else {
    // Connection error.
    local_logger().ingest_connection_error(
        static_cast<u8>(client_type), jb_blob(client_hostname), jb_blob(std::string_view(uv_strerror(err))));
  }
}

void IngestWorker::Callbacks::on_closed()
{
  worker_->on_close_cb_();
}

} // namespace reducer::ingest
