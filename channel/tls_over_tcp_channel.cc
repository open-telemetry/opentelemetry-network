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

#include <channel/tls_over_tcp_channel.h>

#include <util/log.h>
#include <util/log_formatters.h>

#include <uv.h>

#include <iostream>

namespace channel {

TlsOverTcpChannel::TlsOverTcpChannel(
    uv_loop_t &loop,
    std::string addr,
    std::string port,
    TLSChannel::Credentials &creds,
    std::string server_hostname,
    std::optional<config::HttpProxyConfig> proxy)
    : tcp_callbacks_(*this),
      tcp_(loop, std::move(addr), std::move(port), std::move(proxy)),
      tls_(tcp_, creds, std::move(server_hostname))
{}

TlsOverTcpChannel::~TlsOverTcpChannel() {}

void TlsOverTcpChannel::connect(Callbacks &callbacks)
{
  LOG::debug("TlsOverTcpChannel::connect()");
  callbacks_ = &callbacks;
  tcp_.connect(tcp_callbacks_);
}

std::error_code TlsOverTcpChannel::send(const u8 *data, int data_len)
{
  return tls_.send(data, data_len);
}

void TlsOverTcpChannel::close()
{
  tls_.close();
  tcp_.close();
}

std::error_code TlsOverTcpChannel::flush()
{
  if (auto error = tls_.flush()) {
    return error;
  }
  return tcp_.flush();
}

/**************************************
 * TCP CALLBACKS
 **************************************/

TlsOverTcpChannel::TcpCallbacks::TcpCallbacks(TlsOverTcpChannel &parent_channel) : parent_channel_(parent_channel) {}

u32 TlsOverTcpChannel::TcpCallbacks::received_data(const u8 *data, int data_len)
{
  return parent_channel_.tls_.received_data(data, data_len);
}

void TlsOverTcpChannel::TcpCallbacks::on_error(int err)
{
  LOG::error("TCP error {}", static_cast<std::errc>(-err));

  parent_channel_.callbacks_->on_error(err);
}

void TlsOverTcpChannel::TcpCallbacks::on_closed()
{
  parent_channel_.callbacks_->on_closed();
}

void TlsOverTcpChannel::TcpCallbacks::on_connect()
{
  LOG::info("TCP connected");

  /* ok TCP is connected. connect TLS */
  parent_channel_.tls_.connect(*parent_channel_.callbacks_);
}

} /* namespace channel */
