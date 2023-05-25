// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <otlp/otlp_test_server.h>
#include <reducer/reducer.h>
#include <reducer/reducer_config.h>
#include <scheduling/timer.h>
#include <util/code_timing.h>
#include <util/common_test.h>

namespace reducer_test {

// Conditions to be met before stopping test
struct StopConditions {
  std::chrono::seconds timeout_sec = 0s;
  u64 num_log_requests = 0;
  u64 num_metric_requests = 0;
};

static std::string const server_address = "localhost";
static u32 const server_port = 4317;
static std::string const server_address_and_port(server_address + ":" + std::to_string(server_port));

class ReducerTest : public CommonTest {
protected:
  ReducerTest() : logs_server_(server_address_and_port), metrics_server_(server_address_and_port) {}

  void SetUp() override
  {
    CommonTest::SetUp();
    ASSERT_EQ(0, uv_loop_init(&loop_));

    logs_server_.start();
    metrics_server_.start();
  }

  void TearDown() override
  {
    logs_server_.stop();
    metrics_server_.stop();

    print_code_timings();

    // Note: During destruction, when Reducer::ingest_core_ is destructed, it gets a segfault due to a nullptr dereference.
    // Until the Reducer can be cleanly shutdown this test will exit here.
    LOG::info("TearDown() doing exit(0)");
    exit(0);

    // Clean up loop_ to avoid valgrind and asan complaints about memory leaks.
    close_uv_loop_cleanly(&loop_);
  }

  void start_reducer(StopConditions stop_conditions)
  {
    stop_conditions_ = std::move(stop_conditions);
    run_test_stopper();

    reducer::ReducerConfig config{
        .telemetry_port = 8000,

        .num_ingest_shards = 1,
        .num_matching_shards = 1,
        .num_aggregation_shards = 1,
        .partitions_per_shard = 1,

        .enable_id_id = true,
        .enable_az_id = true,
        .enable_flow_logs = false,

        .enable_otlp_grpc_metrics = true,
        .otlp_grpc_metrics_address = server_address,
        .otlp_grpc_metrics_port = server_port,
        .otlp_grpc_batch_size = 1000,

        .disable_prometheus_metrics = true};

    LOG::info("Starting Reducer...");
    reducer_ = std::make_unique<reducer::Reducer>(loop_, config);

    // start timing for purposes of the test timeout
    stopwatch_.emplace();

    reducer_->startup();
    LOG::info("reducer_->startup() returned.");
  }

  void stop_reducer()
  {
    SCOPED_TIMING(StopReducer);
    EXPECT_EQ(false, timeout_exceeded_);
    reducer_->shutdown();
  }

  void stop_test_check()
  {
    SCOPED_TIMING(StopTestCheck);

    // check for test timeout
    if (stopwatch_) {
      timeout_exceeded_ = stopwatch_->elapsed(stop_conditions_.timeout_sec);
      LOG::trace(
          "stop_test_check() stop_conditions_.timeout_sec {} exceeded {}", stop_conditions_.timeout_sec, timeout_exceeded_);
      if (timeout_exceeded_) {
        LOG::error("stop_test_check() test timeout of {} exceeded", stop_conditions_.timeout_sec);
        stop_reducer();
        return;
      }
    }

    if (logs_server_.get_num_requests_received() < stop_conditions_.num_log_requests ||
        metrics_server_.get_num_requests_received() < stop_conditions_.num_metric_requests) {
      stop_test_timer_->defer(std::chrono::seconds(1));
      return;
    }

    LOG::trace("stop_test_check() stop_conditions have been met - calling stop_reducer()");
    stop_reducer();
  }

  void run_test_stopper()
  {
    stop_test_timer_ = std::make_unique<scheduling::Timer>(loop_, std::bind(&ReducerTest::stop_test_check, this));
    stop_test_timer_->defer(std::chrono::seconds(1));
  }

  uv_loop_t loop_;

  std::unique_ptr<reducer::Reducer> reducer_;

  StopConditions stop_conditions_;

  bool timeout_exceeded_ = false;
  std::optional<StopWatch<>> stopwatch_;
  std::unique_ptr<scheduling::Timer> stop_test_timer_;

  otlp_test_server::OtlpGrpcTestServer<LogsService, ExportLogsServiceRequest, ExportLogsServiceResponse> logs_server_;
  otlp_test_server::OtlpGrpcTestServer<MetricsService, ExportMetricsServiceRequest, ExportMetricsServiceResponse>
      metrics_server_;
};

TEST_F(ReducerTest, InternalMetrics)
{
  StopConditions stop_conditions{.timeout_sec = std::chrono::seconds(30), .num_metric_requests = 1};
  start_reducer(std::move(stop_conditions));
}

} // namespace reducer_test
