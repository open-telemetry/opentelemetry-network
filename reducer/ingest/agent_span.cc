// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/ingest/agent_span.h>
#include <reducer/ingest/component.h>
#include <reducer/ingest/shared_state.h>

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <reducer/uid_key.h>

#include <common/kernel_headers_source.h>
#include <common/linux_distro.h>
#include <common/operating_system.h>

#include <generated/ebpf_net/ingest/handles.h>
#include <generated/ebpf_net/ingest/modifiers.h>
#include <generated/ebpf_net/ingest/spans.h>
#include <generated/ebpf_net/ingest/weak_refs.h>

#include <util/environment_variables.h>
#include <util/ip_address.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/lookup3.h>
#include <util/short_string.h>

#include <config.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <tuple>

namespace reducer::ingest {

std::atomic<u64> AgentSpan::agent_counter(0);

namespace {

u64 create_random_agent_id()
{
  static thread_local auto *const rng = new std::mt19937_64(std::random_device()());
  return std::uniform_int_distribution<u64>()(*rng);
}

} // namespace

AgentSpan::AgentSpan()
    : agent_id_(create_random_agent_id()),
      hostname_(kUnknown),
      namespace_(kUnknown),
      cluster_(kUnknown),
      node_id_("unknown-" + std::to_string(++agent_counter)),
      node_az_(kUnknown),
      node_role_(kUnknown),
      instance_type_(kUnknown)
{}

AgentSpan::~AgentSpan()
{
  // TODO: now that the local index is thread local, this clean up must be done
  // in the agent thread
  //       as it stands, this destructor is broken and will fail assertion on
  //       core destruction

  /* clean up all the k8s_pod handles before clearing the map */
  for (auto &it : k8s_pods_) {
    it.second.put(*local_index());
  }

  auto &addr_map = global_private_to_public_address_map();
  for (auto private_addr : private_mapped_addrs_) {
    addr_map.erase(private_addr);
  }
}

void AgentSpan::process_steady_state(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__process_steady_state *msg)
{
  LOG::debug("--------PROCESS STEADY STATE---------");
}

void AgentSpan::socket_steady_state(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__socket_steady_state *msg)
{
  LOG::debug("--------SOCKET STEADY STATE---------");
  is_socket_steady_state_ = true;
}

void AgentSpan::version_info(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__version_info *msg)
{
  version_.set(msg->major, msg->minor, msg->patch);

  LOG::info("agent version: {}", version_);

  if (version_ < versions::client::MINIMUM_ACCEPTED_VERSION) {
    throw std::runtime_error("version is lower than minimum");
  }
}

void AgentSpan::cloud_platform(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__cloud_platform *msg)
{
  cloud_platform_ = sanitize_enum(static_cast<CloudPlatform>(msg->cloud_platform));
}

void AgentSpan::cloud_platform_account_info(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__cloud_platform_account_info *msg)
{
  cloud_platform_account_id_ = msg->account_id;
}

void AgentSpan::collector_health(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__collector_health *msg)
{
  status_ = sanitize_enum(static_cast<::collector::CollectorStatus>(msg->status));
  status_detail_ = msg->detail;
}

void AgentSpan::system_wide_process_settings(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__system_wide_process_settings *msg)
{
  LOG::trace_in(
      Component::agent,
      "agent_span::system_wide_process_settings"
      " clock_ticks_per_second={}"
      " memory_page_bytes={}",
      msg->clock_ticks_per_second,
      msg->memory_page_bytes);
  clock_ticks_per_second_ = msg->clock_ticks_per_second;
  memory_page_bytes_ = msg->memory_page_bytes;
}

void AgentSpan::report_cpu_cores(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__report_cpu_cores *msg)
{
  cpu_cores_ = msg->cpu_core_count;
}

void AgentSpan::collect_blob(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__collect_blob *msg)
{
  blob_collector_.collect(msg->blob_type, msg->subtype, msg->metadata, msg->blob);
}

void AgentSpan::set_node_info(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_node_info *msg)
{
  // node az, role, id, namespace, and cluster are pre-set to unknown on construction as a fallback when resolution fails

  if (!namespace_override_.empty()) {
    // use namespace label override if available
    namespace_ = namespace_override_;
  }

  if (!cluster_override_.empty()) {
    // use cluster label override if available
    cluster_ = cluster_override_;
  }

  if (!zone_override_.empty()) {
    // use zone label override if available
    node_az_ = zone_override_;
  } else if (auto const az = msg->az.string_view(); !az.empty()) {
    // otherwise use given az if available
    node_az_.assign(az.data(), az.size());
  }

  if (!service_override_.empty()) {
    // use service label override if available
    node_role_ = service_override_;
  } else if (auto const role = msg->role.string_view(); !role.empty()) {
    // otherwise use given role if available
    node_role_.assign(role.data(), role.size());
  }

  if (!host_override_.empty()) {
    // use host label override if available
    node_id_ = host_override_;
  } else if (hostname_ != kUnknown) {
    // otherwise use hostname if available
    node_id_ = hostname_;
  } else if (auto const id = msg->instance_id.string_view(); !id.empty()) {
    // otherwise use given id if available
    node_id_.assign(id.data(), id.size());
  }

  if (auto const instance_type = msg->instance_type.string_view(); !instance_type.empty()) {
    instance_type_.assign(instance_type.data(), instance_type.size());
  }
}

void AgentSpan::set_config_label(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_config_label *msg)
{
  auto const key = msg->key.string_view();
  auto const value = msg->value.string_view();

  LOG::trace_in(Component::agent, "custom config label: '{}'='{}'", key, value);

  if (key == "namespace") {
    LOG::trace_in(Component::agent, "\"namespace\" override detected!");
    namespace_override_.assign(value.data(), value.size());
  }
  if (key == "environment" || key == "cluster") {
    LOG::trace_in(Component::agent, "\"cluster\" override detected!");
    cluster_override_.assign(value.data(), value.size());
  }
  if (key == "service") {
    LOG::trace_in(Component::agent, "\"service\" override detected!");
    service_override_.assign(value.data(), value.size());
  }
  if (key == "host") {
    LOG::trace_in(Component::agent, "\"host\" override detected!");
    host_override_.assign(value.data(), value.size());
  }
  if (key == "zone") {
    LOG::trace_in(Component::agent, "\"zone\" override detected!");
    zone_override_.assign(value.data(), value.size());
  }
}

void AgentSpan::set_availability_zone_deprecated(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_availability_zone_deprecated *msg)
{
  std::string az;

  if (!zone_override_.empty()) {
    // always use zone label for az, if it was defined
    az = zone_override_;
  } else if (msg->retcode == 0) {
    az.assign((char *)msg->az, strnlen((char *)msg->az, sizeof(msg->az)));
    LOG::trace_in(Component::agent, "AgentSpan::set_availability_zone_deprecated: az='{}'", az);
  } else {
    LOG::trace_in(Component::agent, "AgentSpan::set_availability_zone_deprecated: retcode={}", msg->retcode);
  }

  if (!az.empty()) {
    LOG::trace_in(Component::agent, "Node az='{}', agent={}", az, agent_id_);
    node_az_ = az;
  }
}

void AgentSpan::set_iam_role_deprecated(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_iam_role_deprecated *msg)
{
  std::string role;

  if (!service_override_.empty()) {
    // always use service label for role, if it was defined
    role = service_override_;
  } else if (msg->retcode == 0) {
    role.assign((char *)msg->role, strnlen((char *)msg->role, sizeof(msg->role)));
    LOG::trace_in(Component::agent, "AgentSpan::set_iam_role_deprecated: role='{}'", role);
  } else {
    LOG::trace_in(Component::agent, "AgentSpan::set_iam_role_deprecated: retcode={}", msg->retcode);
  }

  if (!role.empty()) {
    LOG::trace_in(Component::agent, "Node role='{}', agent={}", role, agent_id_);
    node_role_ = role;
  }
}

void AgentSpan::set_instance_id_deprecated(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_instance_id_deprecated *msg)
{
  std::string id;

  if (!host_override_.empty()) {
    // always use host label for id, if it was defined
    id = host_override_;
  } else if (!hostname_.empty()) {
    id = hostname_;
  } else if (msg->retcode == 0) {
    id = "i-";
    id.append((char *)msg->id, strnlen((char *)msg->id, sizeof(msg->id)));
    LOG::trace_in(Component::agent, "AgentSpan::set_instance_id_deprecated: id='{}'", id);
  } else {
    LOG::trace_in(Component::agent, "AgentSpan::set_instance_id_deprecated: retcode={}", msg->retcode);
  }

  if (!id.empty()) {
    LOG::trace_in(Component::agent, "Node id='{}', agent={}", id, agent_id_);
    node_id_ = id;
  }
}

void AgentSpan::set_instance_type_deprecated(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_instance_type_deprecated *msg)
{
  if (msg->retcode == 0) {
    instance_type_.assign((char *)msg->val, strnlen((char *)msg->val, sizeof(msg->val)));
  }
}

void AgentSpan::dns_response_fake(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__dns_response_fake *msg)
{
  in_addr *ipv4_addrs = (in_addr *)msg->ips.buf;
  int num_ipv4_addrs = msg->ips.len / sizeof(in_addr);

  map_ips_to_domain(ipv4_addrs, num_ipv4_addrs, nullptr, 0, {msg->domain_name.buf, msg->domain_name.len});
}

void AgentSpan::dns_response_dep_a_deprecated(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__dns_response_dep_a_deprecated *msg)
{
  /* copy because we're not sure of message alignment */

  int num_ipv4_addrs = msg->ipv4_addrs.len / sizeof(in_addr);
  in_addr ipv4_addrs[num_ipv4_addrs];
  memcpy(ipv4_addrs, msg->ipv4_addrs.buf, sizeof(in_addr) * num_ipv4_addrs);

  int num_ipv6_addrs = msg->ipv6_addrs.len / sizeof(in6_addr);
  in6_addr ipv6_addrs[num_ipv6_addrs];
  memcpy(ipv6_addrs, msg->ipv6_addrs.buf, sizeof(in6_addr) * num_ipv6_addrs);

  map_ips_to_domain(ipv4_addrs, num_ipv4_addrs, ipv6_addrs, num_ipv6_addrs, {msg->domain_name.buf, msg->domain_name.len});
}

void AgentSpan::map_ips_to_domain(
    in_addr *ipv4_addrs, int num_ipv4_addrs, in6_addr *ipv6_addrs, int num_ipv6_addrs, std::string_view domain_name)
{
  static constexpr auto max_len = dns::dns_record::max_len;

  char const *dn_buf = domain_name.data();
  size_t dn_len = domain_name.length();

  /* if record is too big, truncate the beginning */
  if (dn_len > max_len) {
    dn_buf += dn_len - max_len;
    dn_len = max_len;
  }

  /* prepare DNS record */
  dns::dns_record rec(dn_buf, dn_len);

  /* prepare an IPv6-mapped IPv4 address */
  struct in6_addr addr = {};
  addr.s6_addr16[5] = 0xffff;

  /* add LRU for each of the reported IP addresses */

  for (int i = 0; i < num_ipv4_addrs; ++i) {
    addr.s6_addr32[3] = ipv4_addrs[i].s_addr;
    map_ip_to_domain(addr, rec);
  }

  for (int i = 0; i < num_ipv6_addrs; ++i) {
    map_ip_to_domain(ipv6_addrs[i], rec);
  }
}

void AgentSpan::map_ip_to_domain(in6_addr const &ip_addr, dns::dns_record const &rec)
{
  LOG::trace_in(
      Component::agent, "AgentSpan::map_ip_to_domain: ip_addr={}, rec={}", IPv6Address::from(ip_addr), rec.to_string_view());

  auto const addr = IPv6Address::from(ip_addr);

  /* is the IP already in the LRU? */
  auto *found = ip_to_domain_.find(addr);
  if (found != nullptr) {
    /* remove the old entry */
    LOG::trace_in(Component::agent, "reducer::AgentSpan::map_ip_to_domain - removing old entry");
    ip_to_domain_.remove(addr);
  }

  if (rec.len == 0) {
    LOG::warn("attempting to insert an empty DNS record for ip {}", addr);
  }

  /* insert */
  auto *inserted = ip_to_domain_.insert(addr, rec);
  if (inserted == nullptr) {
    local_logger().failed_to_insert_dns_record();
  }

  LOG::trace_in(Component::agent, "reducer::AgentSpan::map_ip_to_domain - LRU.size = {}", ip_to_domain_.size());
}

void AgentSpan::set_config_label_deprecated(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_config_label_deprecated *msg)
{
  jsrv_ingest__set_config_label message;
  message.key = render_array_to_string_view(msg->key);
  message.value = render_array_to_string_view(msg->val);
  set_config_label(span_ref, timestamp, &message);
}

void AgentSpan::span_duration_info(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__span_duration_info *msg)
{
  // OBSOLETED
}

void AgentSpan::connect(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__connect *msg)
{
  auto const npm_connection = local_connection();
  assert(npm_connection);

  if (msg->hostname.len) {
    hostname_.assign(msg->hostname.buf, msg->hostname.len);
  }

  type_ = sanitize_enum(static_cast<ClientType>(msg->collector_type));

  npm_connection->set_client_info(hostname_, type_);

  auto const connection = npm_connection->ingest_connection();
  connection->on_connection_authenticated();
}

void AgentSpan::os_info_deprecated(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__os_info_deprecated *msg)
{
  auto const os = sanitize_enum(static_cast<OperatingSystem>(msg->os));
  os_ = to_string(os);

  switch (os) {
  case OperatingSystem::Linux:
    os_flavor_ = to_string(static_cast<LinuxDistro>(msg->flavor));
    break;

  default:
    os_flavor_ = "unknown";
    break;
  }

  if (!msg->kernel_version.empty()) {
    kernel_version_ = msg->kernel_version.to_string();
  }
}

void AgentSpan::os_info(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__os_info *msg)
{
  auto const os = sanitize_enum(static_cast<OperatingSystem>(msg->os));
  os_ = to_string(os);

  switch (os) {
  case OperatingSystem::Linux:
    os_flavor_ = to_string(static_cast<LinuxDistro>(msg->flavor));
    break;

  default:
    os_flavor_ = "unknown";
    break;
  }

  if (!msg->os_version.empty()) {
    os_version_ = msg->os_version.to_string();
  }

  if (!msg->kernel_version.empty()) {
    kernel_version_ = msg->kernel_version.to_string();
  }
}

void AgentSpan::kernel_headers_source(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__kernel_headers_source *msg)
{
  kernel_headers_source_ = to_string(static_cast<KernelHeadersSource>(msg->source));
}

void AgentSpan::entrypoint_error(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__entrypoint_error *msg)
{
  LOG::error(
      "error while booting up client {} at {} running under OS {}/{}: {}", type_, hostname_, os_, os_flavor_, msg->error);
}

void AgentSpan::health_check(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__health_check *msg)
{
  type_ = sanitize_enum(static_cast<ClientType>(msg->client_type));
  if (msg->origin.len) {
    hostname_.assign(msg->origin.buf, msg->origin.len);
  }

  LOG::trace_in(type_, "AgentSpan::health_check from client {} at '{}'", type_, hostname_);

  auto const npm_connection = local_connection();
  assert(npm_connection);
  npm_connection->set_client_info(hostname_, type_);
}

// XXX: currently all {private/public}_ipv4 and ipv6 messages come with
// a vpc_id, which gets ignored.
void AgentSpan::private_ipv4_addr(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__private_ipv4_addr *msg)
{
  auto private_addr = IPv4Address::from(msg->addr).to_ipv6();
  host_ips_.insert(private_addr);

  // check that this ipv4 address was not used in a private-to-public mapping
  auto &addr_map = global_private_to_public_address_map();
  if (auto existing_public = addr_map.get(private_addr); existing_public) {
    local_logger().private_ip_in_private_to_public_ip_mapping(jb_blob(private_addr.str()), jb_blob(existing_public->str()));
  }
}

void AgentSpan::ipv6_addr(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__ipv6_addr *msg)
{

  auto private_addr = IPv6Address::from(msg->addr);
  host_ips_.insert(private_addr);

  // check that the ipv6 address is not used as a private address in the mapping
  auto &addr_map = global_private_to_public_address_map();
  if (auto existing_public = addr_map.get(private_addr); existing_public) {
    local_logger().private_ip_in_private_to_public_ip_mapping(jb_blob(private_addr.str()), jb_blob(existing_public->str()));
  }
}

void AgentSpan::public_to_private_ipv4(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__public_to_private_ipv4 *msg)
{
  auto public_addr = IPv4Address::from(msg->public_addr).to_ipv6();
  auto private_addr = IPv4Address::from(msg->private_addr).to_ipv6();

  LOG::trace_in(Component::agent, "AgentSpan::public_to_private: public_addr={}, private_addr={}", public_addr, private_addr);

  host_ips_.insert(public_addr);
  host_ips_.insert(private_addr);

  private_mapped_addrs_.insert(private_addr);

  auto &addr_map = global_private_to_public_address_map();

  if (auto existing_public = addr_map.get(private_addr); existing_public && !(*existing_public == public_addr)) {
    local_logger().rewriting_private_to_public_ip_mapping(
        jb_blob(private_addr.str()), jb_blob(existing_public->str()), jb_blob(public_addr.str()));
  }

  addr_map.insert(private_addr, public_addr);
}

bool AgentSpan::is_host_address(IPv6Address const &addr) const
{
  return host_ips_.contains(addr);
}

void AgentSpan::metadata_complete(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__metadata_complete *msg)
{
  std::cout << "--------METADATA COMPLETE---------\n";
  std::cout << "Agent info:\n";
  std::cout << "\tVersion:               " << version_ << '\n';
  std::cout << "\tOS:                    " << os_ << " (" << os_flavor_ << ")\n";
  std::cout << "\tKernel:                " << kernel_version_ << '\n';
  std::cout << "\tCPU Cores:             " << cpu_cores_ << '\n';
  std::cout << "\tHostname:              " << hostname_ << '\n';
  std::cout << "\tCollector:             " << type_ << '\n';
  if (type_ == ClientType::kernel) {
    std::cout << "\tKernel Headers Source: " << kernel_headers_source_ << '\n';
  }
  std::cout << "\tEntrypoint Error:      " << entrypoint_error_ << '\n';
  std::cout << "\tRole:                  " << node_role_ << '\n';
  std::cout << "\tAZ:                    " << node_az_ << '\n';
  std::cout << "\tId:                    " << node_id_ << '\n';
  std::cout << "\tInstance:              " << instance_type_ << '\n';
  std::cout << "\tAgent:                 " << agent_id_ << '\n';
  std::cout << "\tOverrides:\n";
  std::cout << "\t\tnamespace:   " << namespace_override_ << '\n';
  std::cout << "\t\tcluster:     " << cluster_override_ << '\n';
  std::cout << "\t\tservice:     " << service_override_ << '\n';
  std::cout << "\t\thost:        " << host_override_ << '\n';
  std::cout << "\t\tzone:        " << zone_override_ << '\n';
  std::cout << "\tIPs:\n";
  for (auto &addr : host_ips_) {
    std::cout << "\t\t" << addr << '\n';
  }
  std::cout << "Metadata Report Complete." << std::endl;
}

void AgentSpan::heartbeat(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__heartbeat *msg)
{
  // TODO: 1. add more descriptive debug message.
  //       2. add functionality in reducer such that it disconnects if
  //          heartbeat does not come back regularly.
  LOG::trace_in(Component::heartbeat, "Got a heartbeat from {} at '{}' (timestamp={})", type_, hostname_, timestamp);
}

void AgentSpan::agent_resource_usage(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__agent_resource_usage *msg)
{}

void AgentSpan::bpf_lost_samples(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__bpf_lost_samples *msg)
{
  const auto conn = local_connection();

  local_logger().agent_lost_events(msg->count, jb_blob(conn->client_hostname()));
}

void AgentSpan::set_pod_new(
    std::string_view uid,
    std::string_view owner_name,
    std::string_view owner_uid,
    std::string_view pod_name,
    std::string_view ns,
    std::string_view version,
    uint32_t ip)
{
  ::ebpf_net::ingest::keys::k8s_pod k8s_pod_key;

  // Get the key for k8s_pod
  u64 uid_u64 = uid_to_u64(uid);
  k8s_pod_key.uid_hash = uid_u64;
  uid_suffix(uid, k8s_pod_key.uid_suffix.data(), k8s_pod_key.uid_suffix.max_size());

  // get the k8s_pod span
  auto k8s_pod = local_index()->k8s_pod.by_key(k8s_pod_key);
  if (!k8s_pod.valid()) {
    LOG::trace_in(Component::k8s_pod, "no space for new k8s pod in AgentSpan::set_pod_new");
    return; // no space.
  }

  LOG::trace_in(
      std::make_tuple(NodeResolutionType::K8S_CONTAINER, ClientType::k8s),
      "set_pod_new: uid {}, ip {:x}, uid_hash {}, owner_name {}, ns {}, version {}",
      uid,
      ip,
      uid_u64,
      owner_name,
      ns,
      version);

  // populate the pod span with pod metadata
  k8s_pod.set_pod_detail(jb_blob(owner_name), jb_blob{pod_name}, jb_blob(ns), jb_blob(version), jb_blob(owner_uid));

  // Add pod to k8s_pod_set if it isn't already there.

  // TODO: remove the delete_k8s_pod below

  // Occasionally the k8s collector will die and restart without sending
  // a `pod_delete` message. We must proactively delete the previous `k8s_pod`
  // handle otherwise the handle we attempt to insert below will be destroyed
  // without a corresponding `put()`, which will result in a broken assertion
  // in render-generated code.
  delete_k8s_pod(uid_u64);

  [[maybe_unused]] auto res = k8s_pods_.emplace(uid_u64, std::move(k8s_pod.to_handle()));

  // Should not occur given the above check, but verify anyway.
  assert(res.second);
}

void AgentSpan::pod_new_legacy(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_new_legacy *msg)
{
  set_pod_new(msg->uid, msg->owner_name, msg->owner_uid, {}, msg->ns, {}, msg->ip);
}

void AgentSpan::pod_new_legacy2(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_new_legacy2 *msg)
{
  set_pod_new(msg->uid, msg->owner_name, msg->owner_uid, {}, msg->ns, msg->version, msg->ip);
}

void AgentSpan::pod_new_with_name(
    ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_new_with_name *msg)
{
  set_pod_new(msg->uid, msg->owner_name, msg->owner_uid, msg->pod_name, msg->ns, msg->version, msg->ip);
}

void AgentSpan::pod_container(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_container *msg)
{
  const std::string uid(msg->uid.buf, msg->uid.len);
  const std::string container_id(msg->container_id.buf, msg->container_id.len);
  const std::string container_name(msg->container_name.buf, msg->container_name.len);
  const std::string container_image(msg->container_image.buf, msg->container_image.len);

  LOG::trace_in(Component::k8s_pod, "AgentSpan::pod_container: uid={} container_id={}", uid, container_id);

  // compute key for the owner k8s_pod
  ::ebpf_net::ingest::keys::k8s_pod k8s_pod_key;
  k8s_pod_key.uid_hash = uid_to_u64(uid);
  uid_suffix(uid, k8s_pod_key.uid_suffix.data(), k8s_pod_key.uid_suffix.max_size());

  // get the owning k8s_pod
  auto k8s_pod = local_index()->k8s_pod.by_key(k8s_pod_key);
  if (!k8s_pod.valid()) {
    LOG::trace_in(Component::k8s_pod, "AgentSpan::pod_container: failed to reference a pod (uid={})", uid);
    return;
  }

  // notify the owner pod's span of the new container
  k8s_pod.impl().pod_container(k8s_pod, container_id, container_name, container_image);
}

void AgentSpan::pod_delete(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_delete *msg)
{
  const std::string uid(msg->uid.buf, msg->uid.len);

  LOG::trace_in(std::make_tuple(NodeResolutionType::K8S_CONTAINER, ClientType::k8s), "pod_delete: uid {}", uid);

  // When the message signifies removal of the pod, call put()
  // on the handle and remove it from k8s_pod_set. This will
  // clean up the endpoint and node handles
  const u64 uid_u64 = uid_to_u64(uid);
  if (!delete_k8s_pod(uid_u64)) {
    local_logger().pod_not_found(jb_blob(uid), (u8) true);
  }
}

void AgentSpan::pod_resync(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_resync *msg)
{
  const uint64_t resync_count = msg->resync_count;

  LOG::trace_in(std::make_tuple(NodeResolutionType::K8S_CONTAINER, ClientType::k8s), "Pod resync {}", resync_count);

  // put all the k8s pod handles that were added by this agent
  for (auto &iter : k8s_pods_) {
    iter.second.put(*local_index());
  }

  // clear the map of handles to get a clean slate for future updates
  k8s_pods_.clear();
}

std::optional<std::string_view> AgentSpan::find_dns_for_ip(const IPv6Address &addr)
{
  /* is the IP already in the LRU? */
  auto *found = ip_to_domain_.find(addr);
  if (found == nullptr) {
    return std::nullopt;
  }

  return found->to_string_view();
}

bool AgentSpan::delete_k8s_pod(const uint64_t uid_u64)
{
  if (auto pod_it = k8s_pods_.find(uid_u64); pod_it != k8s_pods_.end()) {
    // pod exists, delete it
    pod_it->second.put(*local_index());
    k8s_pods_.erase(pod_it);
    return true;
  } else {
    // Doesn't exist.
    return false;
  }
}

void AgentSpan::log_message(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__log_message *msg)
{
  auto const level = static_cast<spdlog::level::level_enum>(msg->log_level);

  switch (level) {
  case spdlog::level::info: {
    ++log_count_.info;
    LOG::info("client log from {} collector {} at '{}': {}", type_, version_, hostname_, msg->message.string_view());
    break;
  }

  case spdlog::level::warn: {
    ++log_count_.warning;
    LOG::warn("client log from {} collector {} at '{}': {}", type_, version_, hostname_, msg->message.string_view());
    break;
  }

  case spdlog::level::err: {
    ++log_count_.error;
    LOG::error("client log from {} collector {} at '{}': {}", type_, version_, hostname_, msg->message.string_view());
    break;
  }

  case spdlog::level::critical: {
    ++log_count_.critical;
    LOG::critical("client log from {} collector {} at '{}': {}", type_, version_, hostname_, msg->message.string_view());
    break;
  }

  default: {
    ++log_count_.ignored;
    LOG::trace_in(type_, "ignored log message with level={}: {}", msg->log_level, msg->message.string_view());
    break;
  }
  }
}

void AgentSpan::bpf_log(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__bpf_log *msg)
{
  bpf_logs_.emplace_back(bpf_log_entry{
      .timestamp = std::chrono::nanoseconds{timestamp},
      .filename = msg->filename.to_string(),
      .line = msg->line,
      .code = msg->code,
      .arg0 = msg->arg0,
      .arg1 = msg->arg1,
      .arg2 = msg->arg2,
  });
}

void AgentSpan::write_internal_stats(
    ::ebpf_net::ingest::weak_refs::ingest_core_stats ingest_core_stats, u64 time_ns, int shard, std::string_view module)
{
  std::stringstream ss;
  ss << version();
  std::string version_as_string = ss.str();

  ingest_core_stats.collector_health_stats(
      jb_blob(module),
      shard,
      jb_blob(version_as_string),
      jb_blob(std::to_string(integer_value(cloud_platform()))),
      jb_blob(cluster()),
      jb_blob(role()),
      jb_blob(node_az()),
      jb_blob(node_az()),
      jb_blob(kernel_version()),
      integer_value(client_type()),
      jb_blob(hostname()),
      jb_blob(os()),
      jb_blob(os_version()),
      time_ns,
      jb_blob(std::to_string(integer_value(status_))),
      jb_blob(std::to_string(status_detail_)));

  for (auto const &data : bpf_logs_) {
    ingest_core_stats.bpf_log_stats(
        jb_blob(module),
        shard,
        jb_blob(version_as_string),
        jb_blob(std::to_string(integer_value(cloud_platform()))),
        jb_blob(cluster()),
        jb_blob(role()),
        jb_blob(node_az()),
        jb_blob(node_id()),
        jb_blob(kernel_version()),
        integer_value(client_type()),
        jb_blob(hostname()),
        jb_blob(os()),
        jb_blob(os_version()),
        time_ns,
        jb_blob(data.filename),
        jb_blob(std::to_string(data.line)),
        jb_blob(std::to_string(data.code)),
        jb_blob(std::to_string(data.arg0)),
        jb_blob(std::to_string(data.arg1)),
        jb_blob(std::to_string(data.arg2)));
  }

  bpf_logs_.clear();
}

thread_local BlobCollector AgentSpan::blob_collector_;

} // namespace reducer::ingest
