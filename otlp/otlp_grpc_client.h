/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include <platform/types.h>
#include <util/code_timing.h>
#include <util/log.h>

#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>

#include <unordered_map>

using opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest;
using opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse;
using opentelemetry::proto::collector::logs::v1::LogsService;

using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;
using opentelemetry::proto::collector::metrics::v1::MetricsService;

namespace otlp_client {

template <typename TService, typename TReq, typename TResp> class OtlpGrpcClient {
public:
  OtlpGrpcClient(std::shared_ptr<grpc::Channel> channel) : stub_(TService::NewStub(channel)) {}

  virtual ~OtlpGrpcClient() { cq_.Shutdown(); }

  virtual grpc::Status Export(TReq const &request)
  {
    SCOPED_TIMING(OtlpGrpcClientExport);

    TResp response;
    grpc::ClientContext context;

    return stub_->Export(&context, request, &response);
  }

  virtual void AsyncExport(TReq const &request)
  {
    SCOPED_TIMING(OtlpGrpcClientAsyncExport);

    u64 async_response_tag = next_async_response_tag_++;

    auto async_response = std::make_unique<AsyncResponse>();
    async_response->response_reader_ = stub_->PrepareAsyncExport(&async_response->context_, request, &cq_);
    async_response->response_reader_->StartCall();

    async_response->response_reader_->Finish(
        &async_response->response_, &async_response->status_, reinterpret_cast<void *>(async_response_tag));

    async_response->num_bytes = request.ByteSizeLong();
    if constexpr (std::is_same_v<ExportLogsServiceRequest, TReq>) {
      for (auto const &resource_logs : request.resource_logs()) {
        for (auto const &scope_logs : resource_logs.scope_logs()) {
          async_response->num_entries += scope_logs.log_records_size();
        }
      }
    } else if constexpr (std::is_same_v<ExportMetricsServiceRequest, TReq>) {
      for (auto const &resource_metrics : request.resource_metrics()) {
        for (auto const &scope_metrics : resource_metrics.scope_metrics()) {
          async_response->num_entries += scope_metrics.metrics_size();
        }
      }
    } else {
      async_response->num_entries = 0;
    }

    bytes_sent_ += async_response->num_bytes;
    entries_sent_ += async_response->num_entries;
    ++requests_sent_;

    async_responses_.emplace(async_response_tag, std::move(async_response));
  }

  void process_async_responses()
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
          if (!async_response->status_.ok()) {
            LOG::debug(
                "RPC failed for tag={}: {}: {}",
                reinterpret_cast<u64>(tag),
                async_response->status_.error_code(),
                log_waive(async_response->status_.error_message()));
            bytes_failed_ += async_response->num_bytes;
            entries_failed_ += async_response->num_entries;
            ++requests_failed_;
          }
          async_responses_.erase(itr);
        } else {
          ++unknown_response_tags_;
        }
        break;
      }
    }
  }

  u64 bytes_failed() const { return bytes_failed_; }
  u64 bytes_sent() const { return bytes_sent_; }
  u64 entries_failed() const { return entries_failed_; }
  u64 entries_sent() const { return entries_sent_; }
  u64 requests_failed() const { return requests_failed_; }
  u64 requests_sent() const { return requests_sent_; }
  u64 unknown_response_tags() const { return unknown_response_tags_; }

private:
  std::unique_ptr<typename TService::Stub> stub_;

  grpc::CompletionQueue cq_;

  struct AsyncResponse {
    grpc::ClientContext context_;
    grpc::Status status_;
    TResp response_;

    std::unique_ptr<grpc::ClientAsyncResponseReader<TResp>> response_reader_;

    u64 num_bytes = 0;
    u64 num_entries = 0;
  };

  static u64 next_async_response_tag_;
  std::unordered_map<u64, std::unique_ptr<AsyncResponse>> async_responses_;

  u64 bytes_failed_ = 0;
  u64 bytes_sent_ = 0;
  u64 entries_failed_ = 0;
  u64 entries_sent_ = 0;
  u64 requests_failed_ = 0;
  u64 requests_sent_ = 0;
  u64 unknown_response_tags_ = 0;
};

template <typename TService, typename TReq, typename TResp>
u64 OtlpGrpcClient<TService, TReq, TResp>::next_async_response_tag_ = 0;

} /* namespace otlp_client */
