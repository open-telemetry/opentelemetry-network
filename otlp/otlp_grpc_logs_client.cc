// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <opentelemetry/proto/logs/v1/logs.pb.h>

#include "otlp_grpc_logs_client.h"

#include <ctime>

namespace otlp_client {

OtlpGrpcLogsClient::OtlpGrpcLogsClient(std::shared_ptr<grpc::Channel> channel) : stub_(LogsService::NewStub(channel)) {}

grpc::Status OtlpGrpcLogsClient::Export(ExportLogsServiceRequest const &request)
{
  ExportLogsServiceResponse response;
  grpc::ClientContext context;

  return stub_->Export(&context, request, &response);
}

} /* namespace otlp_client */
