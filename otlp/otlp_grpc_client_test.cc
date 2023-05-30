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

template <typename TService, typename TReq, typename TResp> class OtlpGrpcClientTester {
public:
  OtlpGrpcClientTester(std::string &server_addr)
      : client_(grpc::CreateChannel(server_addr, grpc::InsecureChannelCredentials())), server_(server_addr)
  {
    start_server();
  }

  ~OtlpGrpcClientTester() { stop_server(); }

  void start_server()
  {
    server_.start();
  }

  void stop_server() { server_.stop(); }

  void send_async(TReq const &request)
  {
    auto prev_bytes_sent = client_.bytes_sent();
    auto prev_data_points_sent = client_.data_points_sent();
    auto prev_requests_sent = client_.requests_sent();

    client_.AsyncExport(request);
    requests_sent_.push_back(request);
    EXPECT_EQ(request.ByteSizeLong(), client_.bytes_sent() - prev_bytes_sent);
    EXPECT_EQ(1, client_.data_points_sent() - prev_data_points_sent);
    EXPECT_EQ(1, client_.requests_sent() - prev_requests_sent);
  }

  void process_responses()
  {
    // wait for async response, but no more than 30 seconds
    for (int i = 0; !client_.async_responses_.empty() && i < 300; ++i) {
      client_.process_async_responses();
      usleep(100'000);
    }
    ASSERT_TRUE(client_.async_responses_.empty());
    ASSERT_EQ(0, client_.unknown_response_tags());
  }

  void
  validate_async_response_failures(u64 expected_bytes_failed, u64 expected_data_points_failed, u64 expected_requests_failed)
  {
    EXPECT_EQ(expected_bytes_failed, client_.bytes_failed());
    EXPECT_EQ(expected_data_points_failed, client_.data_points_failed());
    EXPECT_EQ(expected_requests_failed, client_.requests_failed());
  }

  void validate_requests()
  {
    auto &requests_received = server_.get_requests_received();
    ASSERT_EQ(requests_sent_.size(), requests_received.size());
    ASSERT_EQ(requests_sent_.size(), server_.get_num_requests_received());
    for (size_t i = 0; i < requests_sent_.size(); ++i) {
      EXPECT_EQ(get_request_json(requests_sent_[i]), get_request_json(requests_received[i]));
    }
  };

  OtlpGrpcClient<TService, TReq, TResp> client_;
  otlp_test_server::OtlpGrpcTestServer<TService, TReq, TResp> server_;

private:
  std::vector<TReq> requests_sent_;
};

class OtlpGrpcLogsClientTest : public CommonTest {
protected:
  OtlpGrpcLogsClientTest() : tester_(grpc_test_server_addr) {}

  void SetUp() override { CommonTest::SetUp(); }

  ExportLogsServiceRequest create_request()
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

  OtlpGrpcClientTester<LogsService, ExportLogsServiceRequest, ExportLogsServiceResponse> tester_;
};

class OtlpGrpcMetricsClientTest : public CommonTest {
protected:
  OtlpGrpcMetricsClientTest() : tester_(grpc_test_server_addr) {}

  void SetUp() override { CommonTest::SetUp(); }

  ExportMetricsServiceRequest create_request()
  {
    return otlp_client::OtlpRequestBuilder()
        .metric("test-metric-name")
        .sum()
        .number_data_point(
            456u,
            {{"label1", "value1"}, {"label2", "value2"}},
            std::chrono::nanoseconds(std::chrono::system_clock::now().time_since_epoch()));
  }

  OtlpGrpcClientTester<MetricsService, ExportMetricsServiceRequest, ExportMetricsServiceResponse> tester_;
};

TEST_F(OtlpGrpcLogsClientTest, SyncLogs)
{
  ExportLogsServiceRequest request = create_request();
  auto status = tester_.client_.Export(request);
  EXPECT_TRUE(status.ok()) << "RPC failed: " << status.error_code() << ": " << log_waive(status.error_message());
  EXPECT_EQ(1, tester_.server_.get_num_requests_received());
}

TEST_F(OtlpGrpcLogsClientTest, AsyncLogs)
{
  tester_.send_async(create_request());
  tester_.process_responses();
  EXPECT_EQ(1, tester_.server_.get_num_requests_received());
  tester_.validate_async_response_failures(0, 0, 0);
  tester_.validate_requests();

  tester_.send_async(create_request());
  tester_.process_responses();
  EXPECT_EQ(2, tester_.server_.get_num_requests_received());
  tester_.validate_async_response_failures(0, 0, 0);
  tester_.validate_requests();

  tester_.stop_server();

  // Async response for this request will indicate failure since the server was previously stopped.
  ExportLogsServiceRequest request = create_request();
  tester_.send_async(request);
  tester_.process_responses();
  tester_.validate_async_response_failures(request.ByteSizeLong(), 1, 1);
}

TEST_F(OtlpGrpcMetricsClientTest, SyncMetrics)
{
  ExportMetricsServiceRequest request = create_request();
  auto status = tester_.client_.Export(request);
  EXPECT_TRUE(status.ok()) << "RPC failed: " << status.error_code() << ": " << log_waive(status.error_message());
  EXPECT_EQ(1, tester_.server_.get_num_requests_received());
}

TEST_F(OtlpGrpcMetricsClientTest, AsyncMetrics)
{
  tester_.send_async(create_request());
  tester_.process_responses();
  EXPECT_EQ(1, tester_.server_.get_num_requests_received());
  tester_.validate_async_response_failures(0, 0, 0);
  tester_.validate_requests();

  tester_.send_async(create_request());
  tester_.process_responses();
  EXPECT_EQ(2, tester_.server_.get_num_requests_received());
  tester_.validate_async_response_failures(0, 0, 0);
  tester_.validate_requests();

  tester_.stop_server();

  // Async response for this request will indicate failure since the server was previously stopped.
  ExportMetricsServiceRequest request = create_request();
  tester_.send_async(request);
  tester_.process_responses();
  tester_.validate_async_response_failures(request.ByteSizeLong(), 1, 1);
}

} // namespace otlp_client
