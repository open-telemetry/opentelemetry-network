// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "logger_span.h"
#include "connection_metrics.h"

#include <common/client_type.h>
#include <reducer/constants.h>

#include <util/log.h>

namespace reducer::logging {

namespace {

// Ingest errors should be written to the log only for valid clients -- those that have at least successfully sent the
// version information. Otherwise the log can be spammed by spurious connections. Errors triggered by non-valid clients will be
// sent to the debug log, for debugging purposes.
//
template <typename Format, typename... Args> void log_ingest_error(ClientType client_type, Format &&format, Args &&...args)
{
  if (client_type == ClientType::unknown) {
    LOG::debug(std::forward<Format>(format), std::forward<Args>(args)...);
  } else {
    LOG::error(std::forward<Format>(format), std::forward<Args>(args)...);
  }
}

} // namespace

LoggerSpan::LoggerSpan() {}

LoggerSpan::~LoggerSpan() {}

void LoggerSpan::agent_lost_events(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__agent_lost_events *msg)
{
  LOG::warn("({}) lost events ({}) from agent at '{}'", msg->_rpc_id, msg->count, msg->client_hostname);
}

void LoggerSpan::pod_not_found(::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__pod_not_found *msg)
{
  LOG::error("({}) pod with uid={} not found", msg->_rpc_id, msg->uid);
}

void LoggerSpan::cgroup_not_found(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__cgroup_not_found *msg)
{
  LOG::error("({}) cgroup with id={} not found", msg->_rpc_id, msg->cgroup);
}

void LoggerSpan::rewriting_private_to_public_ip_mapping(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__rewriting_private_to_public_ip_mapping *msg)
{
  LOG::warn(
      "({}) rewriting existing private-to-public IP address mapping:"
      " private={}, existing_public={}, new_public={}",
      msg->_rpc_id,
      msg->private_addr,
      msg->existing_public_addr,
      msg->new_public_addr);
}

void LoggerSpan::private_ip_in_private_to_public_ip_mapping(
    ::ebpf_net::logging::weak_refs::logger span_ref,
    u64 timestamp,
    jsrv_logging__private_ip_in_private_to_public_ip_mapping *msg)
{
  LOG::warn(
      "({}) private-only address exists in private-to-public IP address"
      " mapping: private={}, existing_public={}",
      msg->_rpc_id,
      msg->private_addr,
      msg->existing_public_addr);
}

void LoggerSpan::failed_to_insert_dns_record(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__failed_to_insert_dns_record *msg)
{
  LOG::error("({}) failed to insert dns record", msg->_rpc_id);
}

void LoggerSpan::tcp_socket_failed_getting_process_reference(
    ::ebpf_net::logging::weak_refs::logger span_ref,
    u64 timestamp,
    jsrv_logging__tcp_socket_failed_getting_process_reference *msg)
{
  LOG::error(
      "({}) TCP socket span failed to get process span reference"
      " for pid={}",
      msg->_rpc_id,
      msg->pid);
}

void LoggerSpan::udp_socket_failed_getting_process_reference(
    ::ebpf_net::logging::weak_refs::logger span_ref,
    u64 timestamp,
    jsrv_logging__udp_socket_failed_getting_process_reference *msg)
{
  LOG::error(
      "({}) UDP socket span failed to get process span reference"
      " for pid={}",
      msg->_rpc_id,
      msg->pid);
}

void LoggerSpan::socket_address_already_assigned(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__socket_address_already_assigned *msg)
{
  LOG::error("({}) attempt to assign socket address multiple times", msg->_rpc_id);
}

void LoggerSpan::ingest_decompression_error(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__ingest_decompression_error *msg)
{
  auto client_type = static_cast<ClientType>(msg->client_type);

  log_ingest_error(
      client_type,
      "({}) ingest decompression error from {} at '{}': {}",
      msg->_rpc_id,
      msg->client_type,
      msg->client_hostname,
      msg->error);
}

void LoggerSpan::ingest_processing_error(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__ingest_processing_error *msg)
{
  auto client_type = static_cast<ClientType>(msg->client_type);

  log_ingest_error(
      client_type,
      "({}) error processing data from {} at '{}': {}",
      msg->_rpc_id,
      client_type,
      msg->client_hostname,
      msg->error);
}

void LoggerSpan::ingest_connection_error(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__ingest_connection_error *msg)
{
  auto client_type = static_cast<ClientType>(msg->client_type);

  log_ingest_error(
      client_type,
      "({}) connection error from {} collector at '{}' encountered: {}",
      msg->_rpc_id,
      msg->client_type,
      msg->client_hostname,
      msg->error);
}

void LoggerSpan::k8s_container_pod_not_found(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__k8s_container_pod_not_found *msg)
{
  LOG::error("({}) k8s_container failed to reference a pod", msg->_rpc_id);
}

void LoggerSpan::agent_connect_success(
    ::ebpf_net::logging::weak_refs::logger span_ref, u64 timestamp, jsrv_logging__agent_connect_success *msg)
{
  std::string_view client_hostname{msg->client_hostname.buf, msg->client_hostname.len};

  LOG::info(
      "({}) {} collector at '{}' successfully connected",
      msg->_rpc_id,
      static_cast<ClientType>(msg->client_type),
      client_hostname);
}

}; // namespace reducer::logging
