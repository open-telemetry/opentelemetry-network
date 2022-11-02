// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "tcp_server.h"

#include <reducer/ingest/ingest_worker.h>

#include <util/uv_helpers.h>

#include <iomanip>
#include <memory>
#include <signal.h>
#include <sstream>

#define SERVER_LISTEN_BACKLOG 128

using std::placeholders::_1;
using std::placeholders::_2;

namespace reducer::ingest {

void TcpServer::on_new_connection_cb(uv_stream_t *stream, int status)
{
  uv_tcp_t *server_socket = (uv_tcp_t *)stream;
  TcpServer *server = (TcpServer *)server_socket->data;

  if (status != 0) {
    LOG::error("Error creating new connection: {}", uv_strerror(status));
    return;
  }
  server->on_new_connection();
}

TcpServer::TcpServer(uv_loop_t &loop, u32 telemetry_port, bool localhost, std::vector<std::unique_ptr<IngestWorker>> workers)
    : loop_(loop), workers_(std::move(workers))
{
  // Initialize the workers.
  std::vector<Worker *> worker_ptrs;
  for (std::size_t i = 0; i < workers_.size(); ++i) {
    IngestWorker *const worker_ptr = workers_[i].get();
    worker_ptrs.push_back(worker_ptr);

    worker_ptr->register_close_callback([this, worker_ptr] { on_connection_close(worker_ptr); });
    worker_ptr->start(i);
  }
  worker_balancer_ = std::make_unique<LoadBalancer<Worker *>>(absl::MakeSpan(worker_ptrs));

  /* Listen for telemetry connections */
  CHECK_UV(uv_tcp_init(&loop_, &server_));
  /* save this for callbacks from libuv */
  server_.data = this;

  /* bind + listen */
  struct sockaddr_in addr;
  CHECK_UV(uv_ip4_addr(localhost ? "127.0.0.1" : "0.0.0.0", telemetry_port, &addr));
  CHECK_UV(uv_tcp_bind(&server_, (struct sockaddr *)&addr, 0));
  CHECK_UV(uv_listen((uv_stream_t *)&server_, SERVER_LISTEN_BACKLOG, on_new_connection_cb));
}

TcpServer::~TcpServer()
{
  for (auto &worker : workers_) {
    worker->stop();
  }
}

TcpServer::Stats TcpServer::get_stats()
{
  absl::ReaderMutexLock l(&stats_mu_);
  return stats_;
}

void TcpServer::visit_indexes(const IndexCb &cb, const bool block)
{
  visit_internal(
      [&cb](const int worker_index, IngestWorker *const worker) {
        IngestWorker::IndexCb worker_cb = std::bind(cb, worker_index, _1);
        return worker->visit_index(std::move(worker_cb));
      },
      block);
}

void TcpServer::visit_connections(const ConnectionCb &cb, const bool block)
{
  visit_internal(
      [&cb](const int worker_index, IngestWorker *const worker) {
        IngestWorker::ConnectionCb worker_cb = std::bind(cb, worker_index, _1);
        return worker->visit_connections(std::move(worker_cb));
      },
      block);
}

void TcpServer::visit_channels(const ChannelCb &cb, const bool block)
{
  visit_internal(
      [&cb](const int worker_index, IngestWorker *const worker) {
        IngestWorker::ChannelCb worker_cb = std::bind(cb, worker_index, _1, _2);
        return worker->visit_channels(std::move(worker_cb));
      },
      block);
}

void TcpServer::visit_rpc_stats(const RpcStatsCb &cb, const bool block)
{
  visit_internal(
      [&cb](const int worker_index, IngestWorker *const worker) {
        IngestWorker::RpcStatsCb worker_cb = std::bind(cb, worker_index, _1);
        return worker->visit_rpc_stats(std::move(worker_cb));
      },
      block);
}

TcpServer::Singleton *TcpServer::singleton()
{
  static auto *const value = new Singleton;
  return value;
}

void TcpServer::on_new_connection()
{
  // Accept the new connection.
  auto *const conn = reinterpret_cast<uv_tcp_t *>(std::malloc(sizeof(uv_tcp_t)));
  CHECK_UV(uv_tcp_init(&loop_, conn));
  CHECK_UV(uv_accept(reinterpret_cast<uv_stream_t *>(&server_), reinterpret_cast<uv_stream_t *>(conn)));

  // Hand off connection to worker.
  Worker *const worker = worker_balancer_->least_loaded();
  worker_balancer_->increment_load(worker, 1);
  worker->assign(*conn);

  // Close the connnection.
  uv_close(reinterpret_cast<uv_handle_t *>(conn), reinterpret_cast<uv_close_cb>(&std::free));

  // Update the stats
  {
    absl::MutexLock l(&stats_mu_);
    stats_.connection_counter++;
  }
}

void TcpServer::on_connection_close(IngestWorker *const worker)
{
  // Update the load balancer.
  worker_balancer_->increment_load(worker, -1);

  // Update the connection stats.
  {
    absl::MutexLock l(&stats_mu_);
    stats_.disconnect_counter++;
  }
}

void TcpServer::visit_internal(const WorkerVisitCb &cb, const bool block)
{
  // Run all the callbacks in parallel.
  std::vector<std::shared_ptr<absl::Notification>> notifications;
  for (std::size_t worker_index = 0; worker_index < workers_.size(); worker_index++) {
    notifications.push_back(cb(worker_index, workers_[worker_index].get()));
  }

  // Wait for them to finish.
  if (block) {
    for (const auto &notification : notifications) {
      notification->WaitForNotification();
    }
  }
}

} // namespace reducer::ingest
