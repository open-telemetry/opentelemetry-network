/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/publisher.h>
#include <reducer/tsdb_format.h>

#include <reducer/util/index_dumper.h>

#include <platform/types.h>
#include <scheduling/interval_scheduler.h>

#include <absl/synchronization/notification.h>

#include <uv.h>

namespace reducer {
class RpcQueueMatrix;
}

namespace reducer::ingest {

class TcpServer;

class IngestCore {
public:
  // Arguments:
  //   - ingest_to_logging_queues - Queues for sending messages to the logging
  //       core
  //   - ingest_to_matching_queues - Queues for sending messages to the matching
  //       core
  //   - telemetry_port - The port to which agents will send data.
  //   - stats_writer - Used to write internal metrics to prometheus
  //   - shard_config - Per-shard configuration - will spawn as many shards as
  //       the size of this parameter
  //   - metrics_tsdb_format - Format of metrics published to TSDB
  //   - localhost - Whether or not the ingest TCP listens to
  //         0.0.0.0 (if `localhost` is false) or 127.0.0.1
  IngestCore(
      RpcQueueMatrix &ingest_to_logging_queues,
      RpcQueueMatrix &ingest_to_matching_queues,
      u32 telemetry_port,
      bool localhost = false);

  ~IngestCore();

  /**
   * runs the loop
   */
  void run();

  /**
   * stops the loop - can be run from any thread
   */
  void stop_async();
  void stop_sync();
  void wait_for_shutdown();

private:
  /* Callback function for stop_async. */
  static void on_stop_async(uv_async_t *handle);

  /* Core callbacks */
  static void on_write_internal_stats_timer_cb(uv_timer_t *timer);
  static void on_pulse_timer_cb(uv_timer_t *timer);

  /**
   * (internal) called when it's time to drain internal metrics for
   *   connections and disconnects to prom_queue_
   */
  void on_write_internal_stats_timer();

  /**
   * Called every-now-and-then to send a liveness pulse to peer cores.
   */
  void on_pulse_timer();

  /**
   * Scans for timed-out connections and disconnects them.
   */
  void check_connection_timeouts();

  // Libuv loop object.
  uv_loop_t loop_;

  // Async object used for stopping the loop from another thread.
  uv_async_t stop_async_;

  // Notification triggered when the loop is done running.
  absl::Notification done_;

  std::unique_ptr<TcpServer> tcp_server_;
  std::vector<IndexDumper> index_dumper_;

  uv_timer_t write_internal_stats_timer_;
  uv_timer_t pulse_timer_;
  std::optional<scheduling::IntervalScheduler> connection_timeout_handler_;

  friend void __on_signal_cb(uv_signal_t *, int);
};

} // namespace reducer::ingest
