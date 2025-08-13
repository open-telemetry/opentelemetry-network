// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/matching/component.h>
#include <reducer/matching/flow_span.h>
#include <reducer/matching/matching_core.h>

#include <reducer/constants.h>
#include <reducer/copy_metrics.h>
#include <reducer/uid_key.h>

#include <generated/ebpf_net/matching/containers.inl>
#include <generated/ebpf_net/matching/index.h>
#include <generated/ebpf_net/matching/modifiers.h>
#include <generated/ebpf_net/matching/spans.h>
#include <generated/ebpf_net/metrics.h>

#include <common/client_server_type.h>

#include <util/cgroup_parser.h>
#include <util/error_handling.h>
#include <util/ip_address.h>
#include <util/log.h>
#include <util/string_view.h>

#include <cassert>
#include <optional>
#include <string>

namespace reducer::matching {

namespace {

constexpr std::string_view kNoAgentEnvironmentName = "(no agent)";

static const IPv6Address kAddrInstanceMetadata = IPv6Address::from_host_hextets({0, 0, 0, 0, 0, 0xffff, 0xa9fe, 0xa9fe});

// Returns the underlying value if it is set, otherwise returns a
// default-constructed object.
template <typename T> const T &get_or_default(const std::optional<T> &value)
{
  static const auto *const kDefault = new T;
  return value.has_value() ? *value : *kDefault;
}

} // namespace

bool FlowSpan::aws_enrichment_enabled_ = false;

void FlowSpan::enable_aws_enrichment(bool enabled)
{
  aws_enrichment_enabled_ = enabled;
}

FlowSpan::FlowSpan() {}

FlowSpan::~FlowSpan() {}

void FlowSpan::agent_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__agent_info *msg)
{
  std::string id{msg->id.buf, msg->id.len};
  std::string az{msg->az.buf, msg->az.len};
  std::string env{msg->env.buf, msg->env.len};
  std::string role{msg->role.buf, msg->role.len};
  std::string ns{msg->ns.buf, msg->ns.len};

  LOG::trace_in(
      Component::flow, "matching::FlowSpan::agent_info: side={} id={} az={} env={} ns={}", msg->side, id, az, env, ns);

  auto const side = u8_to_side(msg->side);

  agent_info_[+side] = {
      .id = std::move(id),
      .az = std::move(az),
      .env = std::move(env),
      .role = std::move(role),
      .ns = std::move(ns),
  };

  n_received_info_messages_++;
}

void FlowSpan::task_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__task_info *msg)
{
  std::string comm{msg->comm.buf, msg->comm.len};
  std::string cgroup_name{msg->cgroup_name.buf, msg->cgroup_name.len};

  LOG::trace_in(Component::flow, "matching::FlowSpan::task_info: side={} comm='{}' cgroup='{}'", msg->side, comm, cgroup_name);

  auto const side = u8_to_side(msg->side);

  task_info_[+side] = {
      .comm = comm,
      .cgroup_name = cgroup_name,
  };

  n_received_info_messages_++;
}

void FlowSpan::socket_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__socket_info *msg)
{
  auto local_addr = IPv6Address::from(msg->local_addr);
  auto remote_addr = IPv6Address::from(msg->remote_addr);
  std::string remote_dns_name{msg->remote_dns_name.buf, msg->remote_dns_name.len};

  LOG::trace_in(
      Component::flow,
      "matching::FlowSpan::socket_info: side={}"
      " local_ip={}:{} remote_ip={}:{}",
      msg->side,
      local_addr,
      msg->local_port,
      remote_addr,
      msg->remote_port);

  auto const side = u8_to_side(msg->side);

  socket_info_[+side] = SocketInfo{
      .local_addr = local_addr,
      .local_port = msg->local_port,
      .remote_addr = remote_addr,
      .remote_port = msg->remote_port,
      .is_connector = msg->is_connector,
      .remote_dns_name = std::move(remote_dns_name),
  };

  n_received_info_messages_++;
}

void FlowSpan::k8s_info(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__k8s_info *msg)
{
  auto const side = u8_to_side(msg->side);

  std::array<u8, 64> pod_uid_suffix;
  std::copy_n(msg->pod_uid_suffix, std::min(sizeof(msg->pod_uid_suffix), pod_uid_suffix.max_size()), pod_uid_suffix.begin());

  LOG::trace_in(
      Component::flow,
      "matching::FlowSpan::k8s_info: side={} pod_uid_suffix='{}'"
      " pod_uid_hash={}",
      msg->side,
      std::string_view((char *)pod_uid_suffix.data(), pod_uid_suffix.size()),
      msg->pod_uid_hash);

  k8s_info_[+side] = K8sInfo{
      .pod_uid_suffix = std::move(pod_uid_suffix),
      .pod_uid_hash = msg->pod_uid_hash,
  };

  n_received_info_messages_++;
}

void FlowSpan::container_info(
    ::ebpf_net::matching::weak_refs::flow span_ref, const u64 timestamp, jsrv_matching__container_info *const msg)
{
  auto const side = u8_to_side(msg->side);
  std::string name{msg->name.buf, msg->name.len};
  std::string pod{msg->pod.buf, msg->pod.len};
  std::string role{msg->role.buf, msg->role.len};
  std::string version{msg->version.buf, msg->version.len};
  std::string ns{msg->ns.buf, msg->ns.len};
  auto const type = static_cast<NodeResolutionType>(msg->node_type);

  container_info_[+side] = ContainerInfo{
      .name = std::move(name),
      .pod = std::move(pod),
      .role = std::move(role),
      .version = std::move(version),
      .ns = std::move(ns),
      .type = (sanitize_enum(type) == type ? type : NodeResolutionType::CONTAINER)};

  n_received_info_messages_++;
}

void FlowSpan::service_info(
    ::ebpf_net::matching::weak_refs::flow span_ref, const u64 timestamp, jsrv_matching__service_info *const msg)
{
  auto const side = u8_to_side(msg->side);
  std::string name{msg->name.buf, msg->name.len};

  service_info_[+side] = ServiceInfo{
      .name = std::move(name),
  };

  n_received_info_messages_++;
}

////////////////////////////////////////////////////////////////////////////////

void FlowSpan::update_nodes_if_required(::ebpf_net::matching::weak_refs::flow flow)
{
  bool got_messages = (n_received_info_messages_ != message_count_on_last_update_);

  bool update_needed = got_messages || should_attempt_k8s_enrichment(flow, FlowSide::SIDE_A) ||
                       should_attempt_k8s_enrichment(flow, FlowSide::SIDE_B);

  if (!update_needed) {
    return;
  }

  update_nodes(flow, resolve_node(flow, FlowSide::SIDE_A), resolve_node(flow, FlowSide::SIDE_B));

  // set the last update value to avoid refreshing if there are no messages
  message_count_on_last_update_ = n_received_info_messages_;
}

void FlowSpan::update_nodes(::ebpf_net::matching::weak_refs::flow flow, NodeData const &node_a, NodeData const &node_b)
{
  create_agg_root(flow, node_a, node_b);

  if (!flow.agg_root().valid()) {
    return;
  }

  update_node(flow.agg_root(), FlowSide::SIDE_A, node_a);
  update_node(flow.agg_root(), FlowSide::SIDE_B, node_b);
}

void FlowSpan::update_node(::ebpf_net::matching::weak_refs::agg_root agg_root, FlowSide side, NodeData const &n)
{
  agg_root.update_node(
      static_cast<u8>(side),
      jb_blob(n.id),
      jb_blob(n.az),
      jb_blob(n.role),
      jb_blob(n.version),
      jb_blob(n.env),
      jb_blob(n.ns),
      static_cast<u8>(n.node_type),
      jb_blob(n.address),
      jb_blob(n.comm),
      jb_blob(n.container_name),
      jb_blob(n.pod_name),
      jb_blob(n.role_uid));
}

void FlowSpan::create_agg_root(::ebpf_net::matching::weak_refs::flow flow, NodeData const &node_a, NodeData const &node_b)
{
  using agg_root = ::ebpf_net::matching::spans::agg_root;

  if (node_a.role.empty() || node_b.role.empty()) {
    // can't do anything yet
    return;
  }

  std::string_view role_a = node_a.role;
  std::string_view role_b = node_b.role;
  std::string_view az_a;
  std::string_view az_b;

  // HACK ALERT!
  // We will only shard by (role,az,role,az) if one of the nodes is an IP node,
  // which means that its role is (unknown) or (internet). Not a good thing to
  // shard on. If neither of nodes is IP, we shard only on (role,role).
  //
  if ((node_a.node_type == NodeResolutionType::IP) || (node_b.node_type == NodeResolutionType::IP)) {
    az_a = node_a.az;
    az_b = node_b.az;
  }

  agg_root::role1_t role1;
  agg_root::role2_t role2;
  agg_root::az1_t az1;
  agg_root::az2_t az2;

  if (std::tie(role_a, az_a) <= std::tie(role_b, az_b)) {
    role1 = agg_root::role1_t::truncate(role_a);
    role2 = agg_root::role2_t::truncate(role_b);
    az1 = agg_root::az1_t::truncate(az_a);
    az2 = agg_root::az2_t::truncate(az_b);
  } else {
    role1 = agg_root::role1_t::truncate(role_b);
    role2 = agg_root::role2_t::truncate(role_a);
    az1 = agg_root::az1_t::truncate(az_b);
    az2 = agg_root::az2_t::truncate(az_a);
  }

  if ((flow.agg_root().valid() == false) || (role1 != flow.agg_root().role1()) || (role2 != flow.agg_root().role2()) ||
      (az1 != flow.agg_root().az1()) || (az2 != flow.agg_root().az2())) {
    flow.modify().agg_root(flow.index().agg_root.alloc(role1, az1, role2, az2));
  }
}

bool FlowSpan::should_attempt_k8s_enrichment(::ebpf_net::matching::weak_refs::flow flow, FlowSide side) const
{
  // should we retry to enrich using kubernetes pods?
  //
  // this can happen for a side if:
  //   1. we haven't already resolved kubernetes
  //   2. the side has k8s info

  // get the k8s pod info
  auto k8s_pod = (side == FlowSide::SIDE_A) ? flow.k8s_pod1() : flow.k8s_pod2();

  if (k8s_pod.valid()) {
    // already resolved and enriched as kubernetes
    return false;
  } else {
    // continue attempting if we have k8s info
    return k8s_info_[+side].has_value();
  }
}

////////////////////////////////////////////////////////////////////////////////

FlowSpan::NodeData FlowSpan::resolve_node(::ebpf_net::matching::weak_refs::flow span_ref, FlowSide side)
{
  const std::optional<AddrPort> addr_port = get_addr_port(side);
  if (unlikely(!addr_port.has_value())) {
    debug_state("address/port missing");
    return NodeData{};
  }

  // Textual representation of the IP address.
  std::string address = addr_port->addr.tidy_string();

  auto [id, az, is_autonomous_system] = get_id_az(side);

  std::string role;
  std::string role_uid;
  std::string version;
  std::string env;
  std::string ns;
  std::string container_name;
  auto node_type = NodeResolutionType::NONE;

  if (auto &agent_info = agent_info_[+side]; agent_info.has_value()) {
    env = agent_info->env;
    ns = agent_info->ns;
  } else {
    env = kNoAgentEnvironmentName;
  }

  const std::string &pod_name = get_or_default(container_info_[+side]).pod;

  // sanity checks on messages:
  // we get agent_info_ if and only if task_info and socket info
  if (unlikely(agent_info_[+side].has_value() != task_info_[+side].has_value())) {
    debug_state("agent_info not iff task_info");
  };
  if (unlikely(agent_info_[+side].has_value() != socket_info_[+side].has_value())) {
    debug_state("agent info not iff socket_info");
  }

  // Enrich, in order of data richness.
  //
  // First, try Kubernetes
  if (auto pod = get_k8s_pod(side, span_ref.index()); pod.valid()) {
    // got a k8s pod!
    LOG::trace_in(
        NodeResolutionType::K8S_CONTAINER,
        "matching::FlowSpan::update_node: enriching k8s"
        " pod_uid_suffix='{}' pod_uid_hash={} pod_owner='{}' ns='{}'",
        std::string_view((char *)pod.uid_suffix().data(), pod.uid_suffix().size()),
        pod.uid_hash(),
        pod.owner_name(),
        pod.ns());

    // set our state that we're enriched
    if (side == FlowSide::SIDE_A) {
      span_ref.modify().k8s_pod1(pod.get());
    } else {
      span_ref.modify().k8s_pod2(pod.get());
    }

    // enrich
    node_type = NodeResolutionType::K8S_CONTAINER;
    role = pod.owner_name();
    role_uid = pod.owner_uid();
    version = pod.version();
    ns = pod.ns();

    if (auto &task_info = task_info_[+side]; task_info.has_value()) {
      auto info = CGroupParser{task_info->cgroup_name}.get();
      auto container_id = info.container_id;

      if (!container_id.empty()) {
        auto container_key = make_uid_key<ebpf_net::matching::keys::k8s_container>(container_id);
        auto k8s_container = span_ref.index().k8s_container.by_key(container_key);

        if (k8s_container.valid()) {
          container_name = k8s_container.name();

          if (!k8s_container.version().empty()) {
            version = k8s_container.version();
          }
        }
      }
    }
  } else if (auto const &container = container_info_[+side]; container.has_value()) {
    // if we have container info, use that
    node_type = sanitize_enum(container->type) == container->type ? container->type : NodeResolutionType::CONTAINER;
    role = container->role;
    version = container->version;
    ns = container->ns;
  } else if (agent_info_[+side].has_value()) {
    // if we at least have an agent, use that
    node_type = NodeResolutionType::PROCESS;
    if (service_info_[+side].has_value()) {
      role = service_info_[+side]->name;
    } else if (task_info_[+side].has_value()) {
      role = task_info_[+side]->comm;
    } else {
      // Shouldn't happen :( need to fix
      role = agent_info_[+side]->role;
    }
    ns = agent_info_[+side]->ns;
  } else if (auto const &flipside_socket_info = socket_info_[+(~side)]; flipside_socket_info.has_value()) {
    // no agent: try AWS -> DNS -> IP
    auto const &ipv6 = flipside_socket_info->remote_addr;

    AwsEnrichmentInfo const *aws_info = nullptr;
    if (aws_enrichment_enabled_) {
      auto aws_enrichment_span = span_ref.index().aws_enrichment.by_key({ipv6.as_int()}, false);

      if (aws_enrichment_span.valid()) {
        aws_info = aws_enrichment_span.impl().info();
      }

      if (aws_info) {
        LOG::trace_in(
            NodeResolutionType::AWS,
            "matching::FlowSpan::update_node: found AWS metadata"
            " for: ipv6={} role={} az={} id={}",
            ipv6,
            aws_info->role,
            aws_info->az,
            aws_info->id);
      } else {
        LOG::trace_in(
            NodeResolutionType::AWS,
            "matching::FlowSpan::update_node: found no AWS metadata"
            " for: ipv6={}",
            ipv6);
      }
    }

    if (aws_info && !aws_info->role.empty() && !aws_info->az.empty()) {
      // AWS enrichment exists
      node_type = NodeResolutionType::AWS;
      role = aws_info->role;
      az = aws_info->az;
      if (!aws_info->id.empty()) {
        id = aws_info->id + "/" + id;
      }
    } else {
      if (!flipside_socket_info->remote_dns_name.empty()) {
        node_type = NodeResolutionType::DNS;
        role = flipside_socket_info->remote_dns_name;
      } else {
        // no agent and no DNS, fall back on the IP address
        node_type = NodeResolutionType::IP;
        role = is_autonomous_system ? "(internet)" : "(unknown)";
      }
    }
  }

  if (get_comm(side) == kCommKubelet) {
    role = "kubelet";
  } else if (addr_port->port == kPortDNS) {
    role = "DNS";
  } else if (addr_port->addr == kAddrInstanceMetadata) {
    role = "instance metadata";
    node_type = NodeResolutionType::INSTANCE_METADATA;
    if (auto &agent_info = agent_info_[+(~side)]; agent_info.has_value()) {
      id = agent_info->id;
      az = agent_info->az;
    }
  }

  if (is_autonomous_system && (node_type == NodeResolutionType::IP) && !MatchingCore::autonomous_system_ip_enabled()) {
    id = address = "AS";
  }

  if (container_name.empty()) {
    container_name = get_or_default(container_info_[+side]).name;
  }

  return NodeData{
      .id = id,
      .az = az,
      .role = role,
      .role_uid = role_uid,
      .version = version,
      .env = env,
      .ns = ns,
      .node_type = node_type,
      .address = address,
      .comm = get_comm(side),
      .container_name = container_name,
      .pod_name = pod_name,
  };
}

std::string FlowSpan::get_comm(FlowSide side) const
{
  return get_or_default(task_info_[+side]).comm;
}

std::optional<FlowSpan::AddrPort> FlowSpan::get_addr_port(FlowSide side) const
{
  if (auto &local_info = socket_info_[+side]; local_info.has_value()) {
    return AddrPort{local_info->local_addr, local_info->local_port};
  }

  FlowSide other_side = ~side;
  if (auto &remote_info = socket_info_[+other_side]; remote_info.has_value()) {
    return AddrPort{remote_info->remote_addr, remote_info->remote_port};
  }

  return std::nullopt;
}

std::tuple<std::string, std::string, bool> FlowSpan::get_id_az(FlowSide side) const
{
  if (auto &agent_info = agent_info_[+side]; agent_info.has_value()) {
    // use ID and AZ obtained from this side's agent info
    return std::make_tuple(agent_info->id, agent_info->az, false);
  }

  std::string id;
  std::string az = kUnknown;
  bool is_autonomous_system = false;

  FlowSide other_side = ~side;
  if (auto &socket_info = socket_info_[+other_side]; socket_info.has_value()) {
    // use the remote IP address from other side's socket info for ID
    id = socket_info->remote_addr.tidy_string();

    if (auto &an_db = local_core<MatchingCore>().an_db; an_db) {
      auto addr = reinterpret_cast<in6_addr const *>(&socket_info->remote_addr);
      if (auto entry = an_db.lookup(addr)) {
        is_autonomous_system = geoip::well_known_data::try_autonomous_system_organization(az, entry);
      }
    }
  }

  return std::make_tuple(id, az, is_autonomous_system);
}

::ebpf_net::matching::auto_handles::k8s_pod FlowSpan::get_k8s_pod(FlowSide side, ::ebpf_net::matching::Index &index)
{
  if (auto &k8s_info = k8s_info_[+side]; k8s_info.has_value()) {
    ::ebpf_net::matching::keys::k8s_pod pod_key{k8s_info->pod_uid_suffix, k8s_info->pod_uid_hash};

    LOG::trace_in(
        NodeResolutionType::K8S_CONTAINER,
        "matching::FlowSpan::get_k8s_pod: trying to find pod"
        " uid_suffix='{}' uid_hash={}",
        std::string_view((char *)pod_key.uid_suffix.data(), pod_key.uid_suffix.size()),
        pod_key.uid_hash);

    auto pod = index.k8s_pod.by_key(pod_key);

    if (pod.valid() && !pod.owner_name().empty()) {
      return pod;
    }
  }

  if (auto &task_info = task_info_[+side]; task_info.has_value()) {
    auto info = CGroupParser{task_info->cgroup_name}.get();
    auto container_id = info.container_id;

    if (!container_id.empty()) {
      auto container_key = make_uid_key<ebpf_net::matching::keys::k8s_container>(container_id);

      LOG::trace_in(
          NodeResolutionType::K8S_CONTAINER,
          "matching::FlowSpan::update_node: trying to find container"
          " id='{}' uid_suffix='{}' uid_hash={}",
          container_id,
          std::string_view((char *)container_key.uid_suffix.data(), container_key.uid_suffix.size()),
          container_key.uid_hash);

      auto k8s_container = index.k8s_container.by_key(container_key);

      if (k8s_container.valid() && k8s_container.pod().valid()) {
        return k8s_container.pod().get();
      }
    }
  }

  return ::ebpf_net::matching::auto_handles::k8s_pod(index);
}

////////////////////////////////////////////////////////////////////////////////

UpdateDirection FlowSpan::metrics_update_direction(FlowSide side, int is_rx, bool force_both_sides)
{
  if (!force_both_sides) {
    if (!metrics_update_side_.has_value()) {
      // first side to try gets to send metric updates
      metrics_update_side_ = side;
    } else if (*metrics_update_side_ != side) {
      // the other side is sending metric updates
      return UpdateDirection::NONE;
    }
  }

  if (side == FlowSide::SIDE_A) {
    return (is_rx == 0) ? UpdateDirection::A_TO_B : UpdateDirection::B_TO_A;
  } else { // side == FlowSide::SIDE_B
    return (is_rx == 0) ? UpdateDirection::B_TO_A : UpdateDirection::A_TO_B;
  }
}

void FlowSpan::tcp_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__tcp_update *msg)
{
  auto const side = u8_to_side(msg->side);
  auto const direction = metrics_update_direction(side, msg->is_rx);

  ::ebpf_net::metrics::tcp_metrics_point metrics_point;
  copy_tcp_metrics(metrics_point, *msg);

  // skip RX RTT measurements
  bool skip_rtts = msg->is_rx;

  if (skip_rtts) {
    metrics_point.sum_srtt = 0;
    metrics_point.active_rtts = 0;
  }

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.tcp_a_to_b_update(timestamp, metrics_point);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.tcp_b_to_a_update(timestamp, metrics_point);
  }

  if (!skip_rtts) {
    // Apply RTT-only metrics to side(s) that full metrics have not
    // been applied above.
    ::ebpf_net::metrics::tcp_metrics_point rtt_metrics = {
        .sum_srtt = metrics_point.sum_srtt,
        .active_rtts = metrics_point.active_rtts,
    };
    if (direction == UpdateDirection::A_TO_B) {
      span_ref.tcp_b_to_a_update(timestamp, rtt_metrics);
    } else if (direction == UpdateDirection::B_TO_A) {
      span_ref.tcp_a_to_b_update(timestamp, rtt_metrics);
    } else {
      span_ref.tcp_a_to_b_update(timestamp, rtt_metrics);
      span_ref.tcp_b_to_a_update(timestamp, rtt_metrics);
    }
  }
}

void FlowSpan::udp_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__udp_update *msg)
{
  auto const side = u8_to_side(msg->side);
  auto const direction = metrics_update_direction(side, msg->is_rx);

  ::ebpf_net::metrics::udp_metrics_point metrics_point;
  copy_udp_metrics(metrics_point, *msg);

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.udp_a_to_b_update(timestamp, metrics_point);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.udp_b_to_a_update(timestamp, metrics_point);
  }
}

void FlowSpan::http_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__http_update *msg)
{
  auto const side = u8_to_side(msg->side);
  auto const is_rx = msg->client_server == (u8)SC_SERVER; // client(0) is 'tx', server(1) is 'rx'
  auto direction = metrics_update_direction(side, is_rx);

  ::ebpf_net::metrics::http_metrics_point metrics_point;
  copy_http_metrics(metrics_point, *msg);

  // Ensure metrics are always reported on the correct side
  if (!is_rx) {
    // Client never gets processing time
    metrics_point.sum_processing_time_ns = 0;
  } else {
    // Server never gets total time
    metrics_point.sum_total_time_ns = 0;
  }

  // If we would duplicate metrics, still send a 'single-sided' update
  // but leave out metrics that would contribute twice
  if (direction == UpdateDirection::NONE) {
    // Leave only fields that are not duplicates (this excludes active_sockets)
    metrics_point = {
        .sum_total_time_ns = metrics_point.sum_total_time_ns,
        .sum_processing_time_ns = metrics_point.sum_processing_time_ns,
    };

    // And allow the update regardless of if there's another agent on the other side
    direction = metrics_update_direction(side, is_rx, true);
  }

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.http_a_to_b_update(timestamp, metrics_point);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.http_b_to_a_update(timestamp, metrics_point);
  }
}

void FlowSpan::dns_update(::ebpf_net::matching::weak_refs::flow span_ref, u64 timestamp, jsrv_matching__dns_update *msg)
{
  ASSUME(msg->side <= 1).else_log("side must be either 0 or 1, instead got {}", msg->side);

  auto const side = u8_to_side(msg->side);
  auto const is_rx = msg->client_server == (u8)SC_SERVER; // client(0) is 'tx', server(1) is 'rx'
  auto direction = metrics_update_direction(side, is_rx);

  ::ebpf_net::metrics::dns_metrics_point metrics_point;
  copy_dns_metrics(metrics_point, *msg);

  // Ensure metrics are always reported on the correct side
  if (!is_rx) {
    // Client never gets processing time
    metrics_point.sum_processing_time_ns = 0;
  } else {
    // Server never gets total time
    metrics_point.sum_total_time_ns = 0;
  }

  // If we would duplicate metrics, still send a 'single-sided' update
  // but leave out metrics that would contribute twice
  if (direction == UpdateDirection::NONE) {
    // Leave only fields that are not duplicates (this excludes active_sockets)
    metrics_point = {
        .sum_total_time_ns = metrics_point.sum_total_time_ns,
        .sum_processing_time_ns = metrics_point.sum_processing_time_ns,
    };

    // And allow the update regardless of if there's another agent on the other side
    direction = metrics_update_direction(side, is_rx, true);
  }

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.dns_a_to_b_update(timestamp, metrics_point);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.dns_b_to_a_update(timestamp, metrics_point);
  }
}

////////////////////////////////////////////////////////////////////////////////

namespace {

template <UpdateDirection Dir>
void send_tcp_metrics(
    u64 t, ::ebpf_net::matching::weak_refs::flow span_ref, ::ebpf_net::metrics::tcp_metrics const &m, u64 interval)
{
  span_ref.impl().update_nodes_if_required(span_ref);

  auto agg_root = span_ref.agg_root();
  if (!agg_root.valid()) {
    return;
  }

  agg_root.update_tcp_metrics(
      (u8)Dir,
      m.active_sockets,
      m.sum_retrans,
      m.sum_bytes,
      m.sum_srtt,
      m.sum_delivered,
      m.active_rtts,
      m.syn_timeouts,
      m.new_sockets,
      m.tcp_resets);
}

template <UpdateDirection Dir>
void send_udp_metrics(
    u64 t, ::ebpf_net::matching::weak_refs::flow span_ref, ::ebpf_net::metrics::udp_metrics const &m, u64 interval)
{
  span_ref.impl().update_nodes_if_required(span_ref);

  auto agg_root = span_ref.agg_root();
  if (!agg_root.valid()) {
    return;
  }

  agg_root.update_udp_metrics((u8)Dir, m.active_sockets, m.addr_changes, m.packets, m.bytes, m.drops);
}

template <UpdateDirection Dir>
void send_dns_metrics(
    u64 t, ::ebpf_net::matching::weak_refs::flow span_ref, ::ebpf_net::metrics::dns_metrics const &m, u64 interval)
{
  span_ref.impl().update_nodes_if_required(span_ref);

  auto agg_root = span_ref.agg_root();
  if (!agg_root.valid()) {
    return;
  }

  agg_root.update_dns_metrics(
      (u8)Dir,
      m.active_sockets,
      m.requests_a,
      m.requests_aaaa,
      m.responses,
      m.timeouts,
      m.sum_total_time_ns,
      m.sum_processing_time_ns);
}

template <UpdateDirection Dir>
void send_http_metrics(
    u64 t, ::ebpf_net::matching::weak_refs::flow span_ref, ::ebpf_net::metrics::http_metrics const &m, u64 interval)
{
  span_ref.impl().update_nodes_if_required(span_ref);

  auto agg_root = span_ref.agg_root();
  if (!agg_root.valid()) {
    return;
  }

  agg_root.update_http_metrics(
      (u8)Dir,
      m.active_sockets,
      m.sum_code_200,
      m.sum_code_400,
      m.sum_code_500,
      m.sum_code_other,
      m.sum_total_time_ns,
      m.sum_processing_time_ns);
}

} // namespace

void FlowSpan::send_metrics_to_aggregation(::ebpf_net::matching::containers::flow &flows, u64 ts)
{
  flows.tcp_a_to_b_foreach(ts, send_tcp_metrics<UpdateDirection::A_TO_B>);
  flows.tcp_b_to_a_foreach(ts, send_tcp_metrics<UpdateDirection::B_TO_A>);

  flows.udp_a_to_b_foreach(ts, send_udp_metrics<UpdateDirection::A_TO_B>);
  flows.udp_b_to_a_foreach(ts, send_udp_metrics<UpdateDirection::B_TO_A>);

  flows.dns_a_to_b_foreach(ts, send_dns_metrics<UpdateDirection::A_TO_B>);
  flows.dns_b_to_a_foreach(ts, send_dns_metrics<UpdateDirection::B_TO_A>);

  flows.http_a_to_b_foreach(ts, send_http_metrics<UpdateDirection::A_TO_B>);
  flows.http_b_to_a_foreach(ts, send_http_metrics<UpdateDirection::B_TO_A>);
}

////////////////////////////////////////////////////////////////////////////////

void FlowSpan::debug_state(const std::string_view &reason)
{
  LOG::debug_in(
      NodeResolutionType::NONE,
      "FlowSpan - n_messages={} last_update={} - '{}'",
      n_received_info_messages_,
      message_count_on_last_update_,
      reason);

  for (int i = 0; i < 2; i++) {
    LOG::debug_in(NodeResolutionType::NONE, "side[{}]:", i);

    if (agent_info_[i].has_value()) {
      LOG::debug_in(
          NodeResolutionType::NONE,
          "  agent:     id='{}' az='{}' env='{}'",
          agent_info_[i]->id,
          agent_info_[i]->az,
          agent_info_[i]->env);
    } else {
      LOG::debug_in(NodeResolutionType::NONE, "  agent:     null");
    }

    if (task_info_[i].has_value()) {
      LOG::debug_in(NodeResolutionType::NONE, "  task:      comm='{}'", task_info_[i]->comm);
    } else {
      LOG::debug_in(NodeResolutionType::NONE, "  task:      null");
    }

    if (socket_info_[i].has_value()) {
      LOG::debug_in(
          NodeResolutionType::NONE,
          "  socket:    local_addr='{}' local_port={} remote_addr='{}' "
          "remote_port={}",
          socket_info_[i]->local_addr,
          socket_info_[i]->local_port,
          socket_info_[i]->remote_addr,
          socket_info_[i]->remote_port);
    } else {
      LOG::debug_in(NodeResolutionType::NONE, "  socket:    null");
    }

    if (container_info_[i].has_value()) {
      LOG::debug_in(
          NodeResolutionType::NONE, "  container: name='{}' pod='{}'", container_info_[i]->name, container_info_[i]->pod);
    } else {
      LOG::debug_in(NodeResolutionType::NONE, "  container: null");
    }
  }
}

} // namespace reducer::matching
