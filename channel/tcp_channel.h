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

#pragma once

#include <channel/callbacks.h>
#include <channel/network_channel.h>
#include <config/http_proxy_config.h>
#include <platform/platform.h>

#include <uv.h>

namespace channel {

struct buffer_t {
  char *base;
  u32 offset;
  u32 len;
};

/**
 * A TCP channel
 *
 * Errors for on_error callback:
 *   -EPROTO: handler threw exception
 *   -EOVERFLOW: overflow occupies entire buffer SERVER_CONN_BUFFER_SIZE and not
 *     handled by handler
 *   libuv errors.
 */
class TCPChannel : public NetworkChannel {
public:
  static constexpr u32 rx_buffer_size = (64 * 1024);

  struct send_buffer_t {
    uv_write_t req; /* must be first */
    u32 len;
    u64 data[0];
  };

  /**
   * c'tor -- leaves socket ready for accept()
   */
  TCPChannel(uv_loop_t &loop);

  /**
   * c'tor -- leaves socket ready for connect()
   */
  TCPChannel(uv_loop_t &loop, std::string addr, std::string port, std::optional<config::HttpProxyConfig> proxy = {});

  /**
   * d'tor
   */
  virtual ~TCPChannel();

  /**
   * Connects to an endpoint
   * @param callbacks: the callbacks to use during this connection
   * @param addr: ip address or hostname
   * @param port: string holding port number
   */
  void connect(Callbacks &callbacks) override;

  /**
   * Accepts a connection
   *
   * @param callbacks: the callbacks to use during this connection
   * @param listener: the listening socket to accept on
   */
  void accept(Callbacks &callbacks, uv_tcp_t *listener);

  /**
   * Opens a TCP connection from the file descriptor.
   */
  void open_fd(Callbacks &callbacks, uv_os_sock_t fd);

  /**
   * closes the channel. Callbacks::on_close will be called
   */
  void close() override;

  /**
   * Closes the channel, and does not try to reinitilize.
   */
  void close_permanently();

  /**
   * @see Channel::send
   */
  std::error_code send(const u8 *data, int data_len) override;

  /**
   * Allocates a send buffer capable of holding @size bytes.
   *
   * It is the responsibility of the caller to call send() with the buffer.
   */
  struct send_buffer_t *allocate_send_buffer(u32 size);

  /**
   * Sends the given TCPChannel::send_buffer_t allocated with
   *   allocate_send_buffer().
   */
  std::error_code send(struct send_buffer_t *send_buffer);

  /**
   * Returns the address (in binary format) that this channel is connected to,
   * if available. `nullptr` otherwise.
   */
  in_addr_t const *connected_address() const override;

  bool is_open() const override { return connected_; }

private:
  static void conn_read_alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
  static void conn_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
  static void conn_close_cb(uv_handle_t *handle);
  static void conn_close_and_reinit_cb(uv_handle_t *handle);
  static void conn_connect_cb(uv_connect_t *req, int status);
  static void conn_write_cb(uv_write_t *req, int status);

  void close_internal(const uv_close_cb close_cb);

  /**
   * Inits the tcp handle (conn_) and buffers
   */
  void reinit(uv_loop_t *loop);

  /**
   * Completes socket configuration and starts reading
   */
  void start_processing();

  Callbacks *callbacks_ = nullptr;
  std::string addr_;
  std::string port_;
  std::optional<config::HttpProxyConfig> proxy_;
  std::unique_ptr<Callbacks> callback_wrapper_;

  uv_tcp_t conn_;
  uv_connect_t connect_req_;

  u64 rx_buffer_[(rx_buffer_size + 7) / 8];

  /* number of bytes currently in the rx_buffer */
  u32 rx_len_;

  bool allocated_ = false;

  bool connected_address_available_ = false;
  bool connected_ = false;
  in_addr_t connected_address_ = 0;
};

} /* namespace channel */
