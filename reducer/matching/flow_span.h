/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/constants.h>

#include <generated/ebpf_net/matching/modifiers.h>
#include <generated/ebpf_net/matching/span_base.h>

#include <util/ip_address.h>

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <tuple>

namespace reducer::matching {

class FlowSpan : public ::ebpf_net::matching::FlowSpanBase {
public:
  FlowSpan();
  ~FlowSpan();

  void agent_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__agent_info *msg);
  void task_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__task_info *msg);
  void socket_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__socket_info *msg);
  void k8s_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__k8s_info *msg);
  void container_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__container_info *msg);
  void service_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__service_info *msg);

  void tcp_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__tcp_update *msg);
  void udp_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__udp_update *msg);
  void http_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__http_update *msg);
  void dns_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__dns_update *msg);

  // Updates nodes if new messages have arrived.
  void update_nodes_if_required(::ebpf_net::matching::weak_refs::flow flow);

  static void send_metrics_to_aggregation(::ebpf_net::matching::containers::flow &flows, u64 timestamp);

  // NOTE: must be called on startup, from main, before any flow spans are
  // created
  static void enable_aws_enrichment(bool enabled);

private:
  struct AgentInfo {
    std::string id;
    std::string az;
    std::string env;
    std::string role;
    std::string ns;
  };

  struct TaskInfo {
    std::string comm;
    std::string cgroup_name;
  };

  struct SocketInfo {
    IPv6Address local_addr;
    u16 local_port = 0;
    IPv6Address remote_addr;
    u16 remote_port = 0;
    u8 is_connector = 0;
    std::string remote_dns_name;
  };

  struct K8sInfo {
    std::array<u8, 64> pod_uid_suffix;
    u64 pod_uid_hash;
  };

  struct ContainerInfo {
    std::string name;
    std::string pod;
    std::string role;
    std::string version;
    std::string ns;
    NodeResolutionType type = NodeResolutionType::CONTAINER;
  };

  struct ServiceInfo {
    std::string name;
  };

  struct AddrPort {
    IPv6Address addr;
    u16 port;
  };

  // Fully resolved node information.
  struct NodeData {
    std::string id;
    std::string az;
    std::string role;
    std::string role_uid;
    std::string version;
    std::string env;
    std::string ns;
    NodeResolutionType node_type;
    std::string address;
    std::string comm;
    std::string container_name;
    std::string pod_name;
  };

  std::array<std::optional<AgentInfo>, 2> agent_info_;
  std::array<std::optional<TaskInfo>, 2> task_info_;
  std::array<std::optional<SocketInfo>, 2> socket_info_;
  std::array<std::optional<K8sInfo>, 2> k8s_info_;
  std::array<std::optional<ContainerInfo>, 2> container_info_;
  std::array<std::optional<ServiceInfo>, 2> service_info_;

  // Updates nodes using the provided full-resolved node data.
  void update_nodes(::ebpf_net::matching::weak_refs::flow flow, NodeData const &node_a, NodeData const &node_b);

  //! Sends the provided node data to the aggregation root.
  void update_node(::ebpf_net::matching::weak_refs::agg_root agg_root, FlowSide side, NodeData const &n);

  // Creates the appropriate agg_root proxy and assignes it to the span.
  void create_agg_root(::ebpf_net::matching::weak_refs::flow flow, NodeData const &node_a, NodeData const &role_b);

  // Returns true if we should try to re-enrich kubernetes information for the
  // side
  bool should_attempt_k8s_enrichment(::ebpf_net::matching::weak_refs::flow flow, FlowSide side) const;

  // Returns the direction of metrics updates.
  UpdateDirection metrics_update_direction(FlowSide side, int is_rx, bool force_both_sides = false);

  // Resolves all available information into full node data.
  NodeData resolve_node(::ebpf_net::matching::weak_refs::flow span_ref, FlowSide side);

  std::string get_comm(FlowSide side) const;
  std::optional<AddrPort> get_addr_port(FlowSide side) const;

  // Write out the entire state of this flow, preceded with the `reason` string.
  // Debug is gated by NodeResolutionType::NONE
  void debug_state(const std::string_view &reason);

  // Returns the ID and AZ information for the specified side.
  // The third tuple element indicates whether this is an autonomous system.
  std::tuple<std::string, std::string, bool> get_id_az(FlowSide side) const;

  // Returns the k8s_pod span of the process for the specified side, if any.
  ::ebpf_net::matching::auto_handles::k8s_pod get_k8s_pod(FlowSide side, ::ebpf_net::matching::Index &index);

  // Side of the flow that will update metrics.
  // Only one side will do that, to avoid double counting.
  std::optional<FlowSide> metrics_update_side_;

  // update_node caching: counter of how many messages had been received since
  // the span was created. Can overflow as long as update_node() is called
  // before overflow (highly likely)
  u32 n_received_info_messages_ = 0;

  // the value of n_received_info_messages_ when update_node() was last called
  u32 message_count_on_last_update_ = ~0u;

  static bool aws_enrichment_enabled_;
};

} // namespace reducer::matching
