/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ingest_core_stats_span.h"

#include "logging_core.h"

#include <reducer/internal_metrics_encoder.h>
#include <reducer/logging/component.h>

#include <reducer/internal_stats.h>
#include <reducer/tsdb_format.h>

#include <util/log.h>
#include <util/log_formatters.h>

namespace reducer::logging {

IngestCoreStatsSpan::IngestCoreStatsSpan() {}

IngestCoreStatsSpan::~IngestCoreStatsSpan() {}

void IngestCoreStatsSpan::client_handle_pool_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__client_handle_pool_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  ClientHandlePoolStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.span = msg->span_name;
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->agent_hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.metrics.client_handle_pool = msg->client_handle_pool;
  stats.metrics.client_handle_pool_fraction = msg->client_handle_pool_fraction;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::client_handle_pool_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} client_handle_pool={} client_handle_pool_fraction ={}  timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->agent_hostname,
      msg->os,
      msg->os_version,
      msg->client_handle_pool,
      msg->client_handle_pool_fraction,
      msg->time_ns);
}

void IngestCoreStatsSpan::agent_connection_message_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref,
    u64 timestamp,
    jsrv_logging__agent_connection_message_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  AgentConnectionMessageStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->agent_hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.labels.message = msg->message;
  stats.labels.severity = std::to_string(msg->severity_);
  stats.metrics.count = msg->count;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::agent_connection_message_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} client_handle_pool={} client_handle_pool_fraction={} message={} severity_={} count={}  timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->agent_hostname,
      msg->os,
      msg->os_version,
      msg->message,
      msg->severity_,
      msg->count,
      msg->time_ns);
}

void IngestCoreStatsSpan::agent_connection_message_error_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref,
    u64 timestamp,
    jsrv_logging__agent_connection_message_error_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  AgentConnectionMessageErrorStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->agent_hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.labels.message = msg->message;
  stats.labels.error = msg->error;
  stats.metrics.count = msg->count;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::agent_connection_message_error_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} client_handle_pool={} client_handle_pool_fraction={} message={} error={} count={}  timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->agent_hostname,
      msg->os,
      msg->os_version,
      msg->message,
      msg->error,
      msg->count,
      msg->time_ns);
}

void IngestCoreStatsSpan::connection_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__connection_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  ConnectionStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->agent_hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.metrics.time_since_last_message_ns = msg->time_since_last_message_ns;
  stats.metrics.clock_offset_ns = msg->clock_offset_ns;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::connection_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} client_handle_pool={} client_handle_pool_fraction={} time_since_last_message_ns={} clock_offset_ns={}  timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->agent_hostname,
      msg->os,
      msg->os_version,
      msg->time_since_last_message_ns,
      msg->clock_offset_ns,
      msg->time_ns);
}

void IngestCoreStatsSpan::collector_log_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__collector_log_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  CollectorLogStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->agent_hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.labels.severity = msg->severity_;
  stats.metrics.collector_log_count = msg->count;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::collector_log_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} severity_={} count={}  timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->agent_hostname,
      msg->os,
      msg->os_version,
      msg->severity_,
      msg->count,
      msg->time_ns);
}

void IngestCoreStatsSpan::entry_point_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__entry_point_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  EntrypointStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->agent_hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.labels.kernel_headers_source = msg->kernel_headers_source;
  stats.labels.error = msg->entrypoint_error;
  stats.metrics.entrypoint_info = 1u;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::entry_point_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} kernel_headers_source={} entrypoint_error={} entrypoint_info={}  timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->agent_hostname,
      msg->os,
      msg->os_version,
      msg->kernel_headers_source,
      msg->entrypoint_error,
      msg->entrypoint_info,
      msg->time_ns);
}

void IngestCoreStatsSpan::collector_health_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__collector_health_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  CollectorHealthStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.labels.status = msg->status;
  stats.labels.detail = msg->status_detail;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::collector_health_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} status={} status_detail={}  timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->hostname,
      msg->os,
      msg->os_version,
      msg->status,
      msg->status_detail,
      msg->time_ns);
}

void IngestCoreStatsSpan::bpf_log_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__bpf_log_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  BpfLogStats stats;
  stats.labels.module = msg->module;
  stats.labels.shard = std::to_string(msg->shard);
  stats.labels.version = msg->version;
  stats.labels.cloud = msg->cloud;
  stats.labels.env = msg->env;
  stats.labels.role = msg->role;
  stats.labels.az = msg->az;
  stats.labels.id = msg->node_id;
  stats.labels.kernel = msg->kernel_version;
  stats.labels.c_type = std::to_string(msg->client_type);
  stats.labels.c_host = msg->hostname;
  stats.labels.os = msg->os;
  stats.labels.os_version = msg->os_version;
  stats.labels.file = msg->filename;
  stats.labels.line = msg->line;
  stats.labels.code = msg->code;
  stats.labels.arg0 = msg->arg0;
  stats.labels.arg1 = msg->arg1;
  stats.labels.arg2 = msg->arg2;
  stats.metrics.bpf_log = 1u;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::bpf_log_stats: module={} shard={}  version={} cloud={} env={} role={} az={} node_id={} kernel_version={} client_type={} agent_hostname={} os={} os_version={} filename={} line={} code={} arg0={} arg1={} arg2={} timestamp={}",
      msg->module,
      msg->shard,
      msg->version,
      msg->cloud,
      msg->env,
      msg->role,
      msg->az,
      msg->node_id,
      msg->kernel_version,
      msg->client_type,
      msg->hostname,
      msg->os,
      msg->os_version,
      msg->filename,
      msg->line,
      msg->code,
      msg->arg0,
      msg->arg1,
      msg->arg2,
      msg->time_ns);
}

void IngestCoreStatsSpan::server_stats(
    ::ebpf_net::logging::weak_refs::ingest_core_stats span_ref, u64 timestamp, jsrv_logging__server_stats *msg)
{
  auto &encoder = local_core<LoggingCore>().encoder_;

  ServerStats stats;
  stats.labels.module = msg->module;
  stats.metrics.connections = msg->connection_counter;
  stats.metrics.disconnects = msg->disconnect_counter;

  encoder.write_internal_stats(stats, msg->time_ns);

  LOG::debug_in(
      reducer::logging::Component::internal_metrics,
      "IngestCoreStatsSpan::server_stats: module={} connection_counter={} disconnect_counter={}  timestamp={}",
      msg->module,
      msg->connection_counter,
      msg->disconnect_counter,
      msg->time_ns);
}

} // namespace reducer::logging
