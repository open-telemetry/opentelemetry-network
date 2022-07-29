// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "protocol_handler_unknown.h"
#include "absl/base/internal/endian.h"
#include "platform/platform.h"
#include "protocol_tools.h"
#include "spdlog/common.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "util/log.h"
#include <string_view>

ProtocolHandler_UNKNOWN::ProtocolHandler_UNKNOWN(TCPDataHandler *data_handler, const tcp_control_key_t &key, u32 pid)
    : ProtocolHandlerBase(data_handler, key, pid)
{
  LOG::debug_in(AgentLogKind::PROTOCOL, "ProtocolHandler_UNKNOWN::$ctor(sk={:x})", control_key().sk);

  available_protocols_ = TCP_PROTOCOL_MASK;
}

ProtocolHandler_UNKNOWN::~ProtocolHandler_UNKNOWN()
{
  LOG::debug_in(AgentLogKind::PROTOCOL, "ProtocolHandler_UNKNOWN::$dtor(sk={:x})", control_key().sk);
}

void ProtocolHandler_UNKNOWN::handle_server_data(
    u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len)
{
  LOG::debug_in(
      AgentLogKind::PROTOCOL,
      "ProtocolHandler_UNKNOWN::handle_server_data(sk={:x}): tstamp={}, offset={}, is_recv={}, data_len={}, data={}",
      tstamp,
      control_key().sk,
      offset,
      stream_type_to_string(stream_type),
      data_len,
      std::string_view((const char *)data, data_len));

  // Currently protocol detection only happens with client data
  // XXX: when we implement other protocols, we may need to validate a server response before upgrading, potentially
  available_protocols_ = 0;
  data_handler()->enable_stream(control_key(), false);
}

TCP_PROTOCOL_DETECT_RESULT
ProtocolHandler_UNKNOWN::detect_http(u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len)
{
  TCP_PROTOCOL_DETECT_RESULT res = TPD_UNKNOWN;

  // XXX: This is not complete. Eventually we should implement buffering if data_len is less than 4, since
  // XXX: in theory 3 bytes could show up, followed by the rest of the data, and since TCP is a stream protocol.

  if (data_len < 4)
    return res;

  switch (U32IDX(data, 0)) {
  case U32CC("GET "):
  case U32CC("PUT "):
    return TPD_SUCCESS;
  case U32CC("HEAD"):
  case U32CC("POST"):
    if (data_len >= 5) {
      res = data[4] == ' ' ? TPD_SUCCESS : TPD_FAILED;
    }
    break;
  case U32CC("DELE"):
    if (data_len >= 7) {
      res = (data[4] == 'T' && data[5] == 'E' && data[6] == ' ') ? TPD_SUCCESS : TPD_FAILED;
    }
    break;
  case U32CC("CONN"):
    if (data_len >= 8) {
      res = U32IDX(data, 1) == U32CC("ECT ") ? TPD_SUCCESS : TPD_FAILED;
    }
    break;
  case U32CC("OPTI"):
    if (data_len >= 8) {
      res = U32IDX(data, 1) == U32CC("ONS ") ? TPD_SUCCESS : TPD_FAILED;
    }
    break;
  case U32CC("TRAC"):
    if (data_len >= 6) {
      res = (data[4] == 'E' && data[5] == ' ') ? TPD_SUCCESS : TPD_FAILED;
    }
    break;
  case U32CC("PATC"):
    if (data_len >= 6) {
      res = (data[4] == 'H' && data[5] == ' ') ? TPD_SUCCESS : TPD_FAILED;
    }
    break;
  default:
    res = TPD_FAILED;
    break;
  }

  return res;
}

void ProtocolHandler_UNKNOWN::handle_client_data(
    u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len)
{
  LOG::debug_in(
      AgentLogKind::PROTOCOL,
      "ProtocolHandler_UNKNOWN::handle_client_data(sk={:x}): tstamp={}, offset={}, is_recv={}, data_len={}, data:\n{}",
      tstamp,
      control_key().sk,
      offset,
      stream_type_to_string(stream_type),
      data_len,
      std::string_view((const char *)data, data_len));

  // All the client-side protocol detection routines we want to run
  std::list<std::pair<int, PROTOCOL_DETECT_FUNC>> client_protocols = {
      {TCPPROTO_HTTP, &ProtocolHandler_UNKNOWN::detect_http},
      //{TCPPROTO_MYSQL, &ProtocolHandler_UNKNOWN::detect_mysql},
  };

  // Run the available detection routines
  for (auto it : client_protocols) {
    int tcpproto = it.first;
    PROTOCOL_DETECT_FUNC detect = it.second;

    if (available_protocols_ & TCP_PROTOCOL_BIT(tcpproto)) {
      // Run the detection if it's still qualified
      auto res = (this->*detect)(offset, stream_type, data, data_len);
      if (res == TPD_SUCCESS) {
        // If we -definitely- detected a particular protocol, upgrade to its handler
        ProtocolHandlerBase::ptr_type upgrade = data_handler()->create_protocol_handler(tcpproto, control_key(), pid());
        set_upgrade(upgrade);
        return;
      } else if (res == TPD_FAILED) {
        // Remove any that are disqualified
        available_protocols_ &= ~TCP_PROTOCOL_BIT(tcpproto);
      }
      // If res==TPD_UNKNOWN, then continue checking with the next protocol
    }
  }

  // When all protocols are done just stop listening completely
  if (available_protocols_ == 0) {
    data_handler()->enable_stream(control_key(), false);
  }
}
