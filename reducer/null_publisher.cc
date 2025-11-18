// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include <reducer/null_publisher.h>
#include <util/time.h>

namespace reducer {

NullPublisher::NullPublisher() {}

NullPublisher::~NullPublisher() {}

Publisher::WriterPtr NullPublisher::make_writer(size_t thread_num)
{
  return std::make_unique<Writer>(thread_num, server_address_and_port_);
}

void NullPublisher::write_internal_stats(InternalMetricsEncoder &encoder, u64 time_ns) const {}

////////////////////////////////////////////////////////////////////////////////
// Writer
//

NullPublisher::Writer::Writer(size_t thread_num, std::string const &server_address_and_port)
    : thread_num_(thread_num), server_address_and_port_(server_address_and_port)
{}

NullPublisher::Writer::~Writer() {}

void NullPublisher::Writer::write(std::stringstream &ss) {}

void NullPublisher::Writer::write(std::string_view prefix, std::string_view labels, std::string_view suffix) {}

void NullPublisher::Writer::flush() {}

void NullPublisher::Writer::write_internal_stats(
    InternalMetricsEncoder &encoder, u64 time_ns, int shard, std::string_view module) const
{}

} // namespace reducer
