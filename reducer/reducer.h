/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/aggregation/agg_core.h>
#include <reducer/ingest/ingest_core.h>
#include <reducer/logging/logging_core.h>
#include <reducer/matching/matching_core.h>
#include <reducer/publisher.h>
#include <reducer/reducer_config.h>
#include <reducer/rpc_queue_matrix.h>

#include <thread>

namespace reducer {

class Reducer {
public:
  Reducer(uv_loop_t &loop, ReducerConfig &config);

  void startup();
  void shutdown();

private:
  void init_config();
  void init_cores();
  void start_threads();

  uv_loop_t &loop_;
  ReducerConfig &config_;

  std::unique_ptr<reducer::Publisher> stats_publisher_;

  reducer::RpcQueueMatrix ingest_to_matching_queues_;
  reducer::RpcQueueMatrix ingest_to_logging_queues_;
  reducer::RpcQueueMatrix matching_to_logging_queues_;
  reducer::RpcQueueMatrix matching_to_aggregation_queues_;
  reducer::RpcQueueMatrix aggregation_to_logging_queues_;

  std::unique_ptr<reducer::logging::LoggingCore> logging_core_;
  std::vector<std::unique_ptr<reducer::aggregation::AggCore>> agg_cores_;
  std::vector<std::unique_ptr<reducer::matching::MatchingCore>> matching_cores_;
  std::unique_ptr<reducer::ingest::IngestCore> ingest_core_;

  std::vector<std::thread> threads_;
};

} // namespace reducer
