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

#include "protocol_handler_base.h"
#include "platform/platform.h"
#include "spdlog/common.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "util/log.h"

ProtocolHandlerBase::ProtocolHandlerBase(TCPDataHandler *data_handler, const tcp_control_key_t &key, u32 pid)
    : data_handler_(data_handler), key_(key), pid_(pid)
{}
