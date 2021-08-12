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
