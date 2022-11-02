/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "prometheus_handler.h"
#include "publisher.h"

#include <util/element_queue_cpp.h>

#include <iosfwd>
#include <memory>
#include <optional>
#include <vector>

class CivetServer;

namespace reducer {

class PrometheusHandler;

// Class used to publish time-series data for Prometheus to scrape.
//
// Prometheus is scraping using HTTP, so objects of this class instantiate
// a HTTP server to serve requests.
//
// As publishing to Prometheus is usually done from multiple threads, this
// class allows for it by keeping a queue for each publishing thread.
//
// To write time-series data the |make_writer| method is first used.
// It creates an object that exposes various writing functions.
// Each such writer can only be used from a single thread.
//
class PrometheusPublisher : public Publisher {
public:
  class Writer;

  enum HandlerType {
    // Handler for scraping on a single port. Multiple writers are allowed
    // for the single specified Prometheus scrape port.
    SINGLE_PORT,
    // Handler for scraping on a port range.
    // Number of writers must match number of Prometheus scrape ports.
    PORT_RANGE,
  };

  // Constructs the object and starts the HTTP server.
  //
  // \param handler_type type of handler to use (see PrometheusHandler class)
  // \param num_writer_threads the number of threads that will be writing
  // \param http_bind_addr IP address on which to listen for HTTP requests
  // \param http_num_threads number of HTTP server threads
  // \param scrape_size_limit_bytes max size of one scrape response
  //
  PrometheusPublisher(
      HandlerType handler_type,
      size_t num_writer_threads,
      std::string_view http_bind_addr,
      int http_num_threads = 1,
      std::optional<u64> scrape_size_limit_bytes = std::nullopt);

  virtual ~PrometheusPublisher();

  // Creates a writer object for the specified thread.
  virtual WriterPtr make_writer(size_t thread_num) override;

  // Number of bytes of content scraped by prometheus.
  u64 bytes_served() const;

  // Number of times scraping by prometheus has failed.
  u64 num_failed_scrapes() const;

  // Gets this writer's stats encoded for TSDB output.
  virtual void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const override;

private:
  // Queue buffer for sending to prometheus handler.
  std::vector<ElementQueueStoragePtr> queue_buffers_;
  // Handler for HTTP requests.
  std::unique_ptr<PrometheusHandler> http_handler_;
  // HTTP server for prometheus to query.
  std::unique_ptr<CivetServer> http_server_;
};

// Writer for PrometheusPublisher.
//
class PrometheusPublisher::Writer : public Publisher::Writer {
public:
  Writer(ElementQueueStoragePtr const &queue);

  Writer(Writer const &) = delete;
  Writer(Writer &&) = default;

  ~Writer();

  // Writes provided stream's content.
  void write(std::stringstream &ss) override;

  // Writes prefix, followed by labels, finished with suffix.
  void write(std::string_view prefix, std::string_view labels, std::string_view suffix) override;

  // Finishes this and starts a new batch.
  void flush() override;

  // Number of bytes successfully written.
  u64 bytes_written() const override { return bytes_written_; }

  // Number of bytes that were not written.
  u64 bytes_failed_to_write() const override { return bytes_failed_to_write_; }

  // Gets this writer's stats encoded for TSDB output.
  void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const override;

private:
  ElementQueue write_queue_;
  u64 bytes_written_{0};
  u64 bytes_failed_to_write_{0};

  struct Stats {
    std::size_t big_items_dropped = 0;
  };

  Stats stats_ = {};
};

} // namespace reducer
