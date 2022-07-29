// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include <collector/kernel/nat_handler.h>

#include <collector/agent_log.h>

#include <util/ip_address.h>
#include <util/log.h>

constexpr auto SIZEOF_STRUCT_NF_CONNTRACK_TUPLE_HASH = 56;

NatHandler::NatHandler(::flowmill::ingest::Writer &writer, logging::Logger &log) : writer_(writer), log_(log) {}

/* END */
void NatHandler::handle_nf_nat_cleanup_conntrack(u64 timestamp, struct jb_agent_internal__nf_nat_cleanup_conntrack *msg)
{
  if (is_log_whitelisted(AgentLogKind::NAT)) {
    LOG::trace_in(
        AgentLogKind::NAT,
        "NatHandler::handle_nf_nat_cleanup_conntrack: ct={}, "
        "src={}:{}, dst={}:{}, proto={}",
        msg->ct,
        IPv4Address::from(msg->src_ip),
        ntohs(msg->src_port),
        IPv4Address::from(msg->dst_ip),
        ntohs(msg->dst_port),
        msg->proto);
  }

  const hostport_tuple ft = {
      msg->src_ip,
      msg->dst_ip,
      msg->src_port,
      msg->dst_port,
      msg->proto,
  };

  remove_nat(ft);
}

/* START */
void NatHandler::handle_nf_conntrack_alter_reply(u64 timestamp, struct jb_agent_internal__nf_conntrack_alter_reply *msg)
{
  if (is_log_whitelisted(AgentLogKind::NAT)) {
    LOG::trace_in(
        AgentLogKind::NAT,
        "NatHandler::handle_nf_conntrack_alter_reply: ct={}, "
        "src={}:{}, dst={}:{}, proto={}, "
        "nat_src={}:{}, nat_dst={}:{}, nat_proto={}",
        msg->ct,
        IPv4Address::from(msg->src_ip),
        ntohs(msg->src_port),
        IPv4Address::from(msg->dst_ip),
        ntohs(msg->dst_port),
        msg->proto,
        IPv4Address::from(msg->nat_src_ip),
        ntohs(msg->nat_src_port),
        IPv4Address::from(msg->nat_dst_ip),
        ntohs(msg->nat_dst_port),
        msg->nat_proto);
  }

  const hostport_tuple map_from = {
      .src_ip = msg->src_ip,
      .dst_ip = msg->dst_ip,
      .src_port = msg->src_port,
      .dst_port = msg->dst_port,
      .proto = msg->proto,
  };

  const hostport_tuple map_to = {
      .src_ip = msg->nat_src_ip,
      .dst_ip = msg->nat_dst_ip,
      .src_port = msg->nat_src_port,
      .dst_port = msg->nat_dst_port,
      .proto = msg->proto,
  };

  record_nat(map_from, map_to);

  // If we've seen an sk for this four-tuple already, we can report to the
  // server
  if (auto sk_pos = existing_sk_table_.find(map_from); sk_pos != existing_sk_table_.end()) {
    u64 sk = sk_pos->second;
    send_nat_remapping(timestamp, sk, map_to);
  } else {
    LOG::trace_in(AgentLogKind::NAT, "sk doesn't exist for this four-tuple yet");
  }
}

/* EXISTING */
void NatHandler::handle_existing_conntrack_tuple(u64 timestamp, struct jb_agent_internal__existing_conntrack_tuple *msg)
{
  if (is_log_whitelisted(AgentLogKind::NAT)) {
    LOG::trace_in(
        AgentLogKind::NAT,
        "NatHandler::handle_existing_conntrack_tuple: ct={}, dir={}, "
        "src={}:{}, dst={}:{}, proto={}",
        msg->ct,
        msg->dir,
        IPv4Address::from(msg->src_ip),
        ntohs(msg->src_port),
        IPv4Address::from(msg->dst_ip),
        ntohs(msg->dst_port),
        msg->proto);
  }

  const u64 ct = msg->ct;
  const u8 dir = msg->dir;

  const hostport_tuple ft = {
      msg->src_ip,
      msg->dst_ip,
      msg->src_port,
      msg->dst_port,
      msg->proto,
  };

  // We have two cases here: either the direction is 0 or 1
  // (IP_CT_DIR_ORIGINAL/IP_CT_DIR_REPLY). Because of the way this probe is
  // triggered, we expect the incoming msgs to be ordered as alternating 0,1
  // pairs, where every pair corresponds to a given connection. dir=O is the
  // first side we see for a connection, so we simply record the four-tuple.
  // dir=1 should always come after 0, so we check to make sure that the
  // corresponding dir=0 info is present in our table, and then we record the
  // connection info if this pair is a NAT.
  if (dir == 0) {
    existing_conntrack_table_[ct] = ft;
    return;
  }

  // in the dir==1 case, we reported "ct-sizeof(struct
  // nf_conntrack_tuple_hash)", which should be the same addr as the
  // corresponding dir==0 conntrack_tuple.
  auto search = existing_conntrack_table_.find(ct);
  if (search == existing_conntrack_table_.end()) {
    log_.error("existing conntrack not found");
    return;
  }

  hostport_tuple const &map_from = search->second;
  hostport_tuple const &map_to = ft;

  // Check whether this is a NAT-ed connection
  if (map_from == map_to) {
    return;
  }

  if (is_log_whitelisted(AgentLogKind::NAT)) {
    LOG::trace_in(
        AgentLogKind::NAT,
        "connection is NAT-ed from: src={}:{}, dst={}:{}, proto={}",
        IPv4Address::from(map_from.src_ip),
        ntohs(map_from.src_port),
        IPv4Address::from(map_from.dst_ip),
        ntohs(map_from.dst_port),
        map_from.proto);
  }

  record_nat(map_from, map_to);

  // Clean up the other direction for this connection
  if (auto revct_it = existing_conntrack_table_.find(ct - SIZEOF_STRUCT_NF_CONNTRACK_TUPLE_HASH);
      revct_it != existing_conntrack_table_.end()) {
    existing_conntrack_table_.erase(revct_it);
  } else {
    LOG::debug_in(AgentLogKind::NAT, "reverse direction not found for ct={}", ct);
  }
}

void NatHandler::handle_set_state_ipv4(u64 timestamp, jb_agent_internal__set_state_ipv4 *msg)
{
  if (is_log_whitelisted(AgentLogKind::NAT)) {
    LOG::trace_in(
        AgentLogKind::NAT,
        "NatHandler::handle_set_state_ipv4: "
        "sk={}, src={}:{}, dest={}:{}, tx_rx={}",
        msg->sk,
        IPv4Address::from(msg->src),
        msg->sport,
        IPv4Address::from(msg->dest),
        msg->dport,
        msg->tx_rx);
  }

  const u64 sk = msg->sk;

  const hostport_tuple ft = {
      .src_ip = msg->src,
      .dst_ip = msg->dest,
      .src_port = htons(msg->sport),
      .dst_port = htons(msg->dport),
      .proto = IPPROTO_TCP,
  };

  record_sk(sk, ft);

  // We had a NAT table entry before getting the socket info.
  if (auto mapping = nat_table_.find(ft); mapping != nat_table_.end()) {
    send_nat_remapping(timestamp, sk, mapping->second);
  }

  if (auto rev_mapping = nat_table_rev_.find(ft.reversed()); rev_mapping != nat_table_rev_.end()) {
    send_nat_remapping(timestamp, sk, rev_mapping->second.reversed());
  }
}

void NatHandler::handle_close_socket(u64 timestamp, jb_agent_internal__close_sock_info *msg)
{
  remove_sk(msg->sk);
}

void NatHandler::record_sk(u64 sk, hostport_tuple const &ft)
{
  // We were hitting the assert in remove_sk(), which happens if two sk's use
  // the same four-tuple without a call to remove_sk() in-between.
  if (auto search = existing_sk_table_.find(ft); search != existing_sk_table_.end()) {
    const auto &existing_sk = search->second;
    LOG::debug_in(
        AgentLogKind::NAT,
        "NatHandler::record_sk: rewriting existing ft->sk mapping: "
        "sk={}, existing_sk={}, ft=({}:{},{}:{})",
        sk,
        existing_sk,
        IPv4Address::from(ft.src_ip),
        ntohs(ft.src_port),
        IPv4Address::from(ft.dst_ip),
        ntohs(ft.dst_port));
    remove_sk(existing_sk);
  }

  // There was also an edge-case where we'd have a memory leak of the same sk
  // gets used for two-dfferent four-tuples without a call to remove_sk()
  // in-between.
  if (auto rev_search = existing_sk_table_rev_.find(sk); rev_search != existing_sk_table_rev_.end()) {
    const auto &existing_ft = rev_search->second;
    LOG::debug_in(
        AgentLogKind::NAT,
        "NatHandler::record_sk: rewriting existing sk->ft mapping: "
        "sk={}, existing_ft={{}:{},{}:{}), ft=({}:{},{}:{})",
        sk,
        IPv4Address::from(existing_ft.src_ip),
        ntohs(existing_ft.src_port),
        IPv4Address::from(existing_ft.dst_ip),
        ntohs(existing_ft.dst_port),
        IPv4Address::from(ft.src_ip),
        ntohs(ft.src_port),
        IPv4Address::from(ft.dst_ip),
        ntohs(ft.dst_port));
    remove_sk(sk);
  }

  existing_sk_table_[ft] = sk;
  existing_sk_table_rev_[sk] = ft;
}

void NatHandler::remove_sk(u64 sk)
{
  auto rev_search = existing_sk_table_rev_.find(sk);
  if (rev_search == existing_sk_table_rev_.end()) {
    // TODO: this happens pretty frequently. Initially this would happen in the
    // gap between the end and set_state probes. However, this would also happen
    // if any socket closes without reaching established as well.
    return;
  }
  auto search = existing_sk_table_.find(rev_search->second);
  assert(search != existing_sk_table_.end()); // tuple should be in existing_sk_table_

  existing_sk_table_rev_.erase(rev_search);
  existing_sk_table_.erase(search);
}

void NatHandler::record_nat(hostport_tuple const &map_from, hostport_tuple const &map_to)
{
  // Clean up possible previous records.
  //
  if (auto it = nat_table_.find(map_from); it != nat_table_.end()) {
    // map_from->some_to exist, remove some_to->map_from
    LOG::debug_in(
        AgentLogKind::NAT,
        "NatHandler::record_nat: rewriting existing mapping: "
        "({}:{},{}:{})->({}:{},{}:{}) with "
        "({}:{},{}:{})->({}:{},{}:{})",
        IPv4Address::from(map_from.src_ip),
        ntohs(map_from.src_port),
        IPv4Address::from(map_from.dst_ip),
        ntohs(map_from.dst_port),
        IPv4Address::from(it->second.src_ip),
        ntohs(it->second.src_port),
        IPv4Address::from(it->second.dst_ip),
        ntohs(it->second.dst_port),
        IPv4Address::from(map_from.src_ip),
        ntohs(map_from.src_port),
        IPv4Address::from(map_from.dst_ip),
        ntohs(map_from.dst_port),
        IPv4Address::from(map_to.src_ip),
        ntohs(map_to.src_port),
        IPv4Address::from(map_to.dst_ip),
        ntohs(map_to.dst_port));
    nat_table_rev_.erase(it->second);
  }
  if (auto rev_it = nat_table_rev_.find(map_to); rev_it != nat_table_rev_.end()) {
    // map_to->some_from exists, remove some_from->map_to
    LOG::debug_in(
        AgentLogKind::NAT,
        "NatHandler::record_nat: rewriting existing reverse mapping: "
        "({}:{},{}:{})<-({}:{},{}:{}) with "
        "({}:{},{}:{})<-({}:{},{}:{})",
        IPv4Address::from(rev_it->second.src_ip),
        ntohs(rev_it->second.src_port),
        IPv4Address::from(rev_it->second.dst_ip),
        ntohs(rev_it->second.dst_port),
        IPv4Address::from(map_to.src_ip),
        ntohs(map_to.src_port),
        IPv4Address::from(map_to.dst_ip),
        ntohs(map_to.dst_port),
        IPv4Address::from(map_from.src_ip),
        ntohs(map_from.src_port),
        IPv4Address::from(map_from.dst_ip),
        ntohs(map_from.dst_port),
        IPv4Address::from(map_to.src_ip),
        ntohs(map_to.src_port),
        IPv4Address::from(map_to.dst_ip),
        ntohs(map_to.dst_port));
    nat_table_.erase(rev_it->second);
  }

  nat_table_[map_from] = map_to;
  nat_table_rev_[map_to] = map_from;
}

void NatHandler::remove_nat(hostport_tuple const &map_from)
{
  if (auto it = nat_table_.find(map_from); it != nat_table_.end()) {
    const auto &map_to = it->second;
    nat_table_rev_.erase(map_to);
    nat_table_.erase(it);
  }
}

hostport_tuple *NatHandler::get_nat_mapping(u32 src, u32 dst, u16 sport, u16 dport, u32 proto)
{
  if (is_log_whitelisted(AgentLogKind::NAT)) {
    LOG::trace_in(
        AgentLogKind::NAT,
        "NatHandler::get_nat_mapping: src={}:{}, dst={}:{}, proto={}",
        IPv4Address::from(src),
        sport,
        IPv4Address::from(dst),
        dport,
        proto);
  }

  const hostport_tuple ft = {
      src,
      dst,
      htons(sport),
      htons(dport),
      proto,
  };

  if (auto it = nat_table_.find(ft); it != nat_table_.end()) {
    LOG::trace_in(AgentLogKind::NAT, "mapping found");
    return &(it->second);
  } else {
    LOG::trace_in(AgentLogKind::NAT, "no mapping found");
    return nullptr;
  }
}

void NatHandler::send_nat_remapping(u64 timestamp, u64 sk, hostport_tuple const &ft)
{
  if (is_log_whitelisted(AgentLogKind::NAT)) {
    LOG::trace_in(
        AgentLogKind::NAT,
        "NatHandler::send_nat_remapping: sk={}, src={}:{}, dst={}:{}",
        sk,
        IPv4Address::from(ft.src_ip),
        ntohs(ft.src_port),
        IPv4Address::from(ft.dst_ip),
        ntohs(ft.dst_port));
  }

  writer_.nat_remapping_tstamp(timestamp, sk, ft.src_ip, ft.dst_ip, ft.src_port, ft.dst_port);
}
