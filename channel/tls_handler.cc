// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/tls_handler.h>

#include <uv.h>

namespace channel {

TLSHandler::TLSHandler(
    uv_loop_t &loop,
    std::string addr,
    std::string port,
    std::string agent_key,
    std::string agent_crt,
    std::string server_hostname,
    std::optional<config::HttpProxyConfig> proxy)
    : creds_(std::move(agent_key), std::move(agent_crt)),
      tls_channel_(loop, std::move(addr), std::move(port), creds_, std::move(server_hostname), std::move(proxy))
{}

void TLSHandler::connect(Callbacks &callbacks)
{
  tls_channel_.connect(callbacks);
}

std::error_code TLSHandler::send(const u8 *data, int data_len)
{
  return tls_channel_.send(data, data_len);
}

void TLSHandler::close()
{
  tls_channel_.close();
}

std::error_code TLSHandler::flush()
{
  return tls_channel_.flush();
}

in_addr_t const *TLSHandler::connected_address() const
{
  return tls_channel_.get_tcp_channel().connected_address();
}

} // namespace channel
