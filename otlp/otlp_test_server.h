// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <otlp/otlp_util.h>
#include <util/common_test.h>

#include <vector>

#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>

using opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;
using opentelemetry::proto::collector::logs::v1::LogsService;

using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;
using opentelemetry::proto::collector::metrics::v1::MetricsService;

namespace otlp_test_server {

class OtlpGrpcTestServer {
public:
  OtlpGrpcTestServer(std::string const &server_addr) : server_addr_(server_addr) {}

  void start()
  {
    ASSERT_FALSE(server_is_running_);
    LOG::debug("starting gRPC logs and metrics server at {}", server_addr_);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_addr_, grpc::InsecureServerCredentials());
    builder.RegisterService(&logs_service_);
    builder.RegisterService(&metrics_service_);
    server_ = builder.BuildAndStart();
    ASSERT_NE(nullptr, server_);

    server_is_running_ = true;

    thr_ = std::thread([&]() { server_->Wait(); });
  }

  void stop()
  {
    if (server_is_running_) {
      server_->Shutdown();
      thr_.join();
      server_is_running_ = false;
      LOG::debug("stopped gRPC logs and metrics server at {}", server_addr_);
    }
  }

  std::vector<ExportLogsServiceRequest> &get_log_requests_received() { return logs_service_.requests_received_; }
  u64 get_num_log_requests_received() { return logs_service_.num_requests_received_; }

  std::vector<ExportMetricsServiceRequest> &get_metric_requests_received() { return metrics_service_.requests_received_; }
  u64 get_num_metric_requests_received() { return metrics_service_.num_requests_received_; }

private:
  template <typename TService, typename TReq, typename TResp> class ServiceImpl final : public TService::Service {
    grpc::Status Export(grpc::ServerContext *context, const TReq *request, TResp *response) override
    {
      LOG::trace("gRPC server {} service Export() received request={}", service_type(), log_waive(get_request_json(*request)));
      requests_received_.push_back(*request);
      ++num_requests_received_;
      LOG::debug("gRPC server {} service received {} request(s)", service_type(), num_requests_received_);

      return grpc::Status::OK;
    }

    std::string_view service_type()
    {
      if constexpr (std::is_same_v<ExportLogsServiceRequest, TReq>) {
        return "logs";
      } else if constexpr (std::is_same_v<ExportMetricsServiceRequest, TReq>) {
        return "metrics";
      } else {
        return "unknown";
      }
    }

  public:
    std::vector<TReq> requests_received_;
    u64 num_requests_received_ = 0;
  };

  std::string const server_addr_;
  std::unique_ptr<grpc::Server> server_;
  ServiceImpl<LogsService, ExportLogsServiceRequest, ExportLogsServiceResponse> logs_service_;
  ServiceImpl<MetricsService, ExportMetricsServiceRequest, ExportMetricsServiceResponse> metrics_service_;
  std::thread thr_;
  bool server_is_running_ = false;
};

} /* namespace otlp_test_server */
