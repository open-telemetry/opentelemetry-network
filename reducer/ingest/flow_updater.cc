// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "flow_updater.h"

#include <reducer/ingest/component.h>
#include <reducer/ingest/shared_state.h>

#include <reducer/constants.h>

#include <generated/ebpf_net/ingest/keys.h>
#include <generated/ebpf_net/ingest/modifiers.h>

#include <platform/userspace-time.h>

#include <util/cgroup_parser.h>
#include <util/log.h>
#include <util/lookup3.h>
#include <util/string_view.h>

#include <tuple>

namespace reducer::ingest {

namespace {

template <size_t N> jb_blob jb(short_string<N> const &s)
{
  return {s.buf, s.len};
}

jb_blob jb(std::string const &s)
{
  return {s.c_str(), (u16)s.length()};
}

u32 reverse_connector(u32 connector)
{
  switch (connector) {
  case 1:
    return 2;
  case 2:
    return 1;
  default:
    return connector;
  }
}

} // namespace

FlowUpdater::FlowUpdater(
    ::ebpf_net::ingest::weak_refs::process process_ref,
    ::ebpf_net::ingest::weak_refs::agent agent_ref,
    IPv6Address local_addr,
    u16 local_port,
    IPv6Address remote_addr,
    u16 remote_port,
    u32 is_connector,
    std::optional<dns::dns_record> remote_dns)
    : local_port_(local_port),
      remote_port_(remote_port),
      is_connector_(is_connector),
      local_addr_(local_addr),
      remote_addr_(remote_addr),
      remote_dns_(remote_dns),
      ignore_updates_(false)
{
  // keep a handle to the process in `process_ref`
  if (process_ref.valid()) {
    process_handle_ = process_ref.get().to_handle();
  } else {
    LOG::error("FlowUpdater: got invalid process for local_addr={}", local_addr);
  }

  // keep a handle to the agent in the `agent_ref`
  if (agent_ref.valid()) {
    agent_handle_ = agent_ref.get().to_handle();
  } else {
    LOG::error("FlowUpdater: got invalid agent for local_addr={}", local_addr);
  }

  (void)create_flow();
}

FlowUpdater::~FlowUpdater()
{
  put_handles();
}

FlowUpdater &FlowUpdater::operator=(FlowUpdater &&other)
{
  put_handles();

  local_port_ = other.local_port_;
  remote_port_ = other.remote_port_;
  is_connector_ = other.is_connector_;
  local_addr_ = other.local_addr_;
  remote_addr_ = other.remote_addr_;
  remote_dns_ = other.remote_dns_;

  process_handle_ = std::move(other.process_handle_);
  agent_handle_ = std::move(other.agent_handle_);
  flow_handle_ = std::move(other.flow_handle_);

  side_ = other.side_;
  ignore_updates_ = other.ignore_updates_;

  return *this;
}

void FlowUpdater::put_handles()
{
  flow_handle_.put(*local_index());
  process_handle_.put(*local_index());
  agent_handle_.put(*local_index());
}

::ebpf_net::ingest::weak_refs::flow FlowUpdater::create_flow()
{
  u128 local_addr_int = local_addr_.as_int();
  u128 remote_addr_int = remote_addr_.as_int();

  bool ordered = (std::tie(local_addr_int, local_port_) < std::tie(remote_addr_int, remote_port_));

  ::ebpf_net::ingest::keys::flow flow_key;
  if (ordered) {
    flow_key.addr1 = local_addr_int;
    flow_key.port1 = local_port_;
    flow_key.addr2 = remote_addr_int;
    flow_key.port2 = remote_port_;
  } else {
    flow_key.addr1 = remote_addr_int;
    flow_key.port1 = remote_port_;
    flow_key.addr2 = local_addr_int;
    flow_key.port2 = local_port_;
  }

  auto flow = local_index()->flow.by_key(flow_key);

  if (!flow.valid()) {
    LOG::trace_in(Component::flow_update, "FlowUpdater::create_flow: could not allocate a flow span");
    return ::ebpf_net::ingest::handles::flow().access(*local_index());
  }

  if (flow_handle_.valid() && flow_handle_.loc() == flow.loc()) {
    // unchanged
    return flow_handle_.access(*local_index());
  }

  if (ordered) {
    // override the flow's "connector" if it is not unknown, otherwise leave it
    // be
    if (is_connector_ != 0) {
      flow.modify().is_connector(is_connector_);
    }
    side_ = FlowSide::SIDE_A;
  } else {
    // override the flow's "connector" if it is not unknown, otherwise leave it
    // be
    if (is_connector_ != 0) {
      flow.modify().is_connector(reverse_connector(is_connector_));
    }
    side_ = FlowSide::SIDE_B;
  }

  if (auto process = process_handle_.access(*local_index()); process.valid()) {
    if (side_ == FlowSide::SIDE_A) {
      flow.modify().process1(process.get());
    } else {
      flow.modify().process2(process.get());
    }

    std::string_view cgroup_name;
    std::string_view service_name;

    if (auto service = process.service(); service.valid()) {
      service_name = service.name().to_string_view();
    }

    if (auto cgroup = process.cgroup(); cgroup.valid()) {
      cgroup_name = cgroup.name();
      auto cgroup_info = CGroupParser{cgroup_name}.get();

      if (auto service = cgroup.service(); service.valid()) {
        // Overrides process service.
        service_name = service.name().to_string_view();
      }

      // for cgroupfs style cgroup hierarchies, the container is the child of the pod
      // so if we are looking at the child, and the parent is valid and has a pod id,
      // send it on.
      if (auto parent = cgroup.parent(); parent.valid() && (parent.pod_uid_suffix().at(0) != 0)) {
        flow.k8s_info((u8)side_, parent.pod_uid_suffix().data(), parent.pod_uid_hash());
      } else if (
          // for systemd style, the cgroup has the entire hierarchy embedded in the cgroup
          // so parse it, and look to see if one has a pod id, a container id, and if
          // this pod_uid_suffix has been assigned.  if so, send that up.
          !cgroup_info.pod_id.empty() && !cgroup_info.container_id.empty() && cgroup.pod_uid_suffix().at(0) != 0) {
        flow.k8s_info((u8)side_, cgroup.pod_uid_suffix().data(), cgroup.pod_uid_hash());
      }
    }

    // Send task information to flow.
    flow.task_info((u8)side_, jb_blob(process.comm()), jb_blob(cgroup_name));

    if (!service_name.empty()) {
      // Send service information, if any.
      flow.service_info((u8)side_, jb_blob(service_name));
    }
  }

  {
    uint8_t local_addr_buf[16];
    local_addr_.write_to(local_addr_buf);

    uint8_t remote_addr_buf[16];
    remote_addr_.write_to(remote_addr_buf);

    std::string_view remote_dns_name;
    if (remote_dns_.has_value()) {
      remote_dns_name = remote_dns_.value().to_string_view();
    }

    // send socket information to flow
    flow.socket_info(
        (u8)side_, local_addr_buf, local_port_, remote_addr_buf, remote_port_, (u8)is_connector_, jb_blob(remote_dns_name));
  }

  if (auto agent_ref = agent_handle_.access(*local_index()); agent_ref.valid()) {

    auto &agent = agent_ref.impl();

    // send agent information to flow
    flow.agent_info((u8)side_, jb(agent.node_id()), jb(agent.node_az()), jb(agent.cluster()), jb(agent.role()), jb(agent.ns()));
  }

  LOG::trace_in(
      Component::flow_update,
      "Flow::get_flow: flow={}, local_ip={}:{}, remote_ip={}:{}",
      flow.loc(),
      local_addr_,
      local_port_,
      remote_addr_,
      remote_port_);

  flow_handle_.put(*local_index());
  flow_handle_ = flow.get().to_handle();

  return flow_handle_.access(*local_index());
}

////////////////////////////////////////////////////////////////////////////////

::ebpf_net::ingest::weak_refs::flow FlowUpdater::get_flow_for_update()
{
  if (ignore_updates_) {
    return ::ebpf_net::ingest::handles::flow().access(*local_index());
  }

  auto flow = create_flow();

  if (!flow.valid()) {
    return ::ebpf_net::ingest::handles::flow().access(*local_index());
  }

  auto proc1 = flow.process1();
  auto proc2 = flow.process2();

  // Is any of the two processes involved in this flow a "proxy process"?
  // When a proxy process is in play, two flows are present: A -> P -> B.
  // is_proxy is 1: proc1 is docker proxy, 2: proc2 is docker proxy. 0: neither
  u32 is_proxy = 0;
  if (proc1.valid() && (proc1.comm().to_string() == "docker-proxy")) {
    is_proxy = 1;
  } else if (proc2.valid() && (proc2.comm().to_string() == "docker-proxy")) {
    is_proxy = 2;
  }

  if ((is_proxy != 0) && (is_proxy == flow.is_connector())) {
    // This is a flow where the proxy process is the connector (connection
    // originator), i.e. the P -> B flow. Updates to this flow should be
    // ignored, but first we assign B's role to P so the A -> P updates
    // are attributed to A -> B.

    if ((is_proxy == 1) && proc2.valid() && proc2.cgroup().valid()) {
      proc1.modify().cgroup_override(proc2.cgroup().get());
      ignore_updates_ = true;
    } else if ((is_proxy == 2) && proc1.valid() && proc1.cgroup().valid()) {
      proc2.modify().cgroup_override(proc1.cgroup().get());
      ignore_updates_ = true;
    }

    return ::ebpf_net::ingest::handles::flow().access(*local_index());
  }

  if ((flow.container1_override().valid() && (flow.container1_override().loc() == flow.container2().loc())) ||
      (flow.container2_override().valid() && (flow.container2_override().loc() == flow.container1().loc()))) {
    // Hack for existing connections, when is_connector is unknown: some other
    // flow updater redirected P's container to point to B's.
    ignore_updates_ = true;
    return ::ebpf_net::ingest::handles::flow().access(*local_index());
  }

  // at this point, if this flow is part of a A->P->B proxy transfer, this flow
  // is not ignored, and one side has a container_override. We'll send the task
  // info message again to ensure the matching core has the correct cgroup ID
  if ((side_ == FlowSide::SIDE_A) && proc1.valid() && proc1.cgroup_override().valid()) {
    auto cgroup = proc1.cgroup_override();
    flow.task_info((u8)side_, jb(proc1.comm()), jb(cgroup.name()));
  }
  if ((side_ == FlowSide::SIDE_B) && proc2.valid() && proc2.cgroup_override().valid()) {
    auto cgroup = proc2.cgroup_override();
    flow.task_info((u8)side_, jb(proc2.comm()), jb(cgroup.name()));
  }

  auto container1 = flow.container1_override().valid() ? flow.container1_override() : flow.container1();
  auto container2 = flow.container2_override().valid() ? flow.container2_override() : flow.container2();

  // send container information if it has changed
  if (container1.valid()) {
    container1_updater_.send_container_info(flow, container1, FlowSide::SIDE_A);
  }
  if (container2.valid()) {
    container2_updater_.send_container_info(flow, container2, FlowSide::SIDE_B);
  }

  return flow;
}

////////////////////////////////////////////////////////////////////////////////

void FlowUpdater::tcp_update(u64 timestamp, ::ebpf_net::metrics::tcp_metrics_point &metrics, int is_rx)
{
  auto flow = get_flow_for_update();

  if (!flow.valid()) {
    return;
  }

  flow.tcp_update(
      (u8)side_,
      is_rx,
      metrics.active_sockets,
      metrics.sum_retrans,
      metrics.sum_bytes,
      metrics.sum_srtt,
      metrics.sum_delivered,
      metrics.active_rtts,
      metrics.syn_timeouts,
      metrics.new_sockets,
      metrics.tcp_resets);
}

void FlowUpdater::udp_update(u64 timestamp, ::ebpf_net::metrics::udp_metrics_point &metrics, int is_rx)
{
  auto flow = get_flow_for_update();

  if (!flow.valid()) {
    return;
  }

  flow.udp_update(
      (u8)side_, is_rx, metrics.active_sockets, metrics.addr_changes, metrics.packets, metrics.bytes, metrics.drops);
}

void FlowUpdater::http_update(u64 timestamp, ::ebpf_net::metrics::http_metrics_point &metrics, u8 client_server)
{
  auto flow = get_flow_for_update();

  if (!flow.valid()) {
    return;
  }

  flow.http_update(
      (u8)side_,
      client_server,
      metrics.active_sockets,
      metrics.sum_code_200,
      metrics.sum_code_400,
      metrics.sum_code_500,
      metrics.sum_code_other,
      metrics.sum_total_time_ns,
      metrics.sum_processing_time_ns);
}

void FlowUpdater::dns_update(u64 timestamp, const ::ebpf_net::metrics::dns_metrics_point &metrics, u8 client_server)
{
  auto flow = get_flow_for_update();

  if (!flow.valid()) {
    return;
  }

  flow.dns_update(
      (u8)side_,
      client_server,
      metrics.active_sockets,
      metrics.requests_a,
      metrics.requests_aaaa,
      metrics.responses,
      metrics.timeouts,
      metrics.sum_total_time_ns,
      metrics.sum_processing_time_ns);
}

} // namespace reducer::ingest
