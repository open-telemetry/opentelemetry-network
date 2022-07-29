/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// This library contains classes for performing conversions to/from IP
// addresses and other representations.

#include <platform/types.h>
#include <util/expected.h>
#include <util/short_string.h>

#include <spdlog/fmt/fmt.h>

#include <array>
#include <iostream>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <system_error>

class IPv4Address;

// Class for managing conversions to/from IPv6 addresses.
class IPv6Address {
public:
  using Hextets = std::array<uint16_t, 8>;

  // Constructs an all-zero (i.e. "::") IPv6 address.
  constexpr IPv6Address() {}

  // Constructs an address from the provided host byte order hextets.
  static IPv6Address from_host_hextets(const Hextets &hextets);

  // Parses the IPv6 address from the buffer. Assumes network byte order.
  static IPv6Address from(const std::array<uint64_t, 2> &buffer);
  static IPv6Address from(const std::array<uint32_t, 4> &buffer);
  static IPv6Address from(const uint8_t buffer[16]);

  static IPv6Address from(struct in6_addr const &address) { return IPv6Address::from(address.s6_addr); }

  static IPv6Address localhost() { return IPv6Address::from_host_hextets({0, 0, 0, 0, 0, 0, 0, 1}); }

  // Parses the null-terminated string representation of an IPv6 address.
  //
  // Returns the `IPv6Address` representation if the string represents a valid IPv6 address,
  // or an error code otherwise.
  //
  // Example:
  //
  //  auto ip_string = "::1";
  //  auto ip = IPv6Address::parse(ip_string);
  //
  //  if (!ip) {
  //    LOG::error("failed to parse ip address with error code {}", ip.error());
  //  } else {
  //    assert(raw_ip->is_localhost());
  //  }
  static Expected<IPv6Address, std::error_code> parse(char const *ip_string);

  // Writes the IPv6 address to the buffers. Writes in network byte order.
  void write_to(std::array<uint64_t, 2> *buffer) const;
  void write_to(std::array<uint32_t, 4> *buffer) const;
  void write_to(uint8_t buffer[16]) const;

  // Returns the 128-bit integer representation of this address.
  u128 as_int() const;

  // A human-readable string representation in IPv6 format.
  std::string str() const;

  // A human-readable string representation in IPv6 format for IPv6 addresses
  // and IPv4 format for IPv4 addresses.
  std::string tidy_string() const;

  // True if this address represents localhost.
  bool is_localhost() const;

  // tells whether all bytes in this address are 0
  bool is_zero() const;

  // True if this is an IPv6 of an IPv4 address.
  bool is_ipv4() const;

  // If IPv6-encoded IPv4 address, returns corresponding IPv4 address.
  // Returns nullopt otherwise.
  std::optional<IPv4Address> to_ipv4() const;

  short_string<16> bytes_view() const { return {reinterpret_cast<char const *>(hextets_.data()), 16}; }

  bool operator==(const IPv6Address &rhs) const;

  template <typename H> friend H AbslHashValue(H hash_state, const IPv6Address &addr);

private:
  // Expects the hextets to be network byte order.
  explicit IPv6Address(const Hextets &hextets) : hextets_(hextets) {}

  Hextets hextets_ = {};
};

template <typename H> H AbslHashValue(H hash_state, const IPv6Address &addr)
{
  return H::combine(std::move(hash_state), addr.hextets_);
}

std::ostream &operator<<(std::ostream &os, const IPv6Address &ipv6);

// A libfmt formatter for IPv6Address, used to print addresses in LOG::trace
// etc.
namespace fmt {
template <> struct formatter<IPv6Address> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

  template <typename FormatContext> auto format(const IPv6Address &p, FormatContext &ctx)
  {
    return format_to(ctx.begin(), "{}", p.str());
  }
};
} // namespace fmt

// Class for managing conversions to/from IPv4 addresses.
class IPv4Address {
public:
  using Octets = std::array<uint8_t, 4>;

  // Constructs an all-zero (i.e. "0.0.0.0") IPv4 address.
  constexpr IPv4Address() {}
  constexpr explicit IPv4Address(const Octets &octets) : octets_(octets) {}

  // Parses the IPv4 address from the buffer. Assumes network byte order.
  static IPv4Address from(uint32_t buffer);

  static IPv4Address from(struct in_addr const &address) { return IPv4Address::from(address.s_addr); }

  // Parses the null-terminated string representation of an IPv4 address.
  //
  // Returns the `IPv4Address` representation if the string represents a valid IPv4 address,
  // or an error code otherwise.
  //
  // Example:
  //
  //  auto ip_string = "127.0.0.1";
  //  auto ip = IPv4Address::parse(ip_string);
  //
  //  if (!ip) {
  //    LOG::error("failed to parse ip address with error code {}", ip.error());
  //  } else {
  //    assert(raw_ip->is_localhost());
  //  }
  static Expected<IPv4Address, std::error_code> parse(char const *ip_string);

  // Returns the 32-bit integer representation of this address, in network
  // byte order.
  uint32_t as_int() const;

  // A human-readable string representation.
  std::string str() const;

  // True if this address represents localhost.
  bool is_localhost() const;

  // tells whether all bytes in this address are 0
  bool is_zero() const;

  // Converts this to an IPv6 address representation.
  IPv6Address to_ipv6() const;

  /**
   * Localhost addresses are represented by 127.0.0.0/8.
   *
   * The `least_significant_octet` parameter can be used to customize the resulting address to
   * something other than the default 127.0.0.1.
   */
  static constexpr IPv4Address localhost(uint8_t least_significant_octet = 1)
  {
    return IPv4Address({127, 0, 0, least_significant_octet});
  }

  short_string<4> bytes_view() const { return {reinterpret_cast<char const *>(octets_.data()), 4}; }

  bool operator==(const IPv4Address &rhs) const;

  template <typename H> friend H AbslHashValue(H hash_state, const IPv4Address &addr);

private:
  Octets octets_ = {};
};

template <typename H> H AbslHashValue(H hash_state, const IPv4Address &addr)
{
  return H::combine(std::move(hash_state), addr.octets_);
}

std::ostream &operator<<(std::ostream &os, const IPv4Address &ipv4);
