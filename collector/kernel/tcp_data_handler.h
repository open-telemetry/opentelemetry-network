/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <linux/bpf.h>

#include <map>
#include <memory>

#include <generated/ebpf_net/ingest/writer.h>
#include <platform/platform.h>
#include <util/logger.h>
#include <uv.h>

#include "collector/agent_log.h"
#include "collector/kernel/bpf_src/tcp-processor/tcp_processor.h"
#include "collector/kernel/perf_reader.h"
#include "protocols/protocol_handler_base.h"

/* forward declarations */
struct render_bpf_bpf;
class ProbeHandler;

class TCPDataHandler {
public:
  /**
   * c'tor
   */
  TCPDataHandler(
      uv_loop_t &loop,
      ProbeHandler &probe_handler,
      struct render_bpf_bpf *skel,
      ::ebpf_net::ingest::Writer &writer,
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
  inline ::ebpf_net::ingest::Writer &writer() { return writer_; }

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
  struct render_bpf_bpf *skel_;
  ::ebpf_net::ingest::Writer &writer_;
  PerfContainer &container_;
  u64 lost_record_total_count_ = 0;
  std::map<tcp_control_key_t, std::shared_ptr<ProtocolHandlerBase>, tcp_control_key_t_comparator> protocol_handlers_;
  int tcp_control_map_fd_;
  logging::Logger &log_;
};
