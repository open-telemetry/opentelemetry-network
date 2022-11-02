/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/element_queue_cpp.h>

#include <CivetServer.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>

namespace reducer {

class PrometheusHandler : public CivetHandler {
public:
  PrometheusHandler(std::vector<ElementQueueStoragePtr> const &queues, std::optional<u64> scrape_size_limit_bytes);

  // Number of bytes of content served by this handler.
  u64 bytes_served() const { return bytes_served_; };

  // Number of times writing a response has failed.
  u64 num_failed_scrapes() const { return num_failed_scrapes_; }

  // Number of queues this handler is reading from.
  size_t num_queues() const { return queues_.size(); }

protected:
  using timeout_t = std::chrono::milliseconds;

  // Returns the scrape timeout that is passed in the
  // X-Prometheus-Scrape-Timeout-Seconds header.
  std::optional<timeout_t> get_scrape_timeout(CivetServer *server, mg_connection *conn);

  // Writes the content response by reading from all queues.
  void write_content_from_queues(CivetServer *server, mg_connection *conn);

  // Writes the content response by reading from the specified queue.
  void write_content_from_queue(CivetServer *server, mg_connection *conn, size_t queue_num);

private:
  // Queues from which the content will be read from.
  std::vector<ElementQueue> queues_;
  // A mutex for each individual queue.
  std::vector<std::mutex> queue_mutex_;
  // This mutex needs to be locked when reading from multiple queues.
  std::mutex handler_mutex_;

  // Maximum number of bytes to return in one response.
  std::optional<u64> scrape_size_limit_bytes_;

  // Number of bytes served.
  std::atomic<u64> bytes_served_{0};
  // Number of times writing a response has failed.
  std::atomic<u64> num_failed_scrapes_{0};

  // Next queue to be scraped.
  //
  // This is used in `write_content_from_queues()`, where reading from queues
  // can be interrupted by timeout or scrape size limit. In such case, on the
  // next scrape request it is important to start on the next queue in line.
  size_t next_queue_{0};
};

// Prometheus handler for scraping on a port range.
//
// Each scrape port is associated with a single queue.
// The port is obtained from the `Host` HTTP header.
//
class PortRangePromHandler : public PrometheusHandler {
public:
  PortRangePromHandler(
      std::vector<ElementQueueStoragePtr> const &queues,
      std::optional<u64> scrape_size_limit_bytes,
      std::optional<u16> extra_ports_base = std::nullopt);

private:
  std::optional<u16> extra_ports_base_;
  bool handleGet(CivetServer *server, mg_connection *conn) override;

  // Decides which queue this request should service, and sends an error
  // response if there is no valid queue communicated in the request.
  // @returns a positive queue index, on success. Negative on failure.
  int choose_queue(CivetServer *server, mg_connection *conn);
};

// Prometheus handler for scraping on a single port.
//
class SinglePortPromHandler : public PrometheusHandler {
public:
  SinglePortPromHandler(std::vector<ElementQueueStoragePtr> const &queues, std::optional<u64> scrape_size_limit_bytes);

private:
  bool handleGet(CivetServer *server, mg_connection *conn) override;
};

} /* namespace reducer */
