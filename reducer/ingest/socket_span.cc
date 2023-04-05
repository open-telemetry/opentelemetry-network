// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "socket_span.h"

#include <reducer/ingest/component.h>
#include <reducer/ingest/shared_state.h>

#include <generated/ebpf_net/ingest/modifiers.h>

#include <util/ip_address.h>
#include <util/log.h>

#include <arpa/inet.h>
#include <config.h>

namespace reducer::ingest {

SocketSpan::~SocketSpan()
{
  LOG::trace_in(
      Component::socket, "SocketSpan::~SocketSpan: {}:{} -> {}:{}", local_addr_, local_port_, remote_addr_, remote_port_);
}

void SocketSpan::new_sock_info(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__new_sock_info *msg)
{
  auto *conn = local_connection()->ingest_connection();

  // get a reference to the process
  if (auto process_ref = conn->get_process(msg->pid); process_ref.valid()) {
    span_ref.modify().process(process_ref.get());
  } else {
    local_logger().tcp_socket_failed_getting_process_reference(msg->pid);
  }
}

void SocketSpan::set_state_ipv4(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__set_state_ipv4 *msg)
{
  auto *conn = local_connection()->ingest_connection();
  AgentSpan &agent = conn->agent().impl();

  auto local_addr = IPv4Address::from(msg->src);
  auto remote_addr = IPv4Address::from(msg->dest);
  auto local_port = msg->sport;
  auto remote_port = msg->dport;

  LOG::trace_in(
      Component::socket,
      "SocketSpan::set_state_ipv4: sk={}, src={}:{}, dst={}:{}, tx_rx={}",
      msg->sk,
      local_addr,
      local_port,
      remote_addr,
      remote_port,
      msg->tx_rx);

  auto local_addr6 = local_addr.to_ipv6();
  auto remote_addr6 = remote_addr.to_ipv6();

  if (flow_updater_ && flow_updater_->valid()) {
    if (std::tie(local_addr6, local_port, remote_addr6, remote_port) !=
        std::tie(local_addr_, local_port_, remote_addr_, remote_port_)) {
      local_logger().socket_address_already_assigned();
    }
    return;
  }

  /* save the IP addresses as IPv6-mapped IPv4 */
  local_addr_ = local_addr6;
  remote_addr_ = remote_addr6;

  /* save ports */
  local_port_ = local_port;
  remote_port_ = remote_port;

  /* connector/acceptor */
  is_connector_ = msg->tx_rx;

  if (agent.is_socket_steady_state()) {
    new_sockets_ = 1;
  }

  /* save the original remote address, before any address translations */
  original_remote_addr_ = remote_addr_;

  /* enrich with recent DNS query on remote endpoint, if available */
  if (!remote_dns_.has_value()) {
    if (auto dns = agent.find_dns_for_ip(original_remote_addr_)) {
      remote_dns_.emplace(short_string_behavior::no_truncate, *dns);
    }
  }

  get_flow(span_ref);
}

void SocketSpan::set_state_ipv6(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__set_state_ipv6 *msg)
{
  auto *conn = local_connection()->ingest_connection();
  AgentSpan &agent = conn->agent().impl();

  auto local_addr = IPv6Address::from(msg->src);
  auto remote_addr = IPv6Address::from(msg->dest);
  auto local_port = msg->sport;
  auto remote_port = msg->dport;

  LOG::trace_in(
      Component::socket,
      "SocketSpan::set_state_ipv6: sk={}, src={}:{}, dst={}:{}, tx_rx={}",
      msg->sk,
      local_addr,
      local_port,
      remote_addr,
      remote_port,
      msg->tx_rx);

  if (flow_updater_ && flow_updater_->valid()) {
    if (std::tie(local_addr, local_port, remote_addr, remote_port) !=
        std::tie(local_addr_, local_port_, remote_addr_, remote_port_)) {
      local_logger().socket_address_already_assigned();
    }
    return;
  }

  /* save the IPv6 addresses */
  local_addr_ = local_addr;
  remote_addr_ = remote_addr;

  /* save ports */
  local_port_ = local_port;
  remote_port_ = remote_port;

  /* connector/acceptor */
  is_connector_ = msg->tx_rx;

  if (agent.is_socket_steady_state()) {
    new_sockets_ = 1;
  }

  /* save the original remote address, before any address translations */
  original_remote_addr_ = remote_addr_;

  /* enrich with recent DNS query on remote endpoint, if available */
  if (!remote_dns_.has_value()) {
    if (auto dns = agent.find_dns_for_ip(original_remote_addr_)) {
      remote_dns_.emplace(short_string_behavior::no_truncate, *dns);
    }
  }

  get_flow(span_ref);
}

void SocketSpan::socket_stats(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__socket_stats *msg)
{
  auto *conn = local_connection()->ingest_connection();
  AgentSpan &agent = conn->agent().impl();

  LOG::trace_in(
      Component::socket,
      "SocketSpan::socket_stats: sk={}, {}:{} -> {}:{}, diff_bytes={},"
      " diff_delivered={}, diff_retrans={}, max_srtt={}, is_rx={}",
      msg->sk,
      local_addr_,
      local_port_,
      remote_addr_,
      remote_port_,
      msg->diff_bytes,
      msg->diff_delivered,
      msg->diff_retrans,
      msg->max_srtt,
      msg->is_rx);

  if (msg->is_rx > 1)
    return; /* ignore is_rx not in {0,1} */

  // Correct small negative differences
  u64 diff_bytes = msg->diff_bytes;
  if (unlikely(diff_bytes > static_cast<u64>(1ULL << 56ULL))) {
    // should not happen (after agent ~0.8.2390)
    LOG::error("truncating negative diff_bytes: was 0x{:x}", diff_bytes);
    diff_bytes = 0;
  }
  u32 diff_retrans = msg->diff_retrans;
  if (unlikely(diff_retrans > static_cast<u32>(1UL << 28UL))) {
    // should not happen (after agent ~0.8.2390)
    LOG::error("truncating negative diff_retrans: was 0x{:x}", diff_retrans);
    diff_retrans = 0;
  }
  u32 diff_delivered = msg->diff_delivered;
  if (unlikely(diff_delivered > static_cast<u32>(1UL << 28UL))) {
    // should not happen (after agent ~0.8.2390)
    LOG::error("truncating negative diff_delivered: was 0x{:x}", diff_delivered);
    diff_delivered = 0;
  }

  ::ebpf_net::metrics::tcp_metrics_point stats = {
      .active_sockets = 1,
      .sum_retrans = diff_retrans,
      .sum_bytes = diff_bytes,
      .sum_srtt = msg->max_srtt,
      .sum_delivered = diff_delivered,
      .active_rtts = 1,
      .syn_timeouts = 0,
      .new_sockets = new_sockets_,
      .tcp_resets = 0,
  };
  new_sockets_ = 0;

  if (!remote_dns_.has_value()) {
    // retry getting remote DNS name
    if (auto dns = agent.find_dns_for_ip(original_remote_addr_)) {
      remote_dns_.emplace(short_string_behavior::no_truncate, *dns);
      get_flow(span_ref);
    }
  }

  if (flow_updater_) {
    flow_updater_->tcp_update(timestamp, stats, msg->is_rx);
  }
}

void SocketSpan::syn_timeout(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__syn_timeout *msg)
{
  ::ebpf_net::metrics::tcp_metrics_point stats = {
      .active_sockets = 1,
      .sum_retrans = 0,
      .sum_bytes = 0,
      .sum_srtt = 0,
      .sum_delivered = 0,
      .active_rtts = 0,
      .syn_timeouts = 1,
      .new_sockets = 0,
      .tcp_resets = 0,
  };

  if (flow_updater_) {
    flow_updater_->tcp_update(
        timestamp,
        stats,
        /* timeouts are always tx: is_rx is 0 */ 0);
  }
}

void SocketSpan::tcp_reset(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__tcp_reset *msg)
{
  ::ebpf_net::metrics::tcp_metrics_point stats = {
      .active_sockets = 1,
      .sum_retrans = 0,
      .sum_bytes = 0,
      .sum_srtt = 0,
      .sum_delivered = 0,
      .active_rtts = 0,
      .syn_timeouts = 0,
      .new_sockets = 0,
      .tcp_resets = 1,
  };

  if (flow_updater_) {
    flow_updater_->tcp_update(timestamp, stats, msg->is_rx);
  }
}

void SocketSpan::http_response(
    ::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, struct jsrv_ingest__http_response *msg)
{
  const enum CLIENT_SERVER_TYPE client_server = (const enum CLIENT_SERVER_TYPE)msg->client_server;
  ::ebpf_net::metrics::http_metrics_point stats = {
      .active_sockets = 1,
      .sum_code_200 = 0,
      .sum_code_400 = 0,
      .sum_code_500 = 0,
      .sum_code_other = 0,
      .sum_total_time_ns = (client_server == SC_CLIENT) ? msg->latency_ns : 0, // client latency contributes to total time
      .sum_processing_time_ns =
          (client_server == SC_SERVER) ? msg->latency_ns : 0 // server latency contributes to processing time
  };

  if (msg->code >= 200 && msg->code <= 299) {
    stats.sum_code_200 = 1;
  } else if (msg->code >= 400 && msg->code <= 499) {
    stats.sum_code_400 = 1;
  } else if (msg->code >= 500 && msg->code <= 599) {
    stats.sum_code_500 = 1;
  } else {
    stats.sum_code_other = 1;
  }

  if (flow_updater_) {
    flow_updater_->http_update(timestamp, stats, msg->client_server);
  }
}

void SocketSpan::get_flow(::ebpf_net::ingest::weak_refs::socket span_ref)
{
  auto *conn = local_connection()->ingest_connection();
  auto agent = conn->agent();
  auto &addr_map = global_private_to_public_address_map();

  auto local_addr = local_addr_;
  auto remote_addr = remote_addr_;

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
        Component::socket,
        "SocketSpan::get_flow: ignoring localhost traffic:"
        " {}:{} -> {}:{}",
        local_addr,
        local_port_,
        remote_addr,
        remote_port_);
    flow_updater_.reset();
    return;
  }

  if ((local_addr == remote_addr) && !agent.impl().is_host_address(local_addr)) {
    LOG::trace_in(
        Component::socket,
        "SocketSpan::get_flow: ignoring local traffic:"
        " {}:{} -> {}:{}",
        local_addr,
        local_port_,
        remote_addr,
        remote_port_);
    flow_updater_.reset();
    return;
  }

  LOG::trace_in(Component::socket, "SocketSpan::get_flow: {}:{} -> {}:{}", local_addr, local_port_, remote_addr, remote_port_);

  flow_updater_ =
      FlowUpdater(span_ref.process(), agent, local_addr, local_port_, remote_addr, remote_port_, is_connector_, remote_dns_);
}

void SocketSpan::nat_remapping(::ebpf_net::ingest::weak_refs::socket span_ref, u64 timestamp, jsrv_ingest__nat_remapping *msg)
{
  auto *conn = local_connection()->ingest_connection();
  AgentSpan &agent = conn->agent().impl();

  auto nat_src = IPv4Address::from(msg->src);
  auto nat_dst = IPv4Address::from(msg->dst);
  // NOTE: ports reported in the nat_remapping message are in the network byte
  // order, differing from other messages (e.g. set_state_ipv4).
  u16 nat_sport = ntohs(msg->sport);
  u16 nat_dport = ntohs(msg->dport);

  LOG::trace_in(
      Component::socket,
      "SocketSpan::nat_remapping: sk={}, {}:{} -> {}:{},"
      " src={}:{}, dst={}:{}",
      msg->sk,
      local_addr_,
      local_port_,
      remote_addr_,
      remote_port_,
      nat_src,
      nat_sport,
      nat_dst,
      nat_dport);

  // Wasn't sure if we only ever see DNAT, so did the "safe" thing and assumed
  // both could change entirely.

  if (agent.is_host_address(nat_src.to_ipv6())) {
    // remap local address only if it maps to a known host address
    local_addr_ = nat_src.to_ipv6();
    local_port_ = nat_sport;
  } else {
    LOG::trace_in(Component::socket, "SocketSpan::nat_remapping: not remapping local address");
  }

  // save NAT-ed remote address and port
  remote_addr_ = nat_dst.to_ipv6();
  remote_port_ = nat_dport;

  LOG::trace_in(
      Component::socket,
      "SocketSpan::nat_remapping: sk={}, {}:{} -> {}:{}",
      msg->sk,
      local_addr_,
      local_port_,
      remote_addr_,
      remote_port_);

  // recompute the flow of this socket
  get_flow(span_ref);
}

} // namespace reducer::ingest
