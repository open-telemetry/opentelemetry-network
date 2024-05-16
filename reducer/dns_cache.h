/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/ingest/keys.h>
#include <netinet/in.h>
#include <platform/platform.h>
#include <util/LRU.h>
#include <util/ip_address.h>

namespace reducer {

/* forward declarations */
namespace dns {
struct hash_ipv6_address;
struct eq_ipv6_address;
static constexpr u32 max_len = 256;
typedef short_string<max_len> dns_record;
} // namespace dns

template <std::size_t ELEM_POOL_SZ>
class DnsCache : public LRU<IPv6Address, dns::dns_record, ELEM_POOL_SZ, dns::hash_ipv6_address, dns::eq_ipv6_address> {
public:
  using dns_record = dns::dns_record;
};

namespace dns {

struct hash_ipv6_address {
  typedef IPv6Address argument_type;
  typedef std::size_t result_type;
  result_type operator()(IPv6Address const &addr) const noexcept
  {
    std::array<uint32_t, 4> addr32;
    addr.write_to(&addr32);

    return (result_type)fp_jhash_nwords(addr32[3], addr32[2], addr32[1], addr32[0]);
  }
};

/* equality operator for ipv6 addresses */
struct eq_ipv6_address {
  bool operator()(const IPv6Address &lhs, const IPv6Address &rhs) const { return lhs == rhs; }
};

} /* namespace dns */

} /* namespace reducer */
