/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/logging/span_base.h>

#include <absl/container/flat_hash_map.h>

#include <string>

namespace reducer::logging {

struct ConnectionMetrics;

class LoggerSpan : public ::ebpf_net::logging::LoggerSpanBase {
public:
  LoggerSpan();
  ~LoggerSpan();

  void connection_metrics(std::function<void(std::string_view, ConnectionMetrics const &)> const &f);

  void agent_lost_events(::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__agent_lost_events *msg);

  void pod_not_found(::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__pod_not_found *msg);

  void cgroup_not_found(::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__cgroup_not_found *msg);

  void rewriting_private_to_public_ip_mapping(
      ::ebpf_net::logging::weak_refs::logger span_ref,
      u64 timestamp,
      jsrv_logging__rewriting_private_to_public_ip_mapping *msg);

  void private_ip_in_private_to_public_ip_mapping(
      ::ebpf_net::logging::weak_refs::logger span_ref,
      u64 timestamp,
      jsrv_logging__private_ip_in_private_to_public_ip_mapping *msg);

  void failed_to_insert_dns_record(
      ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__failed_to_insert_dns_record *msg);

  void tcp_socket_failed_getting_process_reference(
      ::ebpf_net::logging::weak_refs::logger span_ref,
      u64 timestamp,
      jsrv_logging__tcp_socket_failed_getting_process_reference *msg);

  void udp_socket_failed_getting_process_reference(
      ::ebpf_net::logging::weak_refs::logger span_ref,
      u64 timestamp,
      jsrv_logging__udp_socket_failed_getting_process_reference *msg);

  void socket_address_already_assigned(
      ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__socket_address_already_assigned *msg);

  void ingest_decompression_error(
      ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__ingest_decompression_error *msg);

  void ingest_processing_error(
      ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__ingest_processing_error *msg);

  void ingest_connection_error(
      ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__ingest_connection_error *msg);

  void k8s_container_pod_not_found(
      ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__k8s_container_pod_not_found *msg);

  void agent_connect_success(
      ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__agent_connect_success *msg);
};
}; // namespace reducer::logging
