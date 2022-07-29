// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <opentelemetry/proto/metrics/v1/metrics.pb.h>

#include "otlp_grpc_metrics_client.h"

#include <util/code_timing.h>
#include <util/log.h>

namespace otlp_client {

u64 OtlpGrpcMetricsClient::next_async_response_tag_ = 0;

OtlpGrpcMetricsClient::OtlpGrpcMetricsClient(std::shared_ptr<grpc::Channel> channel) : stub_(MetricsService::NewStub(channel))
{}

OtlpGrpcMetricsClient::~OtlpGrpcMetricsClient()
{
  cq_.Shutdown();
}

grpc::Status OtlpGrpcMetricsClient::Export(ExportMetricsServiceRequest const &request)
{
  SCOPED_TIMING(OtlpGrpcMetricsClientExport);

  ExportMetricsServiceResponse response;
  grpc::ClientContext context;

  return stub_->Export(&context, request, &response);
}

void OtlpGrpcMetricsClient::AsyncExport(ExportMetricsServiceRequest const &request)
{
  SCOPED_TIMING(OtlpGrpcMetricsClientAsyncExport);

  u64 async_response_tag = next_async_response_tag_++;

  auto async_response = std::make_unique<AsyncResponse>();
  async_response->response_reader_ = stub_->PrepareAsyncExport(&async_response->context_, request, &cq_);
  async_response->response_reader_->StartCall();

  async_response->response_reader_->Finish(
      &async_response->response_, &async_response->status_, reinterpret_cast<void *>(async_response_tag));

  async_responses_.emplace(async_response_tag, std::move(async_response));

  ++requests_sent_;
  bytes_sent_ += request.ByteSizeLong();
  for (auto const &resource_metrics : request.resource_metrics()) {
    for (auto const &scope_metrics : resource_metrics.scope_metrics()) {
      metrics_sent_ += scope_metrics.metrics_size();
    }
  }
}

void OtlpGrpcMetricsClient::process_async_responses()
{
  if (async_responses_.empty()) {
    return;
  }

  void *tag = nullptr;
  bool ok;
  while (true) {
    switch (cq_.AsyncNext(&tag, &ok, gpr_time_0(GPR_CLOCK_MONOTONIC))) {
    case grpc::CompletionQueue::TIMEOUT:
    case grpc::CompletionQueue::SHUTDOWN:
      // no responses to process
      return;
      break;
    case grpc::CompletionQueue::GOT_EVENT:
      // Note: Per gRPC docs, for client-side Finish: ok should always be true, so not checking it here.
      if (auto itr = async_responses_.find(reinterpret_cast<u64>(tag)); itr != async_responses_.end()) {
        auto const &async_response = itr->second;
        if (async_response->status_.ok()) {
          ++successful_requests_;
        } else {
          LOG::debug(
              "RPC failed for tag={}: {}: {}",
              reinterpret_cast<u64>(tag),
              async_response->status_.error_code(),
              log_waive(async_response->status_.error_message()));
          ++failed_requests_;
        }
        async_responses_.erase(itr);
      } else {
        ++unknown_response_tags_;
      }
      break;
    }
  }
}

} /* namespace otlp_client */
