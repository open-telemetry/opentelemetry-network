/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include "publisher.h"

#include <optional>
#include <string>
#include <vector>

namespace reducer {
class InternalMetricsEncoder;

// Dummy class to stop publishing internal metrics.

class NullPublisher : public Publisher {
public:
  class Writer;

  NullPublisher();

  virtual ~NullPublisher();

  // Creates a writer object for the specified thread.
  virtual WriterPtr make_writer(size_t thread_num) override;

  // Gets this writer's stats encoded for TSDB output.
  virtual void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const override;

private:
  std::string server_address_and_port_;
};

// Writer for NullPublisher.
//
class NullPublisher::Writer : public Publisher::Writer {
public:
  Writer(size_t thread_num, std::string const &server_address_and_port);

  Writer(Writer const &) = delete;
  Writer(Writer &&) = default;

  ~Writer();

  // Writes provided stream's content.
  void write(std::stringstream &ss) override;

  // Writes prefix, followed by labels, finished with suffix.
  void write(std::string_view prefix, std::string_view labels, std::string_view suffix) override;

  void flush() override;

  // Gets this writer's stats encoded for TSDB output.
  void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const override;

private:
  size_t thread_num_;
  std::string server_address_and_port_;
};

} // namespace reducer
