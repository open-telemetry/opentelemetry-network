/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <collector/kernel/hostport_tuple.h>

#include <generated/flowmill/agent_internal.wire_message.h>
#include <generated/flowmill/ingest/writer.h>
#include <platform/platform.h>
#include <util/logger.h>

#include <absl/container/flat_hash_map.h>

class NatHandler {
public:
  /**
   * c'tor
   */
  NatHandler(::flowmill::ingest::Writer &writer, logging::Logger &log);

  // end
  void handle_nf_nat_cleanup_conntrack(u64 timestamp, struct jb_agent_internal__nf_nat_cleanup_conntrack *msg);
  // start
  void handle_nf_conntrack_alter_reply(u64 timestamp, struct jb_agent_internal__nf_conntrack_alter_reply *msg);
  // existing
  void handle_existing_conntrack_tuple(u64 timestamp, struct jb_agent_internal__existing_conntrack_tuple *msg);

  void handle_set_state_ipv4(u64 timestamp, jb_agent_internal__set_state_ipv4 *msg);

  void handle_close_socket(u64 timestamp, jb_agent_internal__close_sock_info *msg);

  // Returns a pointer to the corresponding val for a key we lookup
  // in the nat_table_. If there is no corresponding val, return nullptr.
  hostport_tuple *get_nat_mapping(u32 src, u32 dst, u16 sport, u16 dport, u32 proto);

private:
  // Maps the IP_CT_DIR_ORIGINAL 4-tuple of a conntrack entry to the
  // IP_CT_DIR_REPLY 4-tuple of a conntrack entry. Only contains NAT-ed
  // connections
  absl::flat_hash_map<hostport_tuple, hostport_tuple> nat_table_;

  // Maps replies to originals.
  absl::flat_hash_map<hostport_tuple, hostport_tuple> nat_table_rev_;

  // Maps a struct nf_conntrack_tuple* to the corresponding 4-tuple.
  // Used when we iterate over existing conntrack entries.
  absl::flat_hash_map<u64, hostport_tuple> existing_conntrack_table_;

  // Maps a 4-tuple to its corresponding sk. Used to keep track of
  // existing sk's so we don't send NAT msgs for sk's we haven't
  // seen a set_state_ipv4 msg for yet.
  absl::flat_hash_map<hostport_tuple, u64> existing_sk_table_;

  // A reverse index of our existing_sk_table_ so that we can do cleanup later.
  absl::flat_hash_map<u64, hostport_tuple> existing_sk_table_rev_;

  ::flowmill::ingest::Writer &writer_;
  logging::Logger &log_;

  // Adds the mapping between a socket and its (local,remote) tuple.
  void record_sk(u64 sk, hostport_tuple const &ft);

  // Removes an entry from our existing_sk_table_
  void remove_sk(u64 sk);

  // Adds the specified NAT mapping to internal tables.
  void record_nat(hostport_tuple const &map_from, hostport_tuple const &map_to);

  // Removes the specified NAT mapping from internal tables.
  void remove_nat(hostport_tuple const &map_from);

  // Sends the nat_remapping message to the specified socket.
  void send_nat_remapping(u64 timestamp, u64 sk, hostport_tuple const &ft);
};
