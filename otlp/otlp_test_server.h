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
using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;

namespace otlp_test_server {

template <typename TService, typename TReq, typename TResp> class OtlpGrpcTestServer {
public:
  OtlpGrpcTestServer(std::string const &server_addr) : server_addr_(server_addr)
  {
    if constexpr (std::is_same_v<ExportLogsServiceRequest, TReq>) {
      server_type_ = "logs";
    } else if constexpr (std::is_same_v<ExportMetricsServiceRequest, TReq>) {
      server_type_ = "metrics";
    }
  }

  void start()
  {
    ASSERT_FALSE(server_is_running_);
    thr_ = std::thread([&]() {
      LOG::debug("starting gRPC {} server at {}", server_type_, server_addr_);
      grpc::ServerBuilder builder;
      builder.AddListeningPort(server_addr_, grpc::InsecureServerCredentials());
      builder.RegisterService(&service_);
      server_ = builder.BuildAndStart();

      server_->Wait();
    });
    server_is_running_ = true;
  }

  void stop()
  {
    if (server_is_running_) {
      server_->Shutdown();
      thr_.join();
      server_is_running_ = false;
      LOG::debug("stopped gRPC {} server at {}", server_type_, server_addr_);
    }
  }

  std::vector<TReq> &get_requests_received() { return service_.requests_received_; }
  u64 get_num_requests_received() { return service_.num_requests_received_; }

private:
  class ServiceImpl final : public TService::Service {
    grpc::Status Export(grpc::ServerContext *context, const TReq *request, TResp *response) override
    {
      LOG::trace("gRPC server Export() received request={}", log_waive(get_request_json(*request)));
      requests_received_.push_back(*request);
      ++num_requests_received_;
      LOG::debug("gRPC server received {} request(s)", num_requests_received_);

      return grpc::Status::OK;
    }

  public:
    std::vector<TReq> requests_received_;
    u64 num_requests_received_ = 0;
  };

  std::string const server_addr_;
  ServiceImpl service_;
  std::thread thr_;
  std::unique_ptr<grpc::Server> server_;
  bool server_is_running_ = false;
  std::string_view server_type_;
};

} /* namespace otlp_test_server */
