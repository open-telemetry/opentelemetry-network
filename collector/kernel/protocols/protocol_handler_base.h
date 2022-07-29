/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include "platform/platform.h"

#include "collector/kernel/bpf_src/tcp-processor/tcp_processor.h"

class TCPDataHandler;

class ProtocolHandlerBase {
public:
  typedef std::shared_ptr<ProtocolHandlerBase> ptr_type;

  ProtocolHandlerBase(TCPDataHandler *data_handler, const tcp_control_key_t &key, u32 pid);
  virtual ~ProtocolHandlerBase() {}

  virtual void handle_server_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len) = 0;
  virtual void handle_client_data(u64 tstamp, u64 offset, STREAM_TYPE stream_type, const u8 *data, size_t data_len) = 0;

  virtual void set_upgrade(ptr_type upgrade) { upgrade_ = upgrade; }
  virtual ptr_type get_upgrade() { return upgrade_; }
  inline TCPDataHandler *data_handler() { return data_handler_; }
  inline tcp_control_key_t control_key() const { return key_; }
  inline u32 pid() const { return pid_; }

private:
  TCPDataHandler *data_handler_;
  tcp_control_key_t key_;
  ptr_type upgrade_;
  u32 pid_;
};
