// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "protocol_handler_base.h"
#include "platform/platform.h"
#include "spdlog/common.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "util/log.h"

ProtocolHandlerBase::ProtocolHandlerBase(TCPDataHandler *data_handler, const tcp_control_key_t &key, u32 pid)
    : data_handler_(data_handler), key_(key), pid_(pid)
{}
