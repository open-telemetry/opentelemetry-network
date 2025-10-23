// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "prometheus_publisher.h"

#include "prometheus_handler.h"

#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>

#include <util/log.h>
#include <util/time.h>

#include <CivetServer.h>

#include <stdexcept>
#include <string>

namespace reducer {

namespace {

// 1MB upper limit
constexpr std::streamoff UPPER_LIMIT = 1 * 1024 * 1024;

std::vector<ElementQueueStoragePtr> make_queue_buffers(size_t n)
{
  static constexpr u32 queue_n_elems = (1 << 19);
  // Each element is 4 bytes
  static constexpr u32 queue_buf_len = (queue_n_elems << 5);

  assert(n > 0);

  std::vector<ElementQueueStoragePtr> result;

  for (size_t i = 0; i < n; ++i) {
    result.emplace_back(std::make_shared<MemElementQueueStorage>(queue_n_elems, queue_buf_len));
  }

  return result;
}

template <typename... Args>
std::unique_ptr<PrometheusHandler> make_handler(PrometheusPublisher::HandlerType type, Args &&...args)
{
  switch (type) {
  case PrometheusPublisher::SINGLE_PORT:
    return std::make_unique<SinglePortPromHandler>(std::forward<Args>(args)...);
  case PrometheusPublisher::PORT_RANGE:
    return std::make_unique<PortRangePromHandler>(std::forward<Args>(args)...);
  default:
    throw std::runtime_error("unknown value for handler type");
  }
}

std::unique_ptr<CivetServer> make_http_server(std::string_view bind_addr, int num_threads)
{
  std::string listening_ports{bind_addr.data(), bind_addr.size()};

  std::vector<std::string> options{
      "listening_ports",
      listening_ports,
      "num_threads",
      std::to_string(num_threads),
  };

  return std::make_unique<CivetServer>(options);
}

} // namespace

PrometheusPublisher::PrometheusPublisher(
    HandlerType handler_type,
    size_t num_writer_threads,
    std::string_view http_bind_addr,
    int http_num_threads,
    std::optional<u64> scrape_size_limit_bytes)
    : queue_buffers_(make_queue_buffers(num_writer_threads)),
      http_handler_(make_handler(handler_type, queue_buffers_, scrape_size_limit_bytes)),
      http_server_(make_http_server(http_bind_addr, http_num_threads))
{
  http_server_->addHandler("", *http_handler_);
}

PrometheusPublisher::~PrometheusPublisher() {}

Publisher::WriterPtr PrometheusPublisher::make_writer(size_t thread_num)
{
  assert(thread_num < queue_buffers_.size());
  return std::make_unique<Writer>(queue_buffers_[thread_num]);
}

u64 PrometheusPublisher::bytes_served() const
{
  return http_handler_->bytes_served();
}

u64 PrometheusPublisher::num_failed_scrapes() const
{
  return http_handler_->num_failed_scrapes();
}

void PrometheusPublisher::write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const
{
  PromStats stats;
  stats.metrics.bytes_ingested = bytes_served();
  stats.metrics.failed_scrapes = num_failed_scrapes();
  encoder.write_internal_stats(stats, time_ns);
}

////////////////////////////////////////////////////////////////////////////////
// Writer
//

namespace {

ssize_t write_to_queue(ElementQueue &write_queue, std::stringstream &ss)
{
  size_t len = ss.tellp();

  // write msg to queue to be read by by prometheus handler
  int res = eq_write(&write_queue, len);

  if (res == -EINVAL) {
    // attempted to write more than the queue size! This is never okay,
    // and should never happen. In this catastrophic case, throw an
    // exception so we can know about it and debug.
    throw std::runtime_error("tried to write more than queue length");
  }

  if (res == -ENOSPC) {
    // no space in the queue
    return -1;
  }

  assert(res >= 0);

  // if we reached here, can write the element to offset
  ss.read((char *)write_queue.data + res, len);

  return len;
}

} // namespace

PrometheusPublisher::Writer::Writer(ElementQueueStoragePtr const &queue) : write_queue_(queue)
{
  write_queue_.start_write_batch();
}

PrometheusPublisher::Writer::~Writer()
{
  write_queue_.finish_write_batch();
}

void PrometheusPublisher::Writer::write(std::stringstream &stream)
{
  stream.seekp(0, std::ios_base::end);

  if (stream.tellp() >= UPPER_LIMIT) {
    ++stats_.big_items_dropped;

    LOG::error(
        "dropping stringstream since it is above the upper limit"
        " - {} bytes won't be written to the TSDB",
        static_cast<std::streamoff>(stream.tellp()));
    return;
  }

  if (stream.tellp() == 0) {
    return;
  }

  auto n = write_to_queue(write_queue_, stream);

  if (n < 0) {
    bytes_failed_to_write_ += static_cast<u64>(static_cast<std::streamoff>(stream.tellp()));
  } else {
    bytes_written_ += n;
  }
}

void PrometheusPublisher::Writer::write(std::string_view prefix, std::string_view labels, std::string_view suffix)
{
  u32 len = prefix.size() + labels.size() + suffix.size();

  int res = eq_write(&write_queue_, len);

  if (res == -EINVAL) {
    throw std::runtime_error("tried to write more than queue length");
  } else if (res == -ENOSPC) {
    bytes_failed_to_write_ += len;
    return;
  }
  assert(res >= 0);

  char *cur = write_queue_.data + res;

  memcpy(cur, prefix.data(), prefix.size());
  memcpy(cur + prefix.size(), labels.data(), labels.size());
  memcpy(cur + prefix.size() + labels.size(), suffix.data(), suffix.size());

  bytes_written_ += len;
}

void PrometheusPublisher::Writer::flush()
{
  write_queue_.finish_write_batch();
  write_queue_.start_write_batch();
}

void PrometheusPublisher::Writer::write_internal_stats(
    InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const
{
  BigItemsDroppedStats stats;
  stats.labels.shard = std::to_string(shard);
  stats.labels.module = module;
  stats.metrics.prometheus_big_items_dropped = stats_.big_items_dropped;
  encoder.write_internal_stats(stats, time_ns);
}

} // namespace reducer
