//
// Copyright 2022 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <opentelemetry/proto/logs/v1/logs.pb.h>

#include <otlp/otlp_grpc_logs_client.h>

#include <ctime>

namespace otlp {

OtlpGrpcLogsClient::OtlpGrpcLogsClient(std::shared_ptr<grpc::Channel> channel) : stub_(LogsService::NewStub(channel)) {}

grpc::Status OtlpGrpcLogsClient::Export(ExportLogsServiceRequest const &request)
{
  ExportLogsServiceResponse response;
  grpc::ClientContext context;

  return stub_->Export(&context, request, &response);
}

} /* namespace otlp */
