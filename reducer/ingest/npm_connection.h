/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <channel/tcp_channel.h>

#include <generated/ebpf_net/ingest/connection.h>
#include <generated/ebpf_net/ingest/index.h>
#include <generated/ebpf_net/ingest/protocol.h>
#include <generated/ebpf_net/ingest/transform_builder.h>
#include <generated/ebpf_net/ingest/weak_refs.h>

#include <reducer/util/time_tracker.h>
#include <util/fixed_hash.h>

namespace reducer::ingest {

class NpmConnection {
public:
  NpmConnection(::ebpf_net::ingest::Index &index);

  int handle(const char *msg, uint32_t len);

  int handle_multiple(const char *msg, u64 len);

  ebpf_net::ingest::Connection *ingest_connection() { return &connection_; }

  std::chrono::nanoseconds clock_offset() const;
  std::chrono::nanoseconds time_since_last_message() const;

  void set_client_info(std::string_view hostname, ClientType type);

  std::string_view client_hostname() const { return client_hostname_; }

  ClientType client_type() const { return client_type_; }

private:
  ebpf_net::ingest::TransformBuilder transform_builder_;
  ebpf_net::ingest::Protocol protocol_;
  ebpf_net::ingest::Connection connection_;
  TimeTracker time_tracker_;
  std::string_view client_hostname_ = kUnknown;
  ClientType client_type_ = ClientType::unknown;
};

} // namespace reducer::ingest
