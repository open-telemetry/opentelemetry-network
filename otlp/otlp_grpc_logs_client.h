/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h>

using opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;
using opentelemetry::proto::collector::logs::v1::LogsService;

namespace otlp_client {

class OtlpGrpcLogsClient {
public:
  OtlpGrpcLogsClient(std::shared_ptr<grpc::Channel> channel);

  grpc::Status Export(ExportLogsServiceRequest const &request);

private:
  std::unique_ptr<LogsService::Stub> stub_;
};

} /* namespace otlp_client */
