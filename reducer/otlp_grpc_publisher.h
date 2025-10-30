/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include "publisher.h"

#include <otlp_export_cxxbridge.h>

#include <optional>
#include <string>

namespace reducer {

// Rust-backed OTLP publisher (gRPC exporter via cxx bridge)
class OtlpGrpcPublisher : public Publisher {
public:
  class Writer;

  // Constructs with number of writer threads and endpoint (host:port or full URL)
  OtlpGrpcPublisher(size_t num_writer_threads, const std::string &endpoint);
  virtual ~OtlpGrpcPublisher();

  virtual WriterPtr make_writer(size_t thread_num) override;

  virtual void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const override;

private:
  std::string endpoint_;
};

class OtlpGrpcPublisher::Writer : public Publisher::Writer {
public:
  Writer(size_t thread_num, std::string const &endpoint);
  Writer(Writer const &) = delete;
  Writer(Writer &&) = default;
  ~Writer();

  void flush() override;

  u64 bytes_written() const override;
  u64 bytes_failed_to_write() const override;

  void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const override;
  void write_internal_stats_to_logging_core(
      ::ebpf_net::aggregation::auto_handles::agg_core_stats &agg_core_stats,
      u64 time_ns,
      int shard,
      std::string_view module) const override;

  // Access to underlying Rust publisher for formatting layer
  ::Publisher &rust_publisher() { return *publisher_; }
  const ::Publisher &rust_publisher() const { return *publisher_; }

  const std::string &endpoint() const { return endpoint_; }

private:
  size_t thread_num_;
  std::string endpoint_;
  ::rust::Box<::Publisher> publisher_;
};

} // namespace reducer
