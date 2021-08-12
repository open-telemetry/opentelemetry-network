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

#include "protocol_handler_http.h"
#include "platform/platform.h"
#include "protocol_tools.h"
#include "spdlog/common.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "util/log.h"

ProtocolHandler_HTTP::ProtocolHandler_HTTP(TCPDataHandler *data_handler, const tcp_control_key_t &key, u32 pid)
    : ProtocolHandlerBase(data_handler, key, pid),
      request_timestamp_(0),
      response_timestamp_(0),
      client_stream_position_(0),
      server_stream_position_(0),
      client_state_(CLIENT_STATE::START),
      server_state_(SERVER_STATE::START),
      http_version_major_(0),
      http_version_minor_(0),
      http_code_(0)

{
  LOG::debug_in(AgentLogKind::PROTOCOL, "ProtocolHandler_HTTP::$ctor(sk={:x})", control_key().sk);
}

ProtocolHandler_HTTP::~ProtocolHandler_HTTP()
{
  LOG::debug_in(AgentLogKind::PROTOCOL, "ProtocolHandler_HTTP::$dtor(sk={:x})", control_key().sk);
}

void ProtocolHandler_HTTP::transition(CLIENT_STATE new_state)
{
  client_state_ = new_state;
}

void ProtocolHandler_HTTP::transition(SERVER_STATE new_state)
{
  server_state_ = new_state;
}

void ProtocolHandler_HTTP::handle_server_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len)
{
  LOG::debug_in(
      AgentLogKind::PROTOCOL,
      "ProtocolHandler_HTTP::handle_server_data(sk={:x}): tstamp={}, offset={}, is_recv={}, data_len={}, data:\n{}",
      tstamp,
      control_key().sk,
      offset,
      stream_type_to_string(stream_type),
      data_len,
      std::string_view((const char *)data, data_len));

  // HTTP server-side state machine
  // TODO: Add buffering if state processing can't proceed without more bytes

  if (offset > server_stream_position_) {
    LOG::debug_in(
        AgentLogKind::PROTOCOL,
        "ProtocolHandler_HTTP::handle_server_data: server stream skipped {} bytes from {} to {}",
        offset - server_stream_position_,
        server_stream_position_,
        offset);
  } else if (offset > server_stream_position_) {
    LOG::debug_in(
        AgentLogKind::PROTOCOL,
        "ProtocolHandler_HTTP::handle_server_data: server stream went backward {} bytes from {} to {}",
        server_stream_position_ - offset,
        server_stream_position_,
        offset);
  }

  while (server_state_ != SERVER_STATE::END) {

    switch (server_state_) {

    // Beginning of stream
    case SERVER_STATE::START:
      response_timestamp_ = tstamp;
      transition(SERVER_STATE::VERSION_AND_CODE);
      break;

    // Wait for HTTP version and code
    case SERVER_STATE::VERSION_AND_CODE: {
      if (data_len < 12) {
        // TODO: Eventually, buffer and just return
        transition(SERVER_STATE::STOP);
        break;
      }

      // Pattern match against HTTP/x.y
      if (!prefix_check(data, 12, "HTTP/") || data[6] != '.' || data[8] != ' ') {
        transition(SERVER_STATE::STOP);
        break;
      }

      http_version_major_ = char_to_number(data[5]);
      http_version_minor_ = char_to_number(data[7]);

      // Check for invalid HTTP version
      if (http_version_major_ == -1 || http_version_minor_ == -1) {
        transition(SERVER_STATE::STOP);
        break;
      }

      // Convert HTTP response code to integer
      int http_code_2 = char_to_number(data[9]);
      int http_code_1 = char_to_number(data[10]);
      int http_code_0 = char_to_number(data[11]);

      // Ensure each digit of HTTP response code is valid
      if (http_code_2 == -1 || http_code_1 == -1 || http_code_0 == -1) {
        transition(SERVER_STATE::STOP);
        break;
      }

      http_code_ = (u16)(http_code_2 * 100 + http_code_1 * 10 + http_code_0);

      // Submit the http response code, and latency
      u64 latency = response_timestamp_ - request_timestamp_;
      enum CLIENT_SERVER_TYPE client_server = (stream_type == ST_SEND) ? SC_SERVER : SC_CLIENT;

      LOG::debug_in(
          AgentLogKind::HTTP,
          "handle_http_response: tstamp={}, sk={:x}, pid={}, code={}, "
          "latency_ns={}, client_server={}",
          response_timestamp_,
          control_key().sk,
          pid(),
          http_code_,
          latency,
          client_server_type_to_string(client_server));

      data_handler()->writer().http_response_tstamp(
          response_timestamp_, control_key().sk, pid(), http_code_, latency, (u8)client_server);

      transition(SERVER_STATE::STOP);
    } break;
    // Stop server side processing
    case SERVER_STATE::STOP:
      data_handler()->enable_stream(control_key(), stream_type, false);
      transition(SERVER_STATE::END);
      break;

    // End of server side processing
    case SERVER_STATE::END:
    default:
      LOG::error("ProtocolHandler_HTTP::handle_server_data: invalid server state");
      return;
    }
  }

  server_stream_position_ += data_len;
}

void ProtocolHandler_HTTP::handle_client_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len)
{
  LOG::debug_in(
      AgentLogKind::PROTOCOL,
      "ProtocolHandler_HTTP::handle_client_data(sk={:x}): tstamp={}, offset={}, is_recv={}, data_len={}, data:\n{}",
      tstamp,
      control_key().sk,
      offset,
      stream_type_to_string(stream_type),
      data_len,
      std::string_view((const char *)data, data_len));

  // HTTP client-side state machine
  // TODO: Add buffering if state processing can't proceed without more bytes

  if (offset > client_stream_position_) {
    LOG::debug_in(
        AgentLogKind::PROTOCOL,
        "ProtocolHandler_HTTP::handle_client_data: client stream skipped {} bytes from {} to {}",
        offset - client_stream_position_,
        client_stream_position_,
        offset);
  } else if (offset > client_stream_position_) {
    LOG::debug_in(
        AgentLogKind::PROTOCOL,
        "ProtocolHandler_HTTP::handle_client_data: client stream went backward {} bytes from {} to {}",
        client_stream_position_ - offset,
        client_stream_position_,
        offset);
  }

  while (client_state_ != CLIENT_STATE::END) {

    switch (client_state_) {

    // Beginning of stream
    case CLIENT_STATE::START:
      request_timestamp_ = tstamp;
      transition(CLIENT_STATE::STOP);
      break;

    // Stop client side processing
    case CLIENT_STATE::STOP:
      data_handler()->enable_stream(control_key(), stream_type, false);
      transition(CLIENT_STATE::END);
      break;

    // End of client side processing
    case CLIENT_STATE::END:
    default:
      LOG::error("ProtocolHandler_HTTP::handle_client_data: invalid client state");
      return;
    }
  }

  client_stream_position_ += data_len;
}
