/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>

#include <generated/ebpf_net/aggregation/auto_handles.h>

#include <chrono>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <string_view>

namespace reducer {
class InternalMetricsEncoder;

// Abstract class representing publishers that send time-series data to a
// time-series database.
//
class Publisher {
public:
  class Writer {
  public:
    virtual ~Writer() = default;

    // Derived classes need to override version(s) of write() they support.

    // Writes provided stream's content.
    virtual void write(std::stringstream &ss)
    {
      std::cerr << "Publisher::Writer::write(stringstream) not supported" << std::endl;
      std::abort();
    }

    // Writes prefix, followed by labels, finished with suffix.
    virtual void write(std::string_view prefix, std::string_view labels, std::string_view suffix)
    {
      std::cerr << "Publisher::Writer::write(prefix, labels, suffix) not supported" << std::endl;
      std::abort();
    }

    // Finishes this and starts a new batch.
    virtual void flush() = 0;

    // Number of bytes successfully written.
    virtual u64 bytes_written() const { return 0; }

    // Number of bytes that were not written.
    virtual u64 bytes_failed_to_write() const { return 0; }

    // Publishes this Writer's internal metrics.  This version is called from the logging core to publish internal metrics on
    // the logging core's Publisher::Writer that publishes internal metrics.
    virtual void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const {}

    // Publishes this Writer's internal metrics.  This version is called from the aggregation core and sends the internal
    // metrics to the logging core to be processed and published as appropriate.
    virtual void write_internal_stats_to_logging_core(
        ::ebpf_net::aggregation::auto_handles::agg_core_stats &agg_core_stats,
        u64 time_ns,
        int shard,
        std::string_view module) const {};
  };

  using WriterPtr = std::unique_ptr<Writer>;

  virtual ~Publisher() = default;

  // Creates a writer object for the specified thread.
  virtual WriterPtr make_writer(size_t thread_num) = 0;

  // Gets this publisher's stats encoded for TSDB output.
  virtual void write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const {}

  // Performs whatever shutdown actions are needed, e.g. closing sockets,
  // stopping threads, etc.
  virtual void shutdown() {}
};

} // namespace reducer
