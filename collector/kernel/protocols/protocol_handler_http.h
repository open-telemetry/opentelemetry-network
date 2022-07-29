/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "../tcp_data_handler.h"
#include "protocol_handler_base.h"

class ProtocolHandler_HTTP : public ProtocolHandlerBase {
private:
  u64 request_timestamp_;
  u64 response_timestamp_;
  u64 client_stream_position_;
  u64 server_stream_position_;
  enum class CLIENT_STATE { START, STOP, END } client_state_;
  enum class SERVER_STATE { START, VERSION_AND_CODE, STOP, END } server_state_;

  int http_version_major_;
  int http_version_minor_;
  int http_code_;

  void transition(CLIENT_STATE new_state);
  void transition(SERVER_STATE new_state);

public:
  ProtocolHandler_HTTP(TCPDataHandler *data_handler, const tcp_control_key_t &key, u32 pid);
  virtual ~ProtocolHandler_HTTP();

  virtual void handle_server_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len) override;
  virtual void handle_client_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len) override;
};
