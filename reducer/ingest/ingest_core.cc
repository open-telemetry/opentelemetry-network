// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "ingest_core.h"

#include <reducer/ingest/shared_state.h>
#include <reducer/ingest/tcp_server.h>

#include <generated/ebpf_net/ingest/span_base.h>

#include <reducer/constants.h>
#include <reducer/internal_metrics_encoder.h>
#include <reducer/internal_stats.h>
#include <reducer/rpc_queue_matrix.h>
#include <reducer/tsdb_formatter.h>
#include <reducer/util/thread_ops.h>

#include <platform/userspace-time.h>
#include <util/error_handling.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/time.h>
#include <util/uv_helpers.h>

#include <iomanip>
#include <sstream>

#include <config.h>

namespace reducer::ingest {

namespace {
using namespace std::literals::chrono_literals;
constexpr auto NO_MESSAGE_TIMEOUT = 30s;
constexpr auto MESSAGE_TIMEOUT_CHECK_INTERVAL = 45s;
constexpr auto WRITE_INTERNAL_STATS_TIMER_REPEAT = 10s;
constexpr auto PULSE_TIMER_REPEAT = 1s;
} // namespace

void IngestCore::on_write_internal_stats_timer_cb(uv_timer_t *timer)
{
  auto const core = reinterpret_cast<IngestCore *>(timer->data);
  core->on_write_internal_stats_timer();
}

void IngestCore::on_pulse_timer_cb(uv_timer_t *timer)
{
  IngestCore *core = (IngestCore *)timer->data;
  core->on_pulse_timer();
}

void IngestCore::on_stop_async(uv_async_t *handle)
{
  auto const core = reinterpret_cast<IngestCore *>(handle->data);
  uv_stop(&core->loop_);
}

IngestCore::IngestCore(
    RpcQueueMatrix &ingest_to_logging_queues, RpcQueueMatrix &ingest_to_matching_queues, u32 telemetry_port, bool localhost)
{
  auto const ingest_shard_count = ingest_to_matching_queues.num_senders();
  int res;

  res = uv_loop_init(&loop_);
  if (res != 0)
    throw std::runtime_error("uv_loop_init failed");

  res = uv_async_init(&loop_, &stop_async_, &on_stop_async);
  if (res != 0) {
    throw std::runtime_error("uv_async_init failed");
  }
  stop_async_.data = this;

  ASSUME(ingest_to_logging_queues.num_receivers() == 1).else_log("Not using multiple logging cores");

  // Create the ingest workers and start the telemetry TCP server.
  ASSUME(ingest_shard_count > 0).else_log("Ingest shards should be > 0, instead got {}", ingest_shard_count);

  std::vector<std::unique_ptr<IngestWorker>> workers;
  workers.reserve(ingest_shard_count);
  for (uint32_t shard = 0; shard < ingest_shard_count; ++shard) {
    workers.push_back(std::make_unique<IngestWorker>(ingest_to_logging_queues, ingest_to_matching_queues, shard));
  }
  tcp_server_.reset(new TcpServer(loop_, telemetry_port, localhost, std::move(workers)));
  index_dumper_.resize(ingest_shard_count);
  TcpServer::singleton()->instance = tcp_server_.get();

  /* internal stats */
  /* initialize the internal stats timer */
  res = uv_timer_init(&loop_, &write_internal_stats_timer_);
  if (res != 0)
    throw std::runtime_error("could not init write internal stats timer");

  /* save 'this' for callback */
  write_internal_stats_timer_.data = this;

  /* start the internal stats timer */
  res = uv_timer_start(
      &write_internal_stats_timer_,
      on_write_internal_stats_timer_cb,
      integer_time<std::chrono::milliseconds>(WRITE_INTERNAL_STATS_TIMER_REPEAT),
      integer_time<std::chrono::milliseconds>(WRITE_INTERNAL_STATS_TIMER_REPEAT));
  if (res != 0) {
    throw std::runtime_error("could not start write internal stats timer");
  }

  CHECK_UV(uv_timer_init(&loop_, &pulse_timer_));
  pulse_timer_.data = this;
  CHECK_UV(uv_timer_start(
      &pulse_timer_,
      on_pulse_timer_cb,
      integer_time<std::chrono::milliseconds>(PULSE_TIMER_REPEAT),
      integer_time<std::chrono::milliseconds>(PULSE_TIMER_REPEAT)));

  connection_timeout_handler_.emplace(loop_, [this] {
    check_connection_timeouts();
    return scheduling::JobFollowUp::ok;
  });
  connection_timeout_handler_->start(MESSAGE_TIMEOUT_CHECK_INTERVAL);
}

IngestCore::~IngestCore()
{
  TcpServer::singleton()->instance = nullptr;
}

void IngestCore::run()
{
  set_self_thread_name("ingest_listen").on_error([](auto const &error) {
    LOG::warn("unable to set name for ingest core listening thread: {}", error);
  });
  uv_run(&loop_, UV_RUN_DEFAULT);
  close_uv_loop_cleanly(&loop_);
  done_.Notify();
}

void IngestCore::stop_async()
{
  uv_async_send(&stop_async_);
}

void IngestCore::stop_sync()
{
  stop_async();
  wait_for_shutdown();
}

void IngestCore::wait_for_shutdown()
{
  done_.WaitForNotification();
}

std::string to_string(VersionInfo v)
{
  std::stringstream ss;
  ss << v;
  return ss.str();
}

void IngestCore::on_write_internal_stats_timer()
{
  u64 time_ns = fp_get_time_ns();
  std::string_view module = "ingest";
  TcpServer::Stats server_stats = tcp_server_->get_stats();

  /* write span statistics */
  tcp_server_->visit_indexes(
      [&](const int shard, ::ebpf_net::ingest::Index *const index) {
        index->size_statistics(
            [&](std::string_view span_name, std::size_t allocated, std::size_t max_allocated, std::size_t pool_size) {
              local_core_stats_handle().span_utilization_stats(
                  jb_blob(std::string(span_name)), jb_blob(module), shard, allocated, max_allocated, pool_size, time_ns);
            });

        local_core_stats_handle().status_stats(
            jb_blob(module), shard, jb_blob(std::string(kServiceName)), jb_blob(to_string(versions::release)), 1u, time_ns);

        /* This internal stat registers total TCP server connects and disconnects. This needs to execute only once
           and not in all shards/workers. */

        if (shard == 0) {
          local_ingest_core_stats_handle().server_stats(
              jb_blob(module), server_stats.connection_counter, server_stats.disconnect_counter, time_ns);
        }

        index_dumper_[shard].dump(
            "ingest", shard, *index, std::chrono::duration_cast<std::chrono::seconds>(std::chrono::nanoseconds{time_ns}));
      },
      true /* block */);

  tcp_server_->visit_connections(
      [&](const int shard, NpmConnection *const ftconn) {
        auto *conn = ftconn->ingest_connection();
        AgentSpan &agent = conn->agent().impl();

        // skip internal stats for AgentSpans that are not initialized (unknown) and for short-lived probe agents
        if (agent.client_type() == ClientType::unknown || agent.client_type() == ClientType::liveness_probe ||
            agent.client_type() == ClientType::readiness_probe) {
          return;
        }

        /* write client span utilization statistics */

        auto const client_handle_utilization = [&](std::string_view span_name, u64 size, u64 capacity) {
          local_ingest_core_stats_handle().client_handle_pool_stats(
              jb_blob(module),
              shard,
              jb_blob(span_name),
              jb_blob(to_string(agent.version())),
              jb_blob(std::to_string(integer_value(agent.cloud_platform()))),
              jb_blob(agent.cluster()),
              jb_blob(agent.role()),
              jb_blob(agent.node_az()),
              jb_blob(agent.node_az()),
              jb_blob(agent.kernel_version()),
              integer_value(agent.client_type()),
              jb_blob(agent.hostname()),
              jb_blob(agent.os()),
              jb_blob(agent.os_version()),
              time_ns,
              size,
              (double)(size / capacity));
        };

        client_handle_utilization("socket", conn->socket__hash.size(), conn->socket__hash.capacity());
        client_handle_utilization("udp_socket", conn->udp_socket__hash.size(), conn->udp_socket__hash.capacity());
        client_handle_utilization("process", conn->process__hash.size(), conn->process__hash.capacity());
        client_handle_utilization(
            "tracked_process", conn->tracked_process__hash.size(), conn->tracked_process__hash.capacity());
        client_handle_utilization("cgroup", conn->cgroup__hash.size(), conn->cgroup__hash.capacity());
        client_handle_utilization(
            "aws_network_interface", conn->aws_network_interface__hash.size(), conn->aws_network_interface__hash.capacity());

        /* write message statistics */
        conn->message_stats.foreach ([&](std::string_view module, std::string_view msg, int severity, u64 count) {
          local_ingest_core_stats_handle().agent_connection_message_stats(
              jb_blob(module),
              shard,
              jb_blob(to_string(agent.version())),
              jb_blob(std::to_string(integer_value(agent.cloud_platform()))),
              jb_blob(agent.cluster()),
              jb_blob(agent.role()),
              jb_blob(agent.node_az()),
              jb_blob(agent.node_az()),
              jb_blob(agent.kernel_version()),
              integer_value(agent.client_type()),
              jb_blob(agent.hostname()),
              jb_blob(agent.os()),
              jb_blob(agent.os_version()),
              time_ns,
              jb_blob(msg),
              severity,
              count);
        });

        conn->message_errors.foreach ([&](std::string_view module, std::string_view msg, std::string_view error, u64 count) {
          local_ingest_core_stats_handle().agent_connection_message_error_stats(
              jb_blob(module),
              shard,
              jb_blob(to_string(agent.version())),
              jb_blob(std::to_string(integer_value(agent.cloud_platform()))),
              jb_blob(agent.cluster()),
              jb_blob(agent.role()),
              jb_blob(agent.node_az()),
              jb_blob(agent.node_az()),
              jb_blob(agent.kernel_version()),
              integer_value(agent.client_type()),
              jb_blob(agent.hostname()),
              jb_blob(agent.os()),
              jb_blob(agent.os_version()),
              time_ns,
              jb_blob(msg),
              jb_blob(error),
              uint64_t(count));
        });

        /* write connection statistics */
        local_ingest_core_stats_handle().connection_stats(
            jb_blob(module),
            shard,
            jb_blob(to_string(agent.version())),
            jb_blob(std::to_string(integer_value(agent.cloud_platform()))),
            jb_blob(agent.cluster()),
            jb_blob(agent.role()),
            jb_blob(agent.node_az()),
            jb_blob(agent.node_az()),
            jb_blob(agent.kernel_version()),
            integer_value(agent.client_type()),
            jb_blob(agent.hostname()),
            jb_blob(agent.os()),
            jb_blob(agent.os_version()),
            time_ns,
            static_cast<u64>(ftconn->time_since_last_message().count()),
            static_cast<u64>(ftconn->clock_offset().count()));

        /* write collector log statistics */
        auto const write_log_count = [&](std::string_view severity, u32 count) {
          local_ingest_core_stats_handle().collector_log_stats(
              jb_blob(module),
              shard,
              jb_blob(to_string(agent.version())),
              jb_blob(std::to_string(integer_value(agent.cloud_platform()))),
              jb_blob(agent.cluster()),
              jb_blob(agent.role()),
              jb_blob(agent.node_az()),
              jb_blob(agent.node_az()),
              jb_blob(agent.kernel_version()),
              integer_value(agent.client_type()),
              jb_blob(agent.hostname()),
              jb_blob(agent.os()),
              jb_blob(agent.os_version()),
              time_ns,
              jb_blob(severity),
              count);
        };

        auto const &log_count = agent.log_count();
        write_log_count("ignored", log_count.ignored);
        write_log_count("info", log_count.info);
        write_log_count("warning", log_count.warning);
        write_log_count("error", log_count.error);
        write_log_count("critical", log_count.critical);

        /* write agent stats */
        auto ingest_core_stats = local_ingest_core_stats_handle();
        agent.write_internal_stats(ingest_core_stats, time_ns, shard, "ingest");

        /* write entrypoint information */
        local_ingest_core_stats_handle().entry_point_stats(
            jb_blob(module),
            shard,
            jb_blob(to_string(agent.version())),
            jb_blob(std::to_string(integer_value(agent.cloud_platform()))),
            jb_blob(agent.cluster()),
            jb_blob(agent.role()),
            jb_blob(agent.node_az()),
            jb_blob(agent.node_az()),
            jb_blob(agent.kernel_version()),
            integer_value(agent.client_type()),
            jb_blob(agent.hostname()),
            jb_blob(agent.os()),
            jb_blob(agent.os_version()),
            time_ns,
            jb_blob(agent.kernel_version()),
            jb_blob(agent.kernel_headers_source()),
            1u);
      },
      true /* block */);

  /* RPC queue statistics */
  tcp_server_->visit_rpc_stats(
      [&](int shard, RpcSenderStats &rpc_stats) {
        auto core_stats_handle = local_core_stats_handle();
        rpc_stats.write_internal_metrics_to_logging_core(core_stats_handle, time_ns);
      },
      true);
}

void IngestCore::on_pulse_timer()
{
  tcp_server_->visit_indexes(
      [&](const int shard, ::ebpf_net::ingest::Index *const index) { index->send_pulse(); }, false /* block */);
}

void IngestCore::check_connection_timeouts()
{
  auto now = std::chrono::nanoseconds(fp_get_time_ns());

  tcp_server_->visit_channels(
      [&](const int shard, channel::TCPChannel *channel, std::chrono::nanoseconds last_message_seen) {
        auto time_since_last_message = now - last_message_seen;
        if (time_since_last_message >= NO_MESSAGE_TIMEOUT) {
          LOG::warn("closing connection due to inactivity");
          channel->close_permanently();
        }
      },
      true /* block */
  );
}

} // namespace reducer::ingest
