/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/ingest/container_updater.h>

#include <reducer/constants.h>
#include <reducer/dns_cache.h>

#include <generated/ebpf_net/ingest/handles.h>
#include <generated/ebpf_net/ingest/weak_refs.h>

#include <util/ip_address.h>

#include <optional>

namespace reducer::ingest {

// Encapsulates access to flow spans.
//
// For clients to keep a reference to a flow span, it is required that they
// also keep references to both endpoints whose nodes are used for flow span's
// key. This class helps to do exactly that.
//
class FlowUpdater {
public:
  // Constructs the object and creates the underlying flow span.
  FlowUpdater(
      ::ebpf_net::ingest::weak_refs::process process_ref,
      ::ebpf_net::ingest::weak_refs::agent agent_ref,
      IPv6Address local_addr,
      u16 local_port,
      IPv6Address remote_addr,
      u16 remote_port,
      u32 is_connector,
      std::optional<dns::dns_record> remote_dns);

  // Puts all handles back to index.
  ~FlowUpdater();

  // Moving is allowed.
  //
  FlowUpdater(FlowUpdater &&) = default;
  FlowUpdater &operator=(FlowUpdater &&);

  // Copying is not allowed.
  //
  FlowUpdater(const FlowUpdater &) = delete;
  FlowUpdater &operator=(const FlowUpdater &) = delete;

  // Returns the local address
  IPv6Address local_addr() { return local_addr_; }

  // Returns the remote address
  IPv6Address remote_addr() { return remote_addr_; }

  // Returns whether the flow span is valid.
  //
  // The flow span handle will be assigned only if both endpoints
  // and their nodes are valid.
  //
  bool valid() { return flow_handle_.valid(); }

  // Sends tcp_update to the flow span.
  void tcp_update(u64 t, ::ebpf_net::metrics::tcp_metrics_point &m, int is_rx);

  // Sends udp_update to the flow span.
  void udp_update(u64 t, ::ebpf_net::metrics::udp_metrics_point &m, int is_rx);

  // Sends http_update to the flow span.
  void http_update(u64 t, ::ebpf_net::metrics::http_metrics_point &m, u8 client_server);

  // Sends dns_update to the flow span.
  void dns_update(u64 t, const ::ebpf_net::metrics::dns_metrics_point &m, u8 client_server);

private:
  u16 local_port_;
  u16 remote_port_;
  u32 is_connector_;
  IPv6Address local_addr_;
  IPv6Address remote_addr_;
  std::optional<dns::dns_record> remote_dns_;
  ::ebpf_net::ingest::handles::process process_handle_;
  ::ebpf_net::ingest::handles::agent agent_handle_;

  // Handle of the flow this object is encapsulating.
  // This handle can change as endpoints' nodes are changing.
  ::ebpf_net::ingest::handles::flow flow_handle_;

  // Indicates whether local side is flow's side-a (node1) or side-b (node2).
  FlowSide side_;

  // Flag indicating whether updates to this flow should be ignored.
  bool ignore_updates_;

  // For sending container info updates to the matching core.
  ContainerUpdater container1_updater_;
  ContainerUpdater container2_updater_;

  // Ensures that the flow span is created and properly constructed.
  // Saves the flow handle to flow_handle_.
  // If the flow key doesn't change between calls, returns the value
  // store in the flow_handle_.
  //
  ::ebpf_net::ingest::weak_refs::flow create_flow();

  // Prepares the flow for stats update.
  // Should be called before any {tcp,udp,http,dns}_update method.
  // If the reference returned is invalid, the update should be ignored.
  //
  ::ebpf_net::ingest::weak_refs::flow get_flow_for_update();

  // Puts all handles back to index.
  void put_handles();
};

} // namespace reducer::ingest
