// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vector>

#include <grpcpp/grpcpp.h>

namespace otlp_client {

template <typename TService, typename TReq, typename TResp> class OtlpGrpcTestServer {
public:
  OtlpGrpcTestServer(std::string &server_addr) : server_addr_(server_addr) {}

  void start()
  {
    thr_ = std::thread([&]() {
      grpc::ServerBuilder builder;
      builder.AddListeningPort(server_addr_, grpc::InsecureServerCredentials());
      builder.RegisterService(&service_);
      server_ = builder.BuildAndStart();

      server_->Wait();
    });
  }

  void stop()
  {
    server_->Shutdown();
    thr_.join();
  }

  std::vector<TReq> &get_requests_received() { return service_.requests_received_; }

private:
  class ServiceImpl final : public TService::Service {
    grpc::Status Export(grpc::ServerContext *context, const TReq *request, TResp *response) override
    {
      LOG::trace("gRPC server Export() received request={}", log_waive(get_request_json(*request)));
      requests_received_.push_back(*request);
      return grpc::Status::OK;
    }

  public:
    std::vector<TReq> requests_received_;
  };

  const std::string server_addr_;
  ServiceImpl service_;
  std::thread thr_;
  std::unique_ptr<grpc::Server> server_;
};

} /* namespace otlp_client */
