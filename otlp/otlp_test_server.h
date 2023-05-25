// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <otlp/otlp_util.h>
#include <util/common_test.h>

#include <vector>

#include <grpcpp/grpcpp.h>

namespace otlp_test_server {

template <typename TService, typename TReq, typename TResp> class OtlpGrpcTestServer {
public:
  OtlpGrpcTestServer(std::string const &server_addr) : server_addr_(server_addr) {}

  void start()
  {
    ASSERT_FALSE(server_is_running_);
    thr_ = std::thread([&]() {
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
};

} /* namespace otlp_test_server */
