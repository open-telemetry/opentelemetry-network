/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include "publisher.h"

#include <otlp/otlp_grpc_client.h>

#include <optional>
#include <string>
#include <vector>

namespace reducer {
class InternalMetricsEncoder;

// Class used to publish logs and metrics via OTLP gRPC.
//
// Objects of this class instantiate OtlpGrpcClient(s) to send OTLP gRPC messages.
//
// To write logs and metrics the |make_writer| method is first used.
// It creates an object that exposes various writing functions.
// Each such writer can only be used from a single thread.
//
class OtlpGrpcPublisher : public Publisher {
public:
  class Writer;

  // Constructs the object and creates OtlpGrpcClient(s).
  //
  // \param num_writer_threads the number of threads that will be writing
  // \param server_address_and_port IP address and port of OTLP gRPC server
  //
  OtlpGrpcPublisher(size_t num_writer_threads, const std::string &server_address_and_port);

  virtual ~OtlpGrpcPublisher();

  // Creates a writer object for the specified thread.
  virtual WriterPtr make_writer(size_t thread_num) override;

  // Gets this writer's stats encoded for TSDB output.
  virtual void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const override;

private:
  std::string server_address_and_port_;
};

// Writer for OtlpGrpcPublisher.
//
class OtlpGrpcPublisher::Writer : public Publisher::Writer {
public:
  Writer(size_t thread_num, std::string const &server_address_and_port);

  Writer(Writer const &) = delete;
  Writer(Writer &&) = default;

  ~Writer();

  void write(ExportLogsServiceRequest &request) override;
  void write(ExportMetricsServiceRequest &request) override;

  // Note that there are no buffered metrics in this Writer to send when flush is called - OtlpGrpcFormatter::flush() deals with
  // buffered metrics that need to be sent - but this will process any outstanding async responses that have been received.
  void flush() override;

  u64 bytes_written() const override { return metrics_client_.bytes_sent() - metrics_client_.bytes_failed(); }

  u64 bytes_failed_to_write() const override { return metrics_client_.bytes_failed(); }

  void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const override;

  void write_internal_stats_to_logging_core(
      ::ebpf_net::aggregation::auto_handles::agg_core_stats &agg_core_stats,
      u64 time_ns,
      int shard,
      std::string_view module) const override;

private:
  size_t thread_num_;
  std::string server_address_and_port_;
  otlp_client::OtlpGrpcClient<LogsService, ExportLogsServiceRequest, ExportLogsServiceResponse> logs_client_;
  otlp_client::OtlpGrpcClient<MetricsService, ExportMetricsServiceRequest, ExportMetricsServiceResponse> metrics_client_;
};

} // namespace reducer
