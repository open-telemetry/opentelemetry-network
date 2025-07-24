// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "otlp_grpc_client.h"
#include "otlp_request_builder.h"
#include "otlp_test_server.h"
#include "otlp_util.h"

#include <util/common_test.h>
#include <util/json.h>
#include <util/time.h>

#include <vector>

namespace otlp_client {

std::string grpc_test_server_addr("localhost:54321");

class OtlpGrpcClientTest : public CommonTest {
protected:
  OtlpGrpcClientTest()
      : logs_client_(grpc::CreateChannel(grpc_test_server_addr, grpc::InsecureChannelCredentials())),
        metrics_client_(grpc::CreateChannel(grpc_test_server_addr, grpc::InsecureChannelCredentials())),
        server_(grpc_test_server_addr)
  {}

  void SetUp() override
  {
    CommonTest::SetUp();
    server_.start();
  }

  void TearDown() override { server_.stop(); }

  ExportLogsServiceRequest create_logs_request()
  {
    ExportLogsServiceRequest request;
    auto resource_logs = request.add_resource_logs();
    auto scope_logs = resource_logs->add_scope_logs();
    opentelemetry::proto::logs::v1::LogRecord log_record;
    log_record.set_time_unix_nano(integer_time<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()));
    log_record.set_severity_text("INFO");
    log_record.set_severity_number(opentelemetry::proto::logs::v1::SeverityNumber::SEVERITY_NUMBER_INFO);

    std::string_view message("test message 127.0.0.1 192.168.1.1 98765");
    log_record.mutable_body()->set_string_value(message.data(), message.size());
    *scope_logs->add_log_records() = std::move(log_record);

    return request;
  }

  ExportMetricsServiceRequest create_metrics_request()
  {
    return otlp_client::OtlpRequestBuilder()
        .metric("test-metric-name")
        .sum()
        .number_data_point(
            456u,
            {{"label1", "value1"}, {"label2", "value2"}},
            std::chrono::nanoseconds(std::chrono::system_clock::now().time_since_epoch()));
  }

  template <typename TService, typename TReq, typename TResp>
  void send_async(OtlpGrpcClient<TService, TReq, TResp> &client_, TReq const &request)
  {
    auto prev_bytes_sent = client_.bytes_sent();
    auto prev_data_points_sent = client_.data_points_sent();
    auto prev_requests_sent = client_.requests_sent();

    client_.AsyncExport(request);
    if constexpr (std::is_same_v<ExportLogsServiceRequest, TReq>) {
      log_requests_sent_.push_back(request);
    } else if constexpr (std::is_same_v<ExportMetricsServiceRequest, TReq>) {
      metric_requests_sent_.push_back(request);
    } else {
      FAIL();
      exit(1);
    }
    EXPECT_EQ(request.ByteSizeLong(), client_.bytes_sent() - prev_bytes_sent);
    EXPECT_EQ(1ull, client_.data_points_sent() - prev_data_points_sent);
    EXPECT_EQ(1ull, client_.requests_sent() - prev_requests_sent);
  }

  void send_async(ExportLogsServiceRequest const &request) { send_async(logs_client_, request); }

  void send_async(ExportMetricsServiceRequest const &request) { send_async(metrics_client_, request); }

  template <typename TService, typename TReq, typename TResp>
  void process_async_responses(OtlpGrpcClient<TService, TReq, TResp> &client_)
  {
    // wait for async response, but no more than 30 seconds
    for (int i = 0; !client_.async_responses_.empty() && i < 300; ++i) {
      client_.process_async_responses();
      usleep(100'000);
    }
    ASSERT_TRUE(client_.async_responses_.empty());
    ASSERT_EQ(0ull, client_.unknown_response_tags());
  }

  template <typename TService, typename TReq, typename TResp>
  void validate_async_response_failures(
      OtlpGrpcClient<TService, TReq, TResp> const &client_,
      u64 expected_bytes_failed,
      u64 expected_data_points_failed,
      u64 expected_requests_failed)
  {
    EXPECT_EQ(expected_bytes_failed, client_.bytes_failed());
    EXPECT_EQ(expected_data_points_failed, client_.data_points_failed());
    EXPECT_EQ(expected_requests_failed, client_.requests_failed());
  }

  void validate_log_requests()
  {
    auto &requests_received = server_.get_log_requests_received();
    ASSERT_EQ(log_requests_sent_.size(), requests_received.size());
    ASSERT_EQ(log_requests_sent_.size(), server_.get_num_log_requests_received());
    for (size_t i = 0; i < log_requests_sent_.size(); ++i) {
      EXPECT_EQ(get_request_json(log_requests_sent_[i]), get_request_json(requests_received[i]));
    }
  }

  void validate_metric_requests()
  {
    auto &requests_received = server_.get_metric_requests_received();
    ASSERT_EQ(metric_requests_sent_.size(), requests_received.size());
    ASSERT_EQ(metric_requests_sent_.size(), server_.get_num_metric_requests_received());
    for (size_t i = 0; i < metric_requests_sent_.size(); ++i) {
      EXPECT_EQ(get_request_json(metric_requests_sent_[i]), get_request_json(requests_received[i]));
    }
  }

protected:
  OtlpGrpcClient<LogsService, ExportLogsServiceRequest, ExportLogsServiceResponse> logs_client_;
  OtlpGrpcClient<MetricsService, ExportMetricsServiceRequest, ExportMetricsServiceResponse> metrics_client_;
  otlp_test_server::OtlpGrpcTestServer server_;

  std::vector<ExportLogsServiceRequest> log_requests_sent_;
  std::vector<ExportMetricsServiceRequest> metric_requests_sent_;
};

TEST_F(OtlpGrpcClientTest, SyncLogs)
{
  ExportLogsServiceRequest request = create_logs_request();
  auto status = logs_client_.Export(request);
  EXPECT_TRUE(status.ok()) << "RPC failed: " << status.error_code() << ": " << log_waive(status.error_message());
  EXPECT_EQ(1ul, server_.get_num_log_requests_received());
}

TEST_F(OtlpGrpcClientTest, AsyncLogs)
{
  send_async(create_logs_request());
  process_async_responses(logs_client_);
  EXPECT_EQ(1ul, server_.get_num_log_requests_received());
  validate_async_response_failures(logs_client_, 0ull, 0ull, 0ull);
  validate_log_requests();

  send_async(create_logs_request());
  process_async_responses(logs_client_);
  EXPECT_EQ(2ul, server_.get_num_log_requests_received());
  validate_async_response_failures(logs_client_, 0ull, 0ull, 0ull);
  validate_log_requests();

  server_.stop();

  // Async response for this request will indicate failure since the server was previously stopped.
  LOG::debug("Sending async request to gRPC server that has been shutdown (will log 'RPC failed' error)");
  ExportLogsServiceRequest request = create_logs_request();
  send_async(request);
  process_async_responses(logs_client_);
  validate_async_response_failures(logs_client_, request.ByteSizeLong(), 1, 1);
}

TEST_F(OtlpGrpcClientTest, SyncMetrics)
{
  ExportMetricsServiceRequest request = create_metrics_request();
  auto status = metrics_client_.Export(request);
  EXPECT_TRUE(status.ok()) << "RPC failed: " << status.error_code() << ": " << log_waive(status.error_message());
  EXPECT_EQ(1ul, server_.get_num_metric_requests_received());
}

TEST_F(OtlpGrpcClientTest, AsyncMetrics)
{
  send_async(create_metrics_request());
  process_async_responses(metrics_client_);
  EXPECT_EQ(1ul, server_.get_num_metric_requests_received());
  validate_async_response_failures(metrics_client_, 0, 0, 0);
  validate_metric_requests();

  send_async(create_metrics_request());
  process_async_responses(metrics_client_);
  EXPECT_EQ(2ul, server_.get_num_metric_requests_received());
  validate_async_response_failures(metrics_client_, 0, 0, 0);
  validate_metric_requests();

  server_.stop();

  // Async response for this request will indicate failure since the server was previously stopped.
  LOG::debug("Sending async request to gRPC server that has been shutdown (will log 'RPC failed' error)");
  ExportMetricsServiceRequest request = create_metrics_request();
  send_async(request);
  process_async_responses(metrics_client_);
  validate_async_response_failures(metrics_client_, request.ByteSizeLong(), 1, 1);
}

} // namespace otlp_client
