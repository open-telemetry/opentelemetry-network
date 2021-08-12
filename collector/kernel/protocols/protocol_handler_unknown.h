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
