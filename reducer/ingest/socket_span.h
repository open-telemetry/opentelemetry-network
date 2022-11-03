/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/ingest/flow_updater.h>

#include <reducer/dns_cache.h>

#include <generated/ebpf_net/ingest/parsed_message.h>
#include <generated/ebpf_net/ingest/span_base.h>

#include <platform/platform.h>
#include <util/ip_address.h>

#include <optional>

namespace reducer::ingest {

class SocketSpan : public ::ebpf_net::ingest::SocketSpanBase {
public:
  ~SocketSpan();

  void new_sock_info(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__new_sock_info *msg);
  void set_state_ipv4(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__set_state_ipv4 *msg);
  void set_state_ipv6(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__set_state_ipv6 *msg);
  void socket_stats(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__socket_stats *msg);
  void nat_remapping(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__nat_remapping *msg);
  void syn_timeout(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__syn_timeout *msg);
  void tcp_reset(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__tcp_reset *msg);
  void http_response(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__http_response *msg);

private:
  IPv6Address local_addr_;
  u16 local_port_ = 0;
  IPv6Address remote_addr_;
  u16 remote_port_ = 0;
  u32 is_connector_ = 0; /* 0: unknown, 1: connector, 2: listener */
  u32 new_sockets_ = 0;

  // Original value or the remote address, before any translations.
  IPv6Address original_remote_addr_;
  // DNS record for the remote address, if any.
  std::optional<dns::dns_record> remote_dns_;

  std::optional<FlowUpdater> flow_updater_;

  void get_flow(::ebpf_net::ingest::weak_refs::socket span_ref);
};

} // namespace reducer::ingest
