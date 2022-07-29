/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/platform.h>
#include <util/lookup3.h>

#include <functional>

// Struct represents the 4-tuple of a connection
// NOTE: when used as a key, src_port and dst_port are in network byte order
// NOTE: hasher relies on the alignment of variables
struct hostport_tuple {
  u32 src_ip;
  u32 dst_ip;
  u16 src_port;
  u16 dst_port;
  u32 proto;

  bool operator==(const hostport_tuple &rhs) const
  {
    return (this->src_ip == rhs.src_ip) && (this->dst_ip == rhs.dst_ip) && (this->src_port == rhs.src_port) &&
           (this->dst_port == rhs.dst_port) && (this->proto == rhs.proto);
  }

  hostport_tuple reversed() const { return {dst_ip, src_ip, dst_port, src_port, proto}; }
};

namespace std {
template <> struct hash<hostport_tuple> {
  size_t operator()(const hostport_tuple &t) const noexcept
  {
    return (std::size_t)lookup3_hashword((uint32_t *)&t, 4, 0x9e3779b9);
  }
};
} // namespace std
