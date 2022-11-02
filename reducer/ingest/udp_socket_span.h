/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/ingest/flow_updater.h>

#include <generated/ebpf_net/ingest/parsed_message.h>
#include <generated/ebpf_net/ingest/span_base.h>

#include <common/client_server_type.h>
#include <platform/platform.h>
#include <util/ip_address.h>

#include <array>
#include <optional>

namespace reducer::ingest {

class UdpSocketSpan : public ::ebpf_net::ingest::UdpSocketSpanBase {
public:
  ~UdpSocketSpan();

  /** handlers */
  void udp_new_socket(::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_new_socket *msg);
  void udp_stats_addr_unchanged(
      ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_addr_unchanged *msg);
  void udp_stats_addr_changed_v4(
      ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_addr_changed_v4 *msg);
  void udp_stats_addr_changed_v6(
      ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_addr_changed_v6 *msg);
  void
  dns_response_dep_b(::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__dns_response_dep_b *msg);
  void dns_response(::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__dns_response *msg);
  void dns_timeout(::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__dns_timeout *msg);
  void udp_stats_drops_changed(
      ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_drops_changed *msg);

private:
  // Local endpoint address.
  IPv6Address local_addr_;
  // Local port.
  u16 local_port_{0};

  struct AddrPort {
    IPv6Address addr;
    u16 port;
  };

  std::array<AddrPort, 2> remote_ip_;

  // Flag indicating whether UDP metrics should be ignored.
  bool ignore_udp_{false};
  // Flag indicating whether this UDP socket has received DNS stats.
  bool is_dns_{false};

  // Flow updaters, [0]:TX, [1]:RX
  std::array<std::optional<FlowUpdater>, 2> flow_updater_;

  /**
   * Updates the flow object given address change.
   */
  void update_handle(::ebpf_net::ingest::weak_refs::udp_socket span_ref, u8 is_rx);

  /**
   * Updates statistics assuming the flow object is already initialized.
   *
   * Checks if the handle is valid.
   */
  void update_udp_stats(u64 timestamp, u8 is_rx, u32 addr_changed, u32 packets, u32 bytes, u32 drops);

  // Same as above, but for dns stats.
  void update_dns_stats(
      ::ebpf_net::ingest::weak_refs::udp_socket span_ref,
      u64 timestamp,
      const CLIENT_SERVER_TYPE client_server,
      const ::ebpf_net::metrics::dns_metrics_point &metrics,
      bool is_timeout);
};

} // namespace reducer::ingest
