/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include <platform/types.h>

#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>

#include <unordered_map>

using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;
using opentelemetry::proto::collector::metrics::v1::MetricsService;

namespace otlp_client {

class OtlpGrpcMetricsClient {
public:
  OtlpGrpcMetricsClient(std::shared_ptr<grpc::Channel> channel);
  ~OtlpGrpcMetricsClient();

  grpc::Status Export(ExportMetricsServiceRequest const &request);

  void AsyncExport(ExportMetricsServiceRequest const &request);

  void process_async_responses();

  u64 bytes_failed() const { return bytes_failed_; }
  u64 bytes_sent() const { return bytes_sent_; }
  u64 metrics_failed() const { return metrics_failed_; }
  u64 metrics_sent() const { return metrics_sent_; }
  u64 requests_failed() const { return requests_failed_; }
  u64 requests_sent() const { return requests_sent_; }
  u64 unknown_response_tags() const { return unknown_response_tags_; }

private:
  std::unique_ptr<MetricsService::Stub> stub_;

  grpc::CompletionQueue cq_;

  struct AsyncResponse {
    grpc::ClientContext context_;
    grpc::Status status_;
    ExportMetricsServiceResponse response_;

    std::unique_ptr<grpc::ClientAsyncResponseReader<ExportMetricsServiceResponse>> response_reader_;

    u64 num_bytes = 0;
    u64 num_metrics = 0;
  };

  static u64 next_async_response_tag_;
  std::unordered_map<u64, std::unique_ptr<AsyncResponse>> async_responses_;

  u64 bytes_failed_ = 0;
  u64 bytes_sent_ = 0;
  u64 metrics_failed_ = 0;
  u64 metrics_sent_ = 0;
  u64 requests_failed_ = 0;
  u64 requests_sent_ = 0;
  u64 unknown_response_tags_ = 0;
};

} /* namespace otlp_client */
