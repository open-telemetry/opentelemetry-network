//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <channel/tcp_channel.h>

#include <channel/component.h>
#include <platform/platform.h>
#include <util/defer.h>
#include <util/error_handling.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/uv_helpers.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h> //for disabling Nagle's
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdexcept>

#define INVALID_FD -1

namespace channel {

static constexpr std::string_view CONNECTED_DISCONNECTED[2] = {"disconnected", "connected"};
/**
 * Callback passed to uv_read_start that allocates memory for the read callback
 */
void TCPChannel::conn_read_alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
  uv_tcp_t *tcp_conn = (uv_tcp_t *)handle;
  TCPChannel *conn = (TCPChannel *)tcp_conn->data;

  if (conn->allocated_) {
    buf->base = nullptr;
    return;
  }

  /* assign the free buffer, reserving bytes for overflow */
  buf->base = (char *)conn->rx_buffer_ + conn->rx_len_;
  buf->len = TCPChannel::rx_buffer_size - conn->rx_len_;
  conn->allocated_ = true;
}

/**
 * Read callback
 */
void TCPChannel::conn_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
  uv_tcp_t *tcp_conn = (uv_tcp_t *)stream;
  TCPChannel *conn = (TCPChannel *)tcp_conn->data;

  if (nread < 0) {
    /* oh-oh, read error */
    /* free buffer if it is valid */
    if (buf->base) {
      conn->allocated_ = false;
    }

    /* need to notify */
    conn->connected_ = false;
    conn->callbacks_->on_error(nread);
    return;
  }

  /* "merge" the read data into the buffer */
  conn->rx_len_ += nread;
  conn->allocated_ = false;

  /* read all complete messages from buffer */
  try {
    u32 res = conn->callbacks_->received_data((u8 *)conn->rx_buffer_, conn->rx_len_);

    if (res > 0) {
      ASSUME(res <= conn->rx_len_);
      conn->rx_len_ -= res;
      memmove(conn->rx_buffer_, (u8 *)conn->rx_buffer_ + res, conn->rx_len_);
    }
  } catch (const std::exception &e) {
    LOG::error("TCPChannel: error handling received data: '{}'", e.what());
    conn->connected_ = false;
    conn->callbacks_->on_error(-EPROTO);
    return;
  }

  /* check that we don't exceed the buffer size */
  if (conn->rx_len_ == TCPChannel::rx_buffer_size) {
    conn->connected_ = false;
    conn->callbacks_->on_error(-EOVERFLOW);
    return;
  }
}

void TCPChannel::conn_close_cb(uv_handle_t *handle)
{
  TCPChannel *conn = (TCPChannel *)handle->data;
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}: calling on_closed()", __func__);
  conn->callbacks_->on_closed();
}

void TCPChannel::conn_close_and_reinit_cb(uv_handle_t *handle)
{
  TCPChannel *conn = (TCPChannel *)handle->data;
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}: calling on_closed()", __func__);
  conn->callbacks_->on_closed();

  /* re-initialize so we can reuse the handle */
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}: calling reinit()", __func__);
  conn->reinit(handle->loop);
}

void TCPChannel::conn_connect_cb(uv_connect_t *req, int status)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  auto tcp = (TCPChannel *)req->handle->data;

  if (status < 0) {
    LOG::trace_in(channel::Component::tcp, "TCPChannel::{}(): error {}", __func__, static_cast<std::errc>(-status));
    /* error occurred */
    tcp->connected_ = false;
    tcp->callbacks_->on_error(status);
    return;
  }

  tcp->connected_ = true;

  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}(): calling callback::on_connect()", __func__);
  tcp->callbacks_->on_connect();

  tcp->start_processing();
}

void TCPChannel::conn_write_cb(uv_write_t *req, int status)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  auto tcp = (TCPChannel *)req->handle->data;

  if (status < 0) {
    /* no need to notify if close() was called, otherwise -- notify */
    if (!uv_is_closing((uv_handle_t *)req->handle)) {
      LOG::trace_in(channel::Component::tcp, "TCPChannel::{}: connection not closing, calling close on handle()", __func__);
      tcp->connected_ = false;
      tcp->callbacks_->on_error(status);
    }
  }

  /* assumes req is the first field in send_buffer_t */
  free(req);
}

TCPChannel::TCPChannel(uv_loop_t &loop)
{
  reinit(&loop);
}

TCPChannel::TCPChannel(uv_loop_t &loop, std::string addr, std::string port) : addr_(std::move(addr)), port_(std::move(port))
{
  reinit(&loop);
}

TCPChannel::~TCPChannel()
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}: connection not closing, calling close on handle()", __func__);
  ASSUME(uv_is_closing((uv_handle_t *)&conn_));
}

void TCPChannel::connect(Callbacks &callbacks)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);

  callbacks_ = callback_wrapper_ ? callback_wrapper_.get() : &callbacks;

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *res = nullptr;
  LOG::debug("TCPChannel::{}: Connecting to intake @ {}:{}", __func__, addr_, port_);
  int status = getaddrinfo(addr_.c_str(), port_.c_str(), &hints, &res);

  if (status != 0) {
    LOG::critical("getaddrinfo failed: {} - calling abort", static_cast<std::errc>(status));
    // TODO: gracefully handle getaddrinfo errors
    std::abort();
  }

  Defer free_addrinfo([&res] { freeaddrinfo(res); });

  if (res->ai_addr->sa_family == AF_INET) {
    struct sockaddr_in *sa = (struct sockaddr_in *)(res->ai_addr);
    connected_address_available_ = true;
    connected_address_ = sa->sin_addr.s_addr;
  }

  if (auto const error = ::uv_tcp_connect(&connect_req_, &conn_, res->ai_addr, &conn_connect_cb)) {
    LOG::error("TCPChannel::{}: failed to establish connection to {}:{}: {}", __func__, addr_, port_, uv_error_t{error});
    callbacks_->on_error(error);
  }
}

void TCPChannel::accept(Callbacks &callbacks, uv_tcp_t *listener)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  callbacks_ = &callbacks;

  if (auto const error = ::uv_accept(reinterpret_cast<uv_stream_t *>(listener), reinterpret_cast<uv_stream_t *>(&conn_))) {
    // this is guaranteed to succeed when called from the connection callback:
    // http://docs.libuv.org/en/v1.x/stream.html#c.uv_accept
    LOG::error("TCPChannel::{}: failed to accept incoming connections: {}", __func__, uv_error_t{error});
    CHECK_UV(error); // TODO: verify that users of `accept` properly handle `on_error`
    callbacks_->on_error(error);
  } else {
    connected_ = true;
    start_processing();
  }
}

void TCPChannel::open_fd(Callbacks &callbacks, const uv_os_sock_t fd)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  callbacks_ = &callbacks;

  if (auto const error = ::uv_tcp_open(&conn_, fd)) {
    LOG::error("TCPChannel::{}: failed to open existing file descriptor as a TCP handle: {}", __func__, uv_error_t{error});
    CHECK_UV(error); // TODO: verify that users of `open_fd` properly handle `on_error`
    callbacks_->on_error(error);
  } else {
    connected_ = true;
    start_processing();
  }
}

void TCPChannel::close()
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  close_internal(&conn_close_and_reinit_cb);
}

void TCPChannel::close_permanently()
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  close_internal(&conn_close_cb);
}

std::error_code TCPChannel::send(const u8 *data, int data_len)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}(len:{})", __func__, data_len);
  auto send_buffer = allocate_send_buffer(data_len);
  if (!send_buffer) {
    // TODO: gracefully handle out-of-memory errors
    std::abort();
    return std::make_error_code(std::errc::not_enough_memory);
  }

  memcpy(send_buffer->data, data, data_len);
  send_buffer->len = data_len;

  return send(send_buffer);
}

struct TCPChannel::send_buffer_t *TCPChannel::allocate_send_buffer(u32 size)
{
  u32 mem_size = ((sizeof(struct send_buffer_t) + 7) & ~7) + size;
  struct send_buffer_t *ret = (struct send_buffer_t *)malloc(mem_size);
  if (ret == nullptr) {
    LOG::critical("Failed to allocate send buffer of size {} mem_size {}", size, mem_size);
    return nullptr;
  }

  // clear memory to ensure we don't exfiltrate uninitialized data
  memset(ret, 0, mem_size);

  return ret;
}

std::error_code TCPChannel::send(struct send_buffer_t *send_buffer)
{
  uv_buf_t uv_buf = {.base = (char *)send_buffer->data, .len = send_buffer->len};

  if (auto const error = ::uv_write(&send_buffer->req, reinterpret_cast<uv_stream_t *>(&conn_), &uv_buf, 1, conn_write_cb)) {
    LOG::error(
        "TCPChannel::{}: failed to write {} bytes into {} channel: {}",
        __func__,
        send_buffer->len,
        CONNECTED_DISCONNECTED[connected_],
        uv_error_t{error});

    callbacks_->on_error(error);
    return {error, libuv_category()};
  }

  return {};
}

void TCPChannel::reinit(uv_loop_t *loop)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  /* reinit RX buffers */
  rx_len_ = 0;
  allocated_ = false;

  /* re-init handle */
  CHECK_UV(uv_tcp_init(loop, &conn_));
  conn_.data = this;
}

void TCPChannel::start_processing()
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);

  if (auto const error = ::uv_tcp_nodelay(&conn_, true)) {
    LOG::error("TCPChannel::{}: failed to disable Nagle's algorithm: {}", __func__, uv_error_t{error});
    // this error is not critical, we may continue
  }

  if (auto const error = ::uv_read_start(reinterpret_cast<uv_stream_t *>(&conn_), &conn_read_alloc_cb, conn_read_cb)) {
    LOG::error("TCPChannel::{}: failed to start read loop on channel: {}", __func__, uv_error_t{error});

    callbacks_->on_error(error);
  }
}

in_addr_t const *channel::TCPChannel::connected_address() const
{
  std::array<in_addr_t const *, 2> choice = {nullptr, &connected_address_};
  return choice[connected_address_available_];
}

void channel::TCPChannel::close_internal(const uv_close_cb close_cb)
{
  LOG::trace_in(channel::Component::tcp, "TCPChannel::{}()", __func__);
  connected_address_available_ = false;
  connected_ = false;
  if (!uv_is_closing((uv_handle_t *)&conn_)) {
    LOG::trace_in(channel::Component::tcp, "TCPChannel::{}: connection not closing, calling close on handle()", __func__);
    uv_close((uv_handle_t *)&conn_, close_cb);
  }
}

} // namespace channel
