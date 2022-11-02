// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "prometheus_handler.h"

#include <util/log.h>

#include <charconv>
#include <optional>
#include <string>

namespace reducer {

namespace {

static constexpr size_t chunk_buffer_size = (1 << 20);

static constexpr char const *response_content_type = "text/plain;version=0.0.4";

static const auto default_timeout = std::chrono::seconds(5);

// Returns number of bytes read and an indication whether there is more
// content in the queue to be read.
std::pair<u32, bool> read_from_queue(ElementQueue &queue, char *buffer, u32 buffer_size)
{
  u32 len = 0;
  bool more = false;

  queue.start_read_batch();

  while ((more = (queue.peek() > 0)) == true) {
    if ((len + queue.peek()) > buffer_size) {
      break; /* we've filled the buffer */
    }

    char *elem_buf = nullptr;
    int elem_len = queue.read(elem_buf);
    assert(elem_len >= 0);

    memcpy(buffer + len, elem_buf, elem_len);
    len += elem_len;
  }

  queue.finish_read_batch();

  return std::make_pair(len, more);
}

} // namespace

PrometheusHandler::PrometheusHandler(
    std::vector<ElementQueueStoragePtr> const &queues, std::optional<u64> scrape_size_limit_bytes)
    : queue_mutex_(queues.size()), scrape_size_limit_bytes_(scrape_size_limit_bytes)
{
  queues_.reserve(queues.size());
  for (auto &queue_storage : queues) {
    queues_.emplace_back(queue_storage);
  }
}

std::optional<PrometheusHandler::timeout_t> PrometheusHandler::get_scrape_timeout(CivetServer *server, mg_connection *conn)
{
  char const *hdr = server->getHeader(conn, "X-Prometheus-Scrape-Timeout-Seconds");

  if (hdr == nullptr) {
    // not specified
    return std::nullopt;
  }

  double seconds = atof(hdr);

  if (seconds <= 0) {
    // invalid value
    return std::nullopt;
  }

  return std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(seconds * 1000));
}

void PrometheusHandler::write_content_from_queues(CivetServer *server, mg_connection *conn)
{
  // max duration of the request
  timeout_t timeout = default_timeout;

  if (auto scrape_timeout = get_scrape_timeout(server, conn); scrape_timeout) {
    timeout = *scrape_timeout;

    // leave some time for network
    //
    if (timeout > 1s) {
      // trim half a second
      timeout -= 500ms;
    } else {
      // halve the value
      timeout /= 2;
    }
  }

  // absolute time at which to finish sending
  u64 timeout_ns = monotonic() + std::chrono::nanoseconds(timeout).count();

  char chunk_buffer[chunk_buffer_size];
  u32 chunk_len = 0;
  u64 bytes_sent = 0;
  bool error = false;

  auto send_chunk = [&]() {
    if (auto nsent = mg_send_chunk(conn, chunk_buffer, chunk_len); nsent > 0) {
      bytes_sent += nsent;
      chunk_len = 0;
    } else {
      error = true;
    }
  };

  std::lock_guard<std::mutex> lock(handler_mutex_);

  // start the response
  if (mg_send_http_ok(conn, response_content_type, -1) == -1) {
    ++num_failed_scrapes_;
    return;
  }

  u64 scrape_limit = scrape_size_limit_bytes_.value_or(std::numeric_limits<u64>::max());

  // This will get set when a read from a queue can't be performed because
  // the scrape size limit would be exceeded. In that case we will complete
  // the request even if a read some other queue would succeed.
  bool scrape_size_limited = false;

  for (size_t i = 0; i < queues_.size(); ++i) {
    if (error) {
      break;
    }

    if (monotonic() >= timeout_ns) {
      break;
    }

    if (scrape_size_limited) {
      break;
    }

    if (bytes_sent >= scrape_limit) {
      break;
    }

    auto &queue = queues_[next_queue_];
    next_queue_ = (next_queue_ + 1) % queues_.size();

    bool more;
    u32 repeat = 0;
    do {
      u64 scrape_remaining = scrape_limit - bytes_sent;
      u64 chunk_remaining = chunk_buffer_size - chunk_len;

      u32 nread;
      std::tie(nread, more) = read_from_queue(queue, chunk_buffer + chunk_len, std::min(chunk_remaining, scrape_remaining));

      if (nread > 0) {
        // something was read and added to the chunk buffer
        chunk_len += nread;
      }

      if (more) {
        if (chunk_remaining < scrape_remaining) {
          // There was more to read but didn't fit in the chunk buffer.
          // We will flush the chunk buffer and try again.
          send_chunk();
        } else {
          // There was more to read but it would go over the scrape size limit.
          // Setting this flag will conclude this request.
          scrape_size_limited = true;
          break;
        }
      }

      // If there is more to read it is because there was not enough space in
      // the chunk buffer. We will repeat the read -- only once, otherwise we
      // can get into a race with the producer.
    } while (more && !error && (bytes_sent < scrape_limit) && (repeat++ < 2));
  }

  if (!error && (chunk_len > 0)) {
    // send what is left to send
    send_chunk();
  }

  // terminating chunk
  mg_send_chunk(conn, nullptr, 0);

  bytes_served_ += bytes_sent;
  if (error) {
    ++num_failed_scrapes_;
  }
}

void PrometheusHandler::write_content_from_queue(CivetServer *server, mg_connection *conn, size_t queue_num)
{
  // max duration of the request
  timeout_t timeout = default_timeout;

  if (auto scrape_timeout = get_scrape_timeout(server, conn); scrape_timeout) {
    timeout = *scrape_timeout;

    // leave some time for network
    //
    if (timeout > 1s) {
      // trim half a second
      timeout -= 500ms;
    } else {
      // halve the value
      timeout /= 2;
    }
  }

  // absolute time at which to finish sending
  u64 timeout_ns = monotonic() + std::chrono::nanoseconds(timeout).count();

  char chunk_buffer[chunk_buffer_size];
  u64 bytes_sent = 0;
  bool error = false;

  std::lock_guard<std::mutex> lock(queue_mutex_[queue_num]);

  auto &queue = queues_[queue_num];

  // start the response
  if (mg_send_http_ok(conn, response_content_type, -1) == -1) {
    ++num_failed_scrapes_;
    return;
  }

  while (monotonic() < timeout_ns) {
    u64 read_size = chunk_buffer_size;

    if (scrape_size_limit_bytes_.has_value()) {
      // Limit this read so the total number of bytes returned doesn't go
      // over the scrape limit.
      if (*scrape_size_limit_bytes_ > bytes_sent) {
        read_size = std::min(chunk_buffer_size, *scrape_size_limit_bytes_ - bytes_sent);
      } else {
        // Already went over the limit. This can happen because mg_send_chunk
        // is writing additional data (chunk headers, etc.)
        break;
      }
    }

    auto [nread, more] = read_from_queue(queue, chunk_buffer, read_size);

    if (nread == 0) {
      // nothing more to read from the queue
      // assert(!more)
      break;
    }

    // NOTE: mg_send_chunk doesn't do partial writes
    auto nsent = mg_send_chunk(conn, chunk_buffer, nread);

    if (nsent > 0) {
      bytes_sent += nsent;
    } else {
      // zero signifies connection closed -- we count that also as failed write
      error = true;
      break;
    }

    if (!more) {
      // Exhausted the queue. We don't want to try to read again as that can get
      // us into a race with the producer.
      break;
    }
  }

  // terminating chunk
  mg_send_chunk(conn, nullptr, 0);

  bytes_served_ += bytes_sent;
  if (error) {
    ++num_failed_scrapes_;
  }
}

////////////////////////////////////////////////////////////////////////////////

PortRangePromHandler::PortRangePromHandler(
    std::vector<ElementQueueStoragePtr> const &queues,
    std::optional<u64> scrape_size_limit_bytes,
    std::optional<u16> extra_ports_base)
    : PrometheusHandler(queues, scrape_size_limit_bytes), extra_ports_base_(extra_ports_base)
{}

int PortRangePromHandler::choose_queue(CivetServer *server, mg_connection *conn)
{
  std::string queue_query_parameter;

  // First, try to get the queue number from the URI query parameters.
  // If it exists, we'll use it.
  if (server->getParam(conn, "queue", queue_query_parameter, 0) == true) {
    int queue_num;

    if (auto [_, ec] = std::from_chars(
            queue_query_parameter.data(), queue_query_parameter.data() + queue_query_parameter.size(), queue_num);
        ec == std::errc()) {
      // got a queue parameter. Validate the requested queue is not negative
      if (queue_num < 0) {
        LOG::error("PrometheusHandler: negative queue number in query parameter: {}", queue_query_parameter);
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
        return -1;
      }

      // non-negative queue number. can return it as chosen queue.
      return queue_num;

    } else {
      LOG::error("PrometheusHandler: invalid queue number in query parameter: {}", queue_query_parameter);
      mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
      return -1;
    }
  }

  // Didn't have an explicit query parameter. Try to extract the queue number from
  // the port number in the Host header.
  std::string_view host;
  std::optional<int> port;
  if (char const *hdr = server->getHeader(conn, "Host"); hdr != nullptr) {
    host = hdr;
  } else {
    LOG::error("PrometheusHandler: missing required Host header");
    mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
    return -1;
  }

  if (auto sep = host.rfind(':'); sep != std::string_view::npos) {
    std::string_view port_txt = host.substr(sep + 1);
    int port_num;

    if (auto [_, ec] = std::from_chars(port_txt.data(), port_txt.data() + port_txt.size(), port_num); ec == std::errc()) {
      port = port_num;
    } else {
      LOG::error("PrometheusHandler: invalid port number in Host header: {}", port_txt);
      mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
      return -1;
    }
  } else {
    LOG::warn("PrometheusHandler: port number not specified in Host header");
  }

  std::size_t requested_queue = 0; // default

  // NOTE: only one listening address can be selected using a command-line
  // parameter, so it is safe to assume that the server has only one
  // listening port.
  int listening_port = server->getListeningPorts().at(0);

  if (port.has_value() && (*port != listening_port)) {
    int extra_ports_base = extra_ports_base_.value_or(listening_port + 1);

    if (*port >= extra_ports_base) {
      requested_queue = *port - extra_ports_base + 1;
    } else {
      LOG::error("PrometheusHandler: invalid port number");
      mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
      return -1;
    }
  }

  return requested_queue;
}

bool PortRangePromHandler::handleGet(CivetServer *server, mg_connection *conn)
{
  auto requested_queue = choose_queue(server, conn);
  if (requested_queue < 0) {
    // there was a failure, error already printed and error code sent.
    return true;
  }

  if ((size_t)requested_queue >= num_queues()) {
    LOG::error("PrometheusHandler: invalid queue number");
    mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n\r\n");
    return true;
  }

  write_content_from_queue(server, conn, requested_queue);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

SinglePortPromHandler::SinglePortPromHandler(
    std::vector<ElementQueueStoragePtr> const &queues, std::optional<u64> scrape_size_limit_bytes)
    : PrometheusHandler(queues, scrape_size_limit_bytes)
{}

bool SinglePortPromHandler::handleGet(CivetServer *server, mg_connection *conn)
{
  write_content_from_queues(server, conn);

  return true;
}

} // namespace reducer
