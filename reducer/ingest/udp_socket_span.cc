// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "udp_socket_span.h"

#include <reducer/ingest/component.h>
#include <reducer/ingest/shared_state.h>

#include <generated/ebpf_net/ingest/modifiers.h>
#include <generated/ebpf_net/metrics.h>

#include <util/log.h>

#include <cstring>

namespace reducer::ingest {

namespace {

// Version in which the IPv6 address bug was fixed in the kernel collector.
// That bug was causing spurious addresses to be reported for UDP sockets.
// Temporary pipeline fix was to ignore UDP metrics.
static constexpr VersionInfo UDP_ADDRESS_FIX_VERSION(0, 8, 2082);

} // namespace

UdpSocketSpan::~UdpSocketSpan() {}

void UdpSocketSpan::udp_new_socket(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_new_socket *msg)
{
  auto *conn = local_connection()->ingest_connection();
  AgentSpan &agent = conn->agent().impl();

  u32 *laddr = reinterpret_cast<u32 *>(msg->laddr);

  if ((laddr[0] | laddr[1] | (laddr[2] ^ 0xffff0000)) == 0) {
    /// IPv6-encoded IPv4 address
    local_addr_ = IPv4Address::from(laddr[3]).to_ipv6();
  } else {
    local_addr_ = IPv6Address::from(msg->laddr);
  }

  local_port_ = msg->lport;

  // get a reference to the process
  if (auto process_ref = conn->get_process(msg->pid); process_ref.valid()) {
    span_ref.modify().process(process_ref.get());
  } else {
    local_logger().udp_socket_failed_getting_process_reference(msg->pid);
  }

  if (agent.version() < UDP_ADDRESS_FIX_VERSION) {
    ignore_udp_ = true;
  }
}

void UdpSocketSpan::udp_stats_addr_unchanged(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_addr_unchanged *msg)
{
  if (msg->is_rx > 1) {
    return; /* TODO: log */
  }

  if (!ignore_udp_) {
    update_udp_stats(timestamp, msg->is_rx, 0 /* addr_changed */, msg->packets, msg->bytes, 0 /* drops */);
  }
}

void UdpSocketSpan::udp_stats_addr_changed_v4(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_addr_changed_v4 *msg)
{
  if (msg->is_rx > 1) {
    return; /* TODO: log */
  }

  auto remote_addr = IPv4Address::from(msg->raddr).to_ipv6();
  auto remote_port = msg->rport;

  remote_ip_[msg->is_rx] = AddrPort{remote_addr, remote_port};

  if (!ignore_udp_ || is_dns_) {
    // if UDP metrics are disabled, we only need the updater for DNS metrics
    update_handle(span_ref, msg->is_rx);
  }

  if (!ignore_udp_) {
    update_udp_stats(timestamp, msg->is_rx, 1 /* addr_changed */, msg->packets, msg->bytes, 0 /* drops */);
  }
}

void UdpSocketSpan::udp_stats_addr_changed_v6(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_addr_changed_v6 *msg)
{
  if (msg->is_rx > 1) {
    return; /* TODO: log */
  }

  auto remote_addr = IPv6Address::from(msg->raddr);
  auto remote_port = msg->rport;

  remote_ip_[msg->is_rx] = AddrPort{remote_addr, remote_port};

  if (!ignore_udp_ || is_dns_) {
    // if UDP metrics are disabled, we only need the updater for DNS metrics
    update_handle(span_ref, msg->is_rx);
  }

  if (!ignore_udp_) {
    update_udp_stats(timestamp, msg->is_rx, 1 /* addr_changed */, msg->packets, msg->bytes, 0 /* drops */);
  }
}

void UdpSocketSpan::dns_response_dep_b(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__dns_response_dep_b *msg)
{
  // for older agents just put dns responses on the client side for now
  jsrv_ingest__dns_response newmsg;
  newmsg._rpc_id = jsrv_ingest__dns_response__rpc_id;
  newmsg.total_dn_len = msg->total_dn_len;
  newmsg.sk_id = msg->sk_id;
  newmsg.domain_name = msg->domain_name;
  newmsg.ipv4_addrs = msg->ipv4_addrs;
  newmsg.ipv6_addrs = msg->ipv6_addrs;
  newmsg.latency_ns = msg->latency_ns;

  // for older agents, just key client_server off of port 53 (like old is_dns_rx)
  newmsg.client_server = (u8)((local_port_ == kPortDNS) ? SC_SERVER : SC_CLIENT);

  // pass new version of message through
  dns_response(span_ref, timestamp, &newmsg);
}

void UdpSocketSpan::dns_response(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__dns_response *msg)
{
  auto *conn = local_connection()->ingest_connection();
  AgentSpan &agent = conn->agent().impl();

  // copy because we're not sure of message alignment
  int num_ipv4_addrs = msg->ipv4_addrs.len / sizeof(in_addr);
  in_addr ipv4_addrs[num_ipv4_addrs];
  memcpy(ipv4_addrs, msg->ipv4_addrs.buf, sizeof(in_addr) * num_ipv4_addrs);

  int num_ipv6_addrs = msg->ipv6_addrs.len / sizeof(in6_addr);
  in6_addr ipv6_addrs[num_ipv6_addrs];
  memcpy(ipv6_addrs, msg->ipv6_addrs.buf, sizeof(in6_addr) * num_ipv6_addrs);

  std::string_view domain_name{msg->domain_name.buf, msg->domain_name.len};

  LOG::debug_in(
      Component::dns,
      "UdpSocketSpan::dns_response: timestamp={}, sk_id={}, domain_name={}, "
      "num_ipv4_addrs={}, num_ipv6_addrs={}, latency_ns={} ",
      timestamp,
      msg->sk_id,
      domain_name,
      num_ipv4_addrs,
      num_ipv6_addrs,
      msg->latency_ns);

  agent.map_ips_to_domain(ipv4_addrs, num_ipv4_addrs, ipv6_addrs, num_ipv6_addrs, domain_name);

  // Update DNS stats.
  const enum CLIENT_SERVER_TYPE client_server = (const enum CLIENT_SERVER_TYPE)msg->client_server;
  update_dns_stats(
      span_ref,
      timestamp,
      client_server,
      ::ebpf_net::metrics::dns_metrics_point{
          .active_sockets = 1,
          .requests_a = (num_ipv4_addrs > 0),
          .requests_aaaa = (num_ipv6_addrs > 0),
          .responses = 1,
          .timeouts = 0,
          .sum_total_time_ns = (client_server == SC_CLIENT) ? msg->latency_ns : 0, // client latency contributes to total time
          .sum_processing_time_ns =
              (client_server == SC_SERVER) ? msg->latency_ns : 0 // server latency contributes to processing time
      },
      false);
}

void UdpSocketSpan::dns_timeout(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__dns_timeout *msg)
{
  LOG::debug_in(
      Component::dns,
      "UdpSocketSpan::dns_timeout: timestamp={}, sk_id={}, "
      "domain_name={}, timeout_ns={} ",
      timestamp,
      msg->sk_id,
      std::string_view(msg->domain_name.buf, msg->domain_name.len),
      msg->timeout_ns);

  // Update DNS stats.
  update_dns_stats(
      span_ref,
      timestamp,
      SC_CLIENT, // dns stats for timeouts are always client side
      ::ebpf_net::metrics::dns_metrics_point{
          .active_sockets = 1,
          .requests_a = 0,
          .requests_aaaa = 0,
          .responses = 0,
          .timeouts = 1,
          .sum_total_time_ns = 0,     // timeout duration does not contribute here
          .sum_processing_time_ns = 0 // timeout duration does not contribute here
      },
      true);
}

void UdpSocketSpan::udp_stats_drops_changed(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref, u64 timestamp, jsrv_ingest__udp_stats_drops_changed *msg)
{
  LOG::debug_in(Component::udp_drops, "UDP Drops: {}", msg->drops);

  // Always assume that drops are for received, not transmit.
  update_udp_stats(timestamp, true /* is_rx */, 0 /* addr_changed */, 0 /* packets */, 0 /* bytes */, msg->drops);
}

void UdpSocketSpan::update_handle(::ebpf_net::ingest::weak_refs::udp_socket span_ref, u8 is_rx)
{
  auto *conn = local_connection()->ingest_connection();
  auto agent = conn->agent();
  auto &addr_map = global_private_to_public_address_map();

  auto local_addr = local_addr_;

  auto remote_addr = remote_ip_[is_rx].addr;
  auto remote_port = remote_ip_[is_rx].port;

  // first, try to translate DNS. Do this before private-public mapping, as
  // the address that DNS would have returned is the socket's remote address,
  // not the mapped addresss.
  auto remote_dns = agent.impl().find_dns_for_ip(remote_addr).value_or(std::string_view());

  // Using the globally-shared private-to-public address map, translate both
  // local and remote addresses to their public equivalent, if such mapping
  // exists.
  //
  if (auto public_addr = addr_map.get(local_addr); public_addr) {
    local_addr = *public_addr;
  }
  if (auto public_addr = addr_map.get(remote_addr); public_addr) {
    remote_addr = *public_addr;
  }

  if (local_addr.is_localhost() || (local_addr.is_ipv4() && (local_addr.to_ipv4()->is_localhost()))) {
    LOG::trace_in(
        Component::udp,
        "UdpSocketSpan::update_handle: ignoring localhost traffic:"
        " {}:{} -> {}:{}",
        local_addr,
        local_port_,
        remote_addr,
        remote_port);
    flow_updater_[is_rx].reset();
    return;
  }

  if ((local_addr == remote_addr) && !agent.impl().is_host_address(local_addr)) {
    LOG::trace_in(
        Component::udp,
        "UdpSocketSpan::update_handle: ignoring local traffic:"
        " {}:{} -> {}:{}",
        local_addr,
        local_port_,
        remote_addr,
        remote_port);
    flow_updater_[is_rx].reset();
    return;
  }

  LOG::trace_in(
      Component::udp, "UdpSocketSpan::update_handle: {}:{} -> {}:{}", local_addr, local_port_, remote_addr, remote_port);

  flow_updater_[is_rx] = FlowUpdater(
      span_ref.process(),
      agent,
      local_addr,
      local_port_,
      remote_addr,
      remote_port,
      0,
      dns::dns_record{short_string_behavior::no_truncate, remote_dns});
}

void UdpSocketSpan::update_udp_stats(u64 timestamp, u8 is_rx, u32 addr_changed, u32 packets, u32 bytes, u32 drops)
{
  ::ebpf_net::metrics::udp_metrics_point stats = {
      .active_sockets = 1,
      .addr_changes = addr_changed,
      .packets = packets,
      .bytes = bytes,
      .drops = drops,
  };

  auto &flow_updater = flow_updater_[is_rx];
  if (flow_updater) {
    flow_updater->udp_update(timestamp, stats, is_rx);
  }
}

void UdpSocketSpan::update_dns_stats(
    ::ebpf_net::ingest::weak_refs::udp_socket span_ref,
    const u64 timestamp,
    const CLIENT_SERVER_TYPE client_server,
    const ::ebpf_net::metrics::dns_metrics_point &metrics,
    bool is_timeout)
{
  // we -received- the dns response if we are the client
  u8 is_rx = (client_server == SC_CLIENT) ? 1 : 0;

  // use server side for timeouts since we may not have RX address
  if (is_timeout)
    is_rx = 0;

  is_dns_ = true;

  // if UDP stats are being ignored, the updater will not be created until the
  // first time we get UDP stats
  if (!flow_updater_[is_rx].has_value()) {
    update_handle(span_ref, is_rx);
  }

  auto &flow_updater = flow_updater_[is_rx];
  if (flow_updater) {
    flow_updater->dns_update(timestamp, metrics, client_server);
  }
}

} // namespace reducer::ingest
