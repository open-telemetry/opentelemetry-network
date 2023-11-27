/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/dns_cache.h>
#include <reducer/util/blob_collector.h>

#include <common/client_type.h>
#include <common/cloud_platform.h>
#include <common/collector_status.h>

#include <generated/ebpf_net/ingest/auto_handles.h>
#include <generated/ebpf_net/ingest/handles.h>
#include <generated/ebpf_net/ingest/keys.h>
#include <generated/ebpf_net/ingest/parsed_message.h>
#include <generated/ebpf_net/ingest/span_base.h>

#include <platform/platform.h>

#include <util/expected.h>
#include <util/resource_usage.h>
#include <util/time.h>
#include <util/version.h>

#include <absl/container/flat_hash_set.h>

#include <chrono>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

namespace reducer {
class InternalMetricsEncoder;
class SpanTest;
} // namespace reducer

namespace reducer::ingest {

class AgentSpan : public ::ebpf_net::ingest::AgentSpanBase {
public:
  /* DNS cache sizes */
  static constexpr u32 dns_lru__pool_size = 100'000;

  typedef DnsCache<dns_lru__pool_size> dns_cache_type;

  AgentSpan(); // Auto-generates a random agent id.

  ~AgentSpan();

  /* handlers
   */
  void
  process_steady_state(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__process_steady_state *msg);
  void socket_steady_state(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__socket_steady_state *msg);
  void version_info(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__version_info *msg);
  void cloud_platform(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__cloud_platform *msg);
  void cloud_platform_account_info(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__cloud_platform_account_info *msg);
  void collector_health(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__collector_health *msg);
  void system_wide_process_settings(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__system_wide_process_settings *msg);
  void report_cpu_cores(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__report_cpu_cores *msg);
  void collect_blob(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__collect_blob *msg);
  void set_node_info(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_node_info *msg);
  void set_config_label(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_config_label *msg);
  void set_availability_zone_deprecated(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_availability_zone_deprecated *msg);
  void set_iam_role_deprecated(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_iam_role_deprecated *msg);
  void set_instance_id_deprecated(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_instance_id_deprecated *msg);
  void set_instance_type_deprecated(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_instance_type_deprecated *msg);
  void dns_response_fake(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__dns_response_fake *msg);
  void dns_response_dep_a_deprecated(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__dns_response_dep_a_deprecated *msg);
  void set_config_label_deprecated(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__set_config_label_deprecated *msg);
  void span_duration_info(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__span_duration_info *msg);
  void connect(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__connect *msg);
  void os_info_deprecated(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__os_info_deprecated *msg);
  void os_info(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__os_info *msg);
  void
  kernel_headers_source(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__kernel_headers_source *msg);
  void entrypoint_error(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__entrypoint_error *msg);
  void health_check(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__health_check *msg);
  void private_ipv4_addr(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__private_ipv4_addr *msg);
  void ipv6_addr(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__ipv6_addr *msg);
  void public_to_private_ipv4(
      ::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__public_to_private_ipv4 *msg);
  void metadata_complete(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__metadata_complete *msg);
  void bpf_lost_samples(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__bpf_lost_samples *msg);
  void heartbeat(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__heartbeat *msg);
  void
  agent_resource_usage(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__agent_resource_usage *msg);

  // Handlers for arriving k8s-collector messages. These will be broadcast to
  // every agent, and the actual business logic will be performed in the
  // `pod_*_internal` messages below.
  void pod_new_legacy2(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_new_legacy2 *msg);
  void pod_new_legacy(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_new_legacy *msg);
  void pod_new_with_name(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_new_with_name *msg);
  void set_pod_new(
      std::string_view uid,
      std::string_view owner_name,
      std::string_view owner_uid,
      std::string_view pod_name,
      std::string_view ns,
      std::string_view version,
      uint32_t ip);
  void pod_container(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_container *msg);
  void pod_delete(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_delete *msg);
  void pod_resync(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__pod_resync *msg);

  // Handlers for k8s-collector messages. These will be invoked via the visitor
  // pattern on `TcpServer`.
  void
  pod_new_internal(std::string_view uid, std::string_view owner_name, std::string_view ns, uint32_t ip, bool is_host_network);
  void pod_container_internal(uint64_t timestamp_ns, std::string_view uid, std::string_view container_id);
  void pod_delete_internal(std::string_view uid);
  void pod_resync_internal(uint64_t resync_count);

  void log_message(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__log_message *msg);
  void bpf_log(::ebpf_net::ingest::weak_refs::agent span_ref, u64 timestamp, jsrv_ingest__bpf_log *msg);

  u64 agent_id() const { return agent_id_; }

  VersionInfo const &version() const { return version_; }

  std::string const &node_id() const { return node_id_; }
  std::string const &node_az() const { return node_az_; }
  std::string const &ns() const { return namespace_; }
  std::string const &cluster() const { return cluster_; }
  std::string const &role() const { return node_role_; }
  std::string const &instance_type() const { return instance_type_; }

  // Returns true if the Agent is in socket steady state.
  bool is_socket_steady_state() const { return is_socket_steady_state_; }

  struct LogCount {
    u32 ignored = 0;
    u32 info = 0;
    u32 warning = 0;
    u32 error = 0;
    u32 critical = 0;
  };

  LogCount const &log_count() const { return log_count_; }

  // Checks whether the supplied address is one of host's addresses.
  bool is_host_address(IPv6Address const &addr) const;

  // Finds most recent DNS query for the given IP
  // @returns: a string_view to the domain name, if found. The string_view is
  //   only valid temporarily (as long as no DNS messages are processed); user
  //   should copy the contents if they are to be preserved.
  std::optional<std::string_view> find_dns_for_ip(const IPv6Address &addr);

  // Adds IP->domain mappings.
  void map_ips_to_domain(
      in_addr *ipv4_addrs, int num_ipv4_addrs, in6_addr *ipv6_addrs, int num_ipv6_addrs, std::string_view domain_name);

  std::string const &hostname() const { return hostname_; }
  ClientType client_type() const { return type_; }
  std::string_view os() const { return os_; }
  std::string_view os_flavor() const { return os_flavor_; }
  std::string_view os_version() const { return os_version_; }
  std::string_view kernel_version() const { return kernel_version_; }
  std::string_view kernel_headers_source() const { return kernel_headers_source_; }
  CloudPlatform cloud_platform() const { return cloud_platform_; }
  std::string_view entrypoint_error() const { return entrypoint_error_; }
  std::uint32_t cpu_cores() const { return cpu_cores_; }

  std::string_view key_id() const { return key_id_; }

  ::collector::CollectorStatus status() const { return status_; }
  std::uint16_t status_detail() const { return status_detail_; }

  void write_internal_stats(
      ::ebpf_net::ingest::weak_refs::ingest_core_stats ingest_core_stats, u64 time_ns, int shard, std::string_view module);

  std::uint64_t clock_ticks_per_second() const { return clock_ticks_per_second_; }
  std::size_t memory_page_bytes() const { return memory_page_bytes_; }

private:
  friend class ::reducer::SpanTest;

  static std::atomic<u64> agent_counter;
  static const std::map<std::string, std::string> tenant_keys_;

  const u64 agent_id_;

  std::string hostname_;
  ClientType type_ = ClientType::unknown;
  std::string_view os_ = "unknown";
  std::string_view os_flavor_ = "unknown";
  std::string os_version_ = "unknown";
  std::string kernel_version_ = "unknown";
  std::string_view kernel_headers_source_ = "unknown";
  CloudPlatform cloud_platform_ = CloudPlatform::unknown;
  std::string_view entrypoint_error_;

  std::string namespace_;
  std::string cluster_;
  std::string node_id_;
  std::string node_az_;
  std::string node_role_;

  VersionInfo version_;

  std::string instance_type_;
  std::string namespace_override_;
  std::string cluster_override_;
  std::string service_override_;
  std::string host_override_;
  std::string zone_override_;

  // Addresses belonging to the host on which the agent is running.
  absl::flat_hash_set<IPv6Address> host_ips_;
  // Host's private addresses that are mapped to public addresses.
  absl::flat_hash_set<IPv6Address> private_mapped_addrs_;

  // maps pod_uid_to_u64(<the uid string of a pod>) to a corresponding k8s_pod
  // handle updated in: pod_new(): adds elements pod_delete(): removes elements
  std::unordered_map<u64, ::ebpf_net::ingest::handles::k8s_pod> k8s_pods_;

  dns_cache_type ip_to_domain_;

  bool is_socket_steady_state_ = false;

  ::collector::CollectorStatus status_ = ::collector::CollectorStatus::unknown;
  std::uint16_t status_detail_ = 0;

  std::string cloud_platform_account_id_;

  void map_ip_to_domain(in6_addr const &ip_addr, dns::dns_record const &rec);

  void maybe_disable_by_env_var();

  // Deletes the pod identified by `uid_hash` from `k8s_pods_`. Returns true
  // is the deletion was successful, otherwise returns false if there was
  // nothing to delete.
  bool delete_k8s_pod(uint64_t uid_hash);

  LogCount log_count_;

  std::uint64_t clock_ticks_per_second_ = {};
  std::size_t memory_page_bytes_ = {};
  std::uint32_t cpu_cores_ = 0;

  std::string key_id_;

  struct bpf_log_entry {
    std::chrono::nanoseconds timestamp;
    std::string filename;
    u32 line;
    u64 code;
    u64 arg0;
    u64 arg1;
    u64 arg2;

    template <typename Out> friend Out &&operator<<(Out &&out, bpf_log_entry const &what) {}
  };

  std::vector<bpf_log_entry> bpf_logs_;

  static thread_local BlobCollector blob_collector_;
};

} // namespace reducer::ingest
