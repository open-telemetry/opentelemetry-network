// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "npm_connection.h"

namespace reducer::ingest {

NpmConnection::NpmConnection(::ebpf_net::ingest::Index &index)
    : transform_builder_(), protocol_(transform_builder_), connection_(protocol_, index), time_tracker_()
{}

int NpmConnection::handle(const char *msg, uint32_t len)
{
  auto const handled = protocol_.handle(msg, len);
  time_tracker_.message_received(handled.client_timestamp);
  return handled.result;
}

int NpmConnection::handle_multiple(const char *msg, u64 len)
{
  auto const handled = protocol_.handle_multiple(msg, len);
  time_tracker_.message_received(handled.client_timestamp);
  return handled.result;
}

std::chrono::nanoseconds NpmConnection::clock_offset() const
{
  return time_tracker_.clock_offset();
}

std::chrono::nanoseconds NpmConnection::time_since_last_message() const
{
  return time_tracker_.time_since_last_message();
}

void NpmConnection::set_client_info(std::string_view hostname, ClientType type)
{
  client_hostname_ = hostname;
  client_type_ = type;
}

} // namespace reducer::ingest
