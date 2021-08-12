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

#include <bcc/BPFTable.h>
#include <bcc/bpf_module.h>
#include <linux/bpf.h>

#include <map>
#include <memory>

#include <uv.h>

#include <generated/flowmill/ingest/writer.h>
#include <platform/platform.h>
#include <util/logger.h>

#include "collector/agent_log.h"
#include "collector/kernel/bpf_src/tcp-processor/tcp_processor.h"
#include "collector/kernel/perf_reader.h"
#include "protocols/protocol_handler_base.h"

class TCPDataHandler {
public:
  /**
   * c'tor
   */
  TCPDataHandler(
      uv_loop_t &loop,
      ebpf::BPFModule &bpf_module,
      ::flowmill::ingest::Writer &writer,
      PerfContainer &container,
      logging::Logger &log);

  /**
   * d'tor
   */
  virtual ~TCPDataHandler();

  // Data processing entrypoint
  void process(
      size_t idx,
      u64 tstamp,
      u64 sk,
      u32 pid,
      u32 length,
      u64 offset,
      STREAM_TYPE stream_type,
      CLIENT_SERVER_TYPE client_server);

  void handle_close_socket(u64 sk);

  // Output
  inline ::flowmill::ingest::Writer &writer() { return writer_; }

  // tcp kernel->userland throttling control backchannel
  void enable_stream(const tcp_control_key_t &key, STREAM_TYPE stream_type, bool enable);
  void enable_stream(const tcp_control_key_t &key, bool enable);
  void update_stream_start(const tcp_control_key_t &key, STREAM_TYPE stream_type, u64 start);

  // create a new protocol handler, possibly for upgrade
  ProtocolHandlerBase::ptr_type create_protocol_handler(int protocol, const tcp_control_key_t &key, u32 pid);

protected:
  void upgrade_protocol_handler(ProtocolHandlerBase::ptr_type new_handler, ProtocolHandlerBase::ptr_type original_handler);

  struct tcp_control_key_t_comparator {
    bool operator()(const tcp_control_key_t &a, const tcp_control_key_t &b) const;
  };

protected:
  uv_loop_t &loop_;
  ebpf::BPFModule &bpf_module_;
  ::flowmill::ingest::Writer &writer_;
  PerfContainer &container_;
  u64 lost_record_total_count_ = 0;
  std::map<tcp_control_key_t, std::shared_ptr<ProtocolHandlerBase>, tcp_control_key_t_comparator> protocol_handlers_;
  ebpf::TableDesc tcp_control_desc_;
  logging::Logger &log_;
};
