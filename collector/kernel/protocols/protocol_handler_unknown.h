/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "../tcp_data_handler.h"
#include "protocol_handler_base.h"

class ProtocolHandler_UNKNOWN : public ProtocolHandlerBase {
public:
  ProtocolHandler_UNKNOWN(TCPDataHandler *data_handler, const tcp_control_key_t &key, u32 pid);
  virtual ~ProtocolHandler_UNKNOWN();

  virtual void handle_server_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len) override;
  virtual void handle_client_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len) override;

protected:
  typedef TCP_PROTOCOL_DETECT_RESULT (ProtocolHandler_UNKNOWN::*PROTOCOL_DETECT_FUNC)(
      u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len);

  TCP_PROTOCOL_DETECT_RESULT detect_http(u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len);

private:
  u32 available_protocols_;
};
