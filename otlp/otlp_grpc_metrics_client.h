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

#pragma once

#include <config.h>

#include <google/protobuf/util/json_util.h>
#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>

using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;
using opentelemetry::proto::collector::metrics::v1::MetricsService;

namespace otlp {

class OtlpGrpcMetricsClient {
public:
  OtlpGrpcMetricsClient(std::shared_ptr<grpc::Channel> channel);

  // Temporary code to convert JsonTsdbFormatter format to OTLP gRPC metrics format before the work to convert directly from
  // internal metrics format to OTLP gRPC metrics format is complete.
  void FormatAndExport(std::string const &metric_string);

  grpc::Status Export(ExportMetricsServiceRequest const &request);

private:
  std::unique_ptr<MetricsService::Stub> stub_;
};

} /* namespace otlp */
