// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "agg_root_span.h"
#include "agg_core.h"

#include <reducer/constants.h>
#include <reducer/copy_metrics.h>

#include <util/error_handling.h>
#include <util/log.h>

#include <generated/ebpf_net/aggregation/index.h>
#include <generated/ebpf_net/aggregation/modifiers.h>
#include <generated/ebpf_net/aggregation/spans.h>

namespace reducer::aggregation {

namespace {

template <typename T> T as_short_string(jb_blob const &value, std::size_t &truncation_counter)
{
  static_assert(std::is_same_v<T, short_string<T::max_len>>, "T must be a short_string");

  if (value.len > T::max_len) {
    ++truncation_counter;
  }

  return T::truncate(value);
}

} // namespace

AggRootSpan::AggRootSpan() {}

AggRootSpan::~AggRootSpan() {}

void AggRootSpan::update_node(
    ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_node *msg)
{
  ASSUME(msg->side <= 1).else_log("side must be either 0 or 1, instead got {}", msg->side);

  auto &stat_counters = local_core<AggCore>().stat_counters;

  using id_t = ::ebpf_net::aggregation::spans::node::id_t;
  auto id = as_short_string<id_t>(msg->id, stat_counters.trunc_id);

  using ip_t = ::ebpf_net::aggregation::spans::node::ip_t;
  ip_t ip;
  if (!AggCore::node_ip_field_disabled()) {
    ip = as_short_string<ip_t>(msg->address, stat_counters.trunc_ip);
  }

  using az_t = ::ebpf_net::aggregation::spans::az::s_t;
  auto az = as_short_string<az_t>(msg->az, stat_counters.trunc_az);

  using role_t = ::ebpf_net::aggregation::spans::role::s_t;
  auto role = as_short_string<role_t>(msg->role, stat_counters.trunc_role);

  using version_t = ::ebpf_net::aggregation::spans::role::version_t;
  auto version = as_short_string<version_t>(msg->version, stat_counters.trunc_version);

  using env_t = ::ebpf_net::aggregation::spans::role::env_t;
  auto env = as_short_string<env_t>(msg->env, stat_counters.trunc_env);

  using ns_t = ::ebpf_net::aggregation::spans::role::ns_t;
  auto ns = as_short_string<ns_t>(msg->ns, stat_counters.trunc_ns);

  using node_type_t = ::ebpf_net::aggregation::spans::role::node_type_t;
  node_type_t node_type{msg->node_type};

  using process_t = ::ebpf_net::aggregation::spans::role::process_t;
  auto process = as_short_string<process_t>(msg->process, stat_counters.trunc_process);

  using container_t = ::ebpf_net::aggregation::spans::role::container_t;
  auto container = as_short_string<container_t>(msg->container, stat_counters.trunc_container);

  using pod_name_t = ::ebpf_net::aggregation::spans::node::pod_name_t;
  auto pod_name = as_short_string<pod_name_t>(msg->pod_name, stat_counters.trunc_pod_name);

  auto &index = span_ref.index();

  auto role_ref = index.role.by_key({role, version, env, ns, node_type, process, container});
  if (!role_ref.valid()) {
    return;
  }

  auto az_ref = index.az.by_key({az, role_ref.loc()});
  if (!az_ref.valid()) {
    return;
  }

  auto node_ref = index.node.by_key({id, ip, az_ref.loc()});
  if (!node_ref.valid()) {
    return;
  }

  // Assumes each node can only have one pod. Otherwise this should be part
  // of the key.
  node_ref.modify().pod_name(pod_name);

  if (msg->side == 0) {
    span_ref.modify().node1(std::move(node_ref));
  } else {
    span_ref.modify().node2(std::move(node_ref));
  }
}

void AggRootSpan::update_tcp_metrics(
    ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_tcp_metrics *msg)
{
  ::ebpf_net::metrics::tcp_metrics_point m;
  copy_tcp_metrics(m, *msg);

  auto direction = static_cast<UpdateDirection>(msg->direction);

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.tcp_a_to_b_update(timestamp, m);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.tcp_b_to_a_update(timestamp, m);
  }
}

void AggRootSpan::update_udp_metrics(
    ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_udp_metrics *msg)
{
  ::ebpf_net::metrics::udp_metrics_point m;
  copy_udp_metrics(m, *msg);

  auto direction = static_cast<UpdateDirection>(msg->direction);

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.udp_a_to_b_update(timestamp, m);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.udp_b_to_a_update(timestamp, m);
  }
}

void AggRootSpan::update_http_metrics(
    ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_http_metrics *msg)
{
  ::ebpf_net::metrics::http_metrics_point m;
  copy_http_metrics(m, *msg);

  auto direction = static_cast<UpdateDirection>(msg->direction);

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.http_a_to_b_update(timestamp, m);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.http_b_to_a_update(timestamp, m);
  }
}

void AggRootSpan::update_dns_metrics(
    ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_dns_metrics *msg)
{
  ::ebpf_net::metrics::dns_metrics_point m;
  copy_dns_metrics(m, *msg);

  auto direction = static_cast<UpdateDirection>(msg->direction);

  if (direction == UpdateDirection::A_TO_B) {
    span_ref.dns_a_to_b_update(timestamp, m);
  } else if (direction == UpdateDirection::B_TO_A) {
    span_ref.dns_b_to_a_update(timestamp, m);
  }
}

} // namespace reducer::aggregation
