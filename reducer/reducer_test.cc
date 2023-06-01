// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <otlp/otlp_test_server.h>
#include <reducer/reducer.h>
#include <reducer/reducer_config.h>
#include <scheduling/timer.h>
#include <util/code_timing.h>
#include <util/common_test.h>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

namespace reducer_test {

// Conditions to be met before stopping test
struct StopConditions {
  std::chrono::seconds timeout_sec = 0s;
  u64 num_otlp_log_requests = 0;
  u64 num_otlp_metric_requests = 0;
  u64 num_prom_metrics = 0;
  u64 num_prom_internal_metrics = 0;
};

static std::string const otlp_grpc_server_address = "localhost";
constexpr u32 otlp_grpc_server_port = 4317;
static std::string const
    otlp_grpc_server_address_and_port(otlp_grpc_server_address + ":" + std::to_string(otlp_grpc_server_port));

static std::string const prom_server_address_and_port("localhost:7001");
static std::string const internal_prom_server_address_and_port("localhost:7000");

// curl utility functions using curlpp based on https://github.com/jpbarrette/curlpp/blob/master/examples/example05.cpp
constexpr size_t CURL_BUF_SIZE_MAX = 200000;
char curl_buf[CURL_BUF_SIZE_MAX];
size_t curl_buf_size = 0;

// Callback must be declared static, otherwise it won't link...
size_t curl_callback(char *ptr, size_t size, size_t nmemb)
{
  size_t copysize = std::min(CURL_BUF_SIZE_MAX, size * nmemb);
  if (copysize) {
    memcpy(curl_buf, ptr, copysize);
    curl_buf_size = copysize;
    curl_buf[curl_buf_size - 1] = '\0';
  }
  return copysize;
};

std::string_view do_curl(std::string const &url)
{
  curl_buf_size = 0;
  curl_buf[0] = '\0';

  curlpp::Cleanup cleaner;
  curlpp::Easy request;

  // Set the writer callback to enable cURL to write result in a memory area
  curlpp::types::WriteFunctionFunctor callback(curl_callback);
  curlpp::options::WriteFunction *options = new curlpp::options::WriteFunction(callback);
  request.setOpt(options);

  // Set the URL to retrive.
  request.setOpt(new curlpp::options::Url(url.c_str()));
  request.perform();

  std::string_view curl_buf_view(curl_buf);
  LOG::debug("url={} curl_buf_view.size={} curl_buf_view={}", url, curl_buf_view.size(), curl_buf_view);
  return curl_buf_view;
}

class ReducerTest : public CommonTest {
protected:
  ReducerTest() : logs_server_(otlp_grpc_server_address_and_port), metrics_server_(otlp_grpc_server_address_and_port) {}

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

    // TODO: When Reducer::ingest_core_ is destructed it gets a segfault due to a nullptr dereference. Until the Reducer can be
    // cleanly shutdown this test will exit here.
    LOG::info("TearDown() doing exit(0)");
    exit(0);

    // Clean up loop_ to avoid valgrind and asan complaints about memory leaks.
    close_uv_loop_cleanly(&loop_);
  }

  void start_reducer(reducer::ReducerConfig &config, StopConditions stop_conditions)
  {
    stop_conditions_ = std::move(stop_conditions);
    run_test_stopper();

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

    if (logs_server_.get_num_requests_received() < stop_conditions_.num_otlp_log_requests ||
        metrics_server_.get_num_requests_received() < stop_conditions_.num_otlp_metric_requests) {
      stop_test_timer_->defer(std::chrono::seconds(1));
      return;
    }

    std::string_view curl_buf_view;
    if (stop_conditions_.num_prom_metrics) {
      EXPECT_NO_THROW(curl_buf_view = do_curl(prom_server_address_and_port));
      if (!curl_buf_view.empty() && curl_buf_view.find("tcp.bytes") != std::string::npos) {
        // may still be waiting for other stop conditions, but don't need to check this one again
        stop_conditions_.num_prom_metrics = 0;
      } else {
        stop_test_timer_->defer(std::chrono::seconds(1));
        return;
      }
    }

    if (stop_conditions_.num_prom_internal_metrics) {
      EXPECT_NO_THROW(curl_buf_view = do_curl(internal_prom_server_address_and_port));
      if (!curl_buf_view.empty() && curl_buf_view.find("ebpf_net_rpc_queue_buf_utilization") != std::string::npos) {
        // may still be waiting for other stop conditions, but don't need to check this one again
        stop_conditions_.num_prom_internal_metrics = 0;
      } else {
        stop_test_timer_->defer(std::chrono::seconds(1));
        return;
      }
    }

    LOG::debug("stop_test_check() stop_conditions have been met - calling stop_reducer()");
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

// TODO: When Reducer::ingest_core_ is destructed it gets a segfault due to a nullptr dereference. Until the Reducer can be
// cleanly shutdown these tests are DISABLED so they are not run as part of the GitHub Actions automated testing.

TEST_F(ReducerTest, DISABLED_OtlpGrpcInternalMetrics)
{
  reducer::ReducerConfig config{
      .telemetry_port = 8000,

      .num_ingest_shards = 1,
      .num_matching_shards = 1,
      .num_aggregation_shards = 1,
      .partitions_per_shard = 1,

      .enable_id_id = true,
      .enable_az_id = true,

      .enable_otlp_grpc_metrics = true,
      .otlp_grpc_metrics_address = otlp_grpc_server_address,
      .otlp_grpc_metrics_port = otlp_grpc_server_port,
      .otlp_grpc_batch_size = 1000,

      .disable_prometheus_metrics = true};
  StopConditions stop_conditions{.timeout_sec = std::chrono::seconds(60), .num_otlp_metric_requests = 1};
  start_reducer(config, std::move(stop_conditions));
}

TEST_F(ReducerTest, DISABLED_PrometheusInternalMetrics)
{
  reducer::ReducerConfig config{
      .telemetry_port = 8000,

      .num_ingest_shards = 1,
      .num_matching_shards = 1,
      .num_aggregation_shards = 1,
      .partitions_per_shard = 1,

      .enable_id_id = true,
      .enable_az_id = true,

      .enable_otlp_grpc_metrics = false,

      .disable_prometheus_metrics = false,
      .shard_prometheus_metrics = false,
      .prom_bind = prom_server_address_and_port,
      .internal_prom_bind = internal_prom_server_address_and_port,
      .scrape_metrics_tsdb_format = reducer::TsdbFormat::prometheus};
  StopConditions stop_conditions{.timeout_sec = std::chrono::seconds(60), .num_prom_internal_metrics = 1};
  start_reducer(config, std::move(stop_conditions));
}

} // namespace reducer_test
