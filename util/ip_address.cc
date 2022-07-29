// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "util/ip_address.h"

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <iostream>
#include <string>

IPv6Address IPv6Address::from(const std::array<uint64_t, 2> &buffer)
{
  return from(reinterpret_cast<const uint8_t *>(buffer.data()));
}

IPv6Address IPv6Address::from(const std::array<uint32_t, 4> &buffer)
{
  return from(reinterpret_cast<const uint8_t *>(buffer.data()));
}

IPv6Address IPv6Address::from_host_hextets(const Hextets &hextets)
{
  Hextets network_hextets;
  std::transform(hextets.begin(), hextets.end(), network_hextets.begin(), htons);
  return IPv6Address(network_hextets);
}

IPv6Address IPv6Address::from(const uint8_t buffer[16])
{
  const uint16_t *const buffer16 = reinterpret_cast<const uint16_t *>(buffer);
  Hextets hextets;
  std::copy(buffer16, buffer16 + 8, hextets.begin());
  return IPv6Address(hextets);
}

Expected<IPv6Address, std::error_code> IPv6Address::parse(char const *ip_string)
{
  struct in6_addr address;
  switch (inet_pton(AF_INET6, ip_string, &address)) {
  case 0:
    return {unexpected, std::make_error_code(std::errc::invalid_argument)};

  case 1:
    return IPv6Address::from(address);

  default:
    return {unexpected, std::make_error_code(std::errc::address_family_not_supported)};
  }
}

u128 IPv6Address::as_int() const
{
  u128 result;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  // nothing to do
  result = *reinterpret_cast<u128 const *>(hextets_.data());
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  auto src = reinterpret_cast<u8 const *>(hextets_.data());
  auto dst = reinterpret_cast<u8 *>(&result);
  std::reverse_copy(src, src + sizeof(result), dst);
#else // __BYTE_ORDER__
#error "unsupported byte order"
#endif // __BYTE_ORDER__
  return result;
}

void IPv6Address::write_to(std::array<uint64_t, 2> *const buffer) const
{
  write_to(reinterpret_cast<uint8_t *>(buffer->data()));
}

void IPv6Address::write_to(std::array<uint32_t, 4> *const buffer) const
{
  write_to(reinterpret_cast<uint8_t *>(buffer->data()));
}

void IPv6Address::write_to(uint8_t buffer[16]) const
{
  std::copy(hextets_.begin(), hextets_.end(), reinterpret_cast<uint16_t *>(buffer));
}

std::string IPv6Address::str() const
{
  char dst[INET6_ADDRSTRLEN];
  struct in6_addr src;
  write_to(src.s6_addr);

  if (inet_ntop(AF_INET6, &src, dst, INET6_ADDRSTRLEN) == nullptr) {
    return "";
  }
  return std::string(dst);
}

std::string IPv6Address::tidy_string() const
{
  if (auto ipv4 = to_ipv4(); ipv4.has_value()) {
    return ipv4->str();
  } else {
    return str();
  }
}

bool IPv6Address::is_localhost() const
{
  return *this == localhost();
}

bool IPv6Address::is_zero() const
{
  for (auto const i : hextets_) {
    if (i) {
      return false;
    }
  }
  return true;
}

bool IPv6Address::is_ipv4() const
{
  // Is IPv4 if it is in the form of ::FFFF:{octet1}:{octet2}:{octet3}:{octet4}
  for (int i = 0; i < 5; i++) {
    if (hextets_[i] != 0)
      return false;
  }
  return hextets_[5] == 0xFFFF;
}

std::optional<IPv4Address> IPv6Address::to_ipv4() const
{
  if (!is_ipv4()) {
    return std::nullopt;
  }

  uint32_t const *quadlet = reinterpret_cast<uint32_t const *>(&hextets_[6]);

  return IPv4Address::from(*quadlet);
}

bool IPv6Address::operator==(const IPv6Address &rhs) const
{
  return hextets_ == rhs.hextets_;
}

std::ostream &operator<<(std::ostream &os, const IPv6Address &ipv6)
{
  return os << ipv6.str();
}

IPv4Address IPv4Address::from(const uint32_t buffer)
{
  union {
    uint32_t int_val;
    std::array<uint8_t, 4> bytes;
  } rep;
  rep.int_val = buffer;
  return IPv4Address(rep.bytes);
}

Expected<IPv4Address, std::error_code> IPv4Address::parse(char const *ip_string)
{
  struct in_addr address;
  switch (inet_pton(AF_INET, ip_string, &address)) {
  case 0:
    return {unexpected, std::make_error_code(std::errc::invalid_argument)};

  case 1:
    return IPv4Address::from(address);

  default:
    return {unexpected, std::make_error_code(std::errc::address_family_not_supported)};
  }
}

uint32_t IPv4Address::as_int() const
{
  return *reinterpret_cast<const uint32_t *>(octets_.data());
}

std::string IPv4Address::str() const
{
  char dst[INET_ADDRSTRLEN];
  struct in_addr src {
    .s_addr = as_int(),
  };

  if (inet_ntop(AF_INET, &src, dst, INET_ADDRSTRLEN) == nullptr) {
    return "";
  }
  return std::string(dst);
}

bool IPv4Address::is_localhost() const
{
  const uint32_t kLocalhostPrefix = IPv4Address({127, 0, 0, 0}).as_int();
  const uint32_t kLocalhostMask = IPv4Address({0xff, 0xff, 0xff, 0}).as_int();
  return (as_int() & kLocalhostMask) == kLocalhostPrefix;
}

bool IPv4Address::is_zero() const
{
  for (auto const i : octets_) {
    if (i) {
      return false;
    }
  }
  return true;
}

IPv6Address IPv4Address::to_ipv6() const
{
  if (is_localhost()) {
    return IPv6Address::localhost();
  }
  uint8_t buffer[16] = {};
  std::copy(octets_.begin(), octets_.end(), &buffer[12]);
  buffer[10] = 0xff;
  buffer[11] = 0xff;
  return IPv6Address::from(buffer);
}

bool IPv4Address::operator==(const IPv4Address &rhs) const
{
  return octets_ == rhs.octets_;
}

std::ostream &operator<<(std::ostream &os, const IPv4Address &ipv4)
{
  return os << ipv4.str();
}
