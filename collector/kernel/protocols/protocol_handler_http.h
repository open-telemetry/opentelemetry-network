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
