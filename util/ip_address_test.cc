// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "util/ip_address.h"

#include <absl/strings/str_format.h>
#include <array>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>

namespace server {
namespace {

using ::testing::ContainerEq;
using ::testing::Property;

TEST(IPv6AddressTest, WriteToInt64Buffer)
{
  IPv6Address address = IPv6Address::from_host_hextets({0x0011, 0x2233, 0x4455, 0x6677, 0x8899, 0xaabb, 0xccdd, 0xeeff});
  std::array<uint64_t, 2> expected = {htobe64(0x0011223344556677ull), htobe64(0x8899aabbccddeeffull)};
  if (htole16(0xff00) != 0xff00) {
    std::swap(expected[0], expected[1]);
  }

  std::array<uint64_t, 2> actual;
  address.write_to(&actual);
  EXPECT_THAT(actual, ContainerEq(expected));
}

TEST(IPv6AddressTest, WriteToInt32Buffer)
{
  IPv6Address address = IPv6Address::from_host_hextets({0x0011, 0x2233, 0x4455, 0x6677, 0x8899, 0xaabb, 0xccdd, 0xeeff});
  std::array<uint32_t, 4> expected = {htonl(0x00112233ul), htonl(0x44556677ul), htonl(0x8899aabbul), htonl(0xccddeefful)};
  std::array<uint32_t, 4> actual;
  address.write_to(&actual);
  EXPECT_THAT(actual, ContainerEq(expected));
}

TEST(IPv6AddressTest, WriteToByteBuffer)
{
  IPv6Address address = IPv6Address::from_host_hextets({0x0011, 0x2233, 0x4455, 0x6677, 0x8899, 0xaabb, 0xccdd, 0xeeff});
  std::array<uint8_t, 16> expected = {
      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  std::array<uint8_t, 16> actual;
  address.write_to(actual.data());
  EXPECT_THAT(actual, ContainerEq(expected));
}

TEST(IPv6AddressTest, FromInt64Buffer)
{
  std::array<uint64_t, 2> buffer = {htobe64(0x0011223344556677ull), htobe64(0x8899aabbccddeeffull)};
  if (htole16(0xff00) != 0xff00) {
    std::swap(buffer[0], buffer[1]);
  }

  IPv6Address actual = IPv6Address::from(buffer);
  IPv6Address expected = IPv6Address::from_host_hextets({0x0011, 0x2233, 0x4455, 0x6677, 0x8899, 0xaabb, 0xccdd, 0xeeff});
  EXPECT_EQ(expected, actual);
}

TEST(IPv6AddressTest, FromInt32Buffer)
{
  std::array<uint32_t, 4> buffer = {htonl(0x00112233ul), htonl(0x44556677ul), htonl(0x8899aabbul), htonl(0xccddeefful)};
  IPv6Address actual = IPv6Address::from(buffer);
  IPv6Address expected = IPv6Address::from_host_hextets({0x0011, 0x2233, 0x4455, 0x6677, 0x8899, 0xaabb, 0xccdd, 0xeeff});
  EXPECT_EQ(expected, actual);
}

TEST(IPv6AddressTest, FromByteBuffer)
{
  std::array<uint8_t, 16> buffer = {
      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  IPv6Address actual = IPv6Address::from(buffer.data());
  IPv6Address expected = IPv6Address::from_host_hextets({0x0011, 0x2233, 0x4455, 0x6677, 0x8899, 0xaabb, 0xccdd, 0xeeff});
  EXPECT_EQ(expected, actual);
}

TEST(IPv6AddressTest, FromByteBuffer_ByteAlignment)
{
  struct {
    uint64_t a = 0;
    uint8_t b = 0xff;
    uint8_t buffer[16] = {
        0x00,
        0x11,
        0x22,
        0x33,
        0x44,
        0x55,
        0x66,
        0x77,
        0x88,
        0x99,
        0xaa,
        0xbb,
        0xcc,
        0xdd,
        0xee,
        0xff,
    };
  } test_struct;

  IPv6Address actual = IPv6Address::from(test_struct.buffer);
  IPv6Address expected = IPv6Address::from_host_hextets({0x0011, 0x2233, 0x4455, 0x6677, 0x8899, 0xaabb, 0xccdd, 0xeeff});
  EXPECT_EQ(expected, actual);
}

TEST(IPv6AddressTest, AsInt)
{
  {
    u8 bytes[] = {
        0x00,
        0x11,
        0x22,
        0x33,
        0x44,
        0x55,
        0x66,
        0x77,
        0x88,
        0x99,
        0xaa,
        0xbb,
        0xcc,
        0xdd,
        0xee,
        0xff,
    };
    auto const address = IPv6Address::from(bytes);
    EXPECT_EQ((std::uint64_t)(address.as_int() >> 64), 0x0011223344556677ull);
    EXPECT_EQ((std::uint64_t)(address.as_int() & UINT64_MAX), 0x8899aabbccddeeffull);
  }

  {
    u8 bytes[] = {0xf3, 0xe4, 0xd5, 0xc6, 0xb7, 0xa8, 0x99, 0x8a, 0x7b, 0x6c, 0x5d, 0x4e, 0x3f, 0x20, 0x11, 0x02};
    auto const address = IPv6Address::from(bytes);
    EXPECT_EQ((std::uint64_t)(address.as_int() >> 64), 0xf3e4d5c6b7a8998aull);
    EXPECT_EQ((std::uint64_t)(address.as_int() & UINT64_MAX), 0x7b6c5d4e3f201102ull);
  }
}

TEST(IPv6AddressTest, Str)
{
  EXPECT_EQ(IPv6Address().str(), "::");
  EXPECT_EQ(IPv6Address::localhost().str(), "::1");
  EXPECT_EQ(IPv6Address::from_host_hextets({0xdead, 0, 0, 0, 0, 0, 0, 0xbeef}).str(), "dead::beef");
}

TEST(IPv6AddressTest, bytes_view)
{
  EXPECT_TRUE(as_short_string(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1) == IPv6Address::localhost().bytes_view());

  EXPECT_TRUE(
      as_short_string(0xde, 0xad, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xbe, 0xef) ==
      IPv6Address::from_host_hextets({0xdead, 0, 0, 0, 0, 0, 0, 0xbeef}).bytes_view());
}

TEST(IPv6AddressTest, IsLocalhost)
{
  EXPECT_FALSE(IPv6Address().is_localhost());
  EXPECT_TRUE(IPv6Address::localhost().is_localhost());
  EXPECT_FALSE(IPv6Address::from_host_hextets({0xdead, 0, 0, 0, 0, 0, 0, 0xbeef}).is_localhost());
}

TEST(IPv6AddressTest, IsIpv4)
{
  EXPECT_FALSE(IPv6Address().is_ipv4());
  EXPECT_FALSE(IPv6Address::localhost().is_ipv4());
  EXPECT_FALSE(IPv6Address::from_host_hextets({0xdead, 0, 0, 0, 0, 0, 0, 0xbeef}).is_ipv4());
  EXPECT_TRUE(IPv6Address::from_host_hextets({0, 0, 0, 0, 0, 0xffff, 0xdead, 0xbeef}).is_ipv4());
}

TEST(IPv6AddressTest, ToIpv4)
{
  auto expect_addr = IPv6Address::parse("::ffff:127.0.0.1");
  EXPECT_TRUE(expect_addr.has_value());
  auto addr = expect_addr.value();
  EXPECT_TRUE(addr.is_ipv4());
  EXPECT_TRUE(addr.to_ipv4().has_value());
  EXPECT_TRUE(addr.to_ipv4()->is_localhost());
}

TEST(IPv6AddressTest, localhost)
{
  auto const localhost = IPv6Address::localhost();
  EXPECT_TRUE(localhost.is_localhost());
}

TEST(IPv6AddressTest, parse_localhost)
{
  auto const result = IPv6Address::parse("localhost");
  EXPECT_FALSE(result);
}

TEST(IPv6AddressTest, parse___1)
{
  auto const result = IPv6Address::parse("::1");
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(IPv6Address::localhost(), ip);

  EXPECT_TRUE(ip.is_localhost());
}

TEST(IPv6AddressTest, parse___)
{
  auto const result = IPv6Address::parse("::");
  auto const expected = IPv6Address::from({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);
}

TEST(IPv6AddressTest, parse_2607_f8b0_4000_811__2004)
{
  auto const result = IPv6Address::parse("2607:f8b0:4000:811::2004");
  auto const expected = IPv6Address::from({0x26, 0x07, 0xf8, 0xb0, 0x40, 0, 0x8, 0x11, 0, 0, 0, 0, 0, 0, 0x20, 0x04});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);
}

TEST(IPv6AddressTest, parse_127_0_0_1)
{
  auto const result = IPv6Address::parse("127.0.0.1");
  EXPECT_FALSE(result);
}

TEST(IPv6AddressTest, parse_192_168_0_1)
{
  auto const result = IPv6Address::parse("192.168.0.1");
  EXPECT_FALSE(result);
}

TEST(IPv6AddressTest, parse_123_456_789_0)
{
  auto const result = IPv6Address::parse("123.456.789.0");
  EXPECT_FALSE(result);
}

TEST(IPv6AddressTest, parse_hello_world)
{
  auto const result = IPv6Address::parse("hello, world!");
  EXPECT_FALSE(result);
}

// ipv4

TEST(IPv4AddressTest, FromInt)
{
  IPv4Address actual = IPv4Address::from(0x33221100);
  IPv4Address expected({0x00, 0x11, 0x22, 0x33});
  EXPECT_EQ(actual, expected);
}

TEST(IPv4AddressTest, AsInt)
{
  IPv4Address address({0x00, 0x11, 0x22, 0x33});
  EXPECT_EQ(address.as_int(), 0x33221100U) << absl::StrFormat("actual = %s (0x%08x)", address.str(), address.as_int());
}

TEST(IPv4AddressTest, Str)
{
  EXPECT_EQ(IPv4Address().str(), "0.0.0.0");
  EXPECT_EQ(IPv4Address({50, 100, 150, 200}).str(), "50.100.150.200");
}

TEST(IPv4AddressTest, bytes_view)
{
  EXPECT_TRUE(as_short_string(127, 0, 0, 1) == IPv4Address::localhost().bytes_view());

  EXPECT_TRUE(as_short_string(0xde, 0xad, 0xbe, 0xef) == IPv4Address({0xde, 0xad, 0xbe, 0xef}).bytes_view());
}

TEST(IPv4AddressTest, IsLocalhost)
{
  EXPECT_FALSE(IPv4Address().is_localhost());
  EXPECT_FALSE(IPv4Address({128, 0, 0, 1}).is_localhost());
  EXPECT_TRUE(IPv4Address({127, 0, 0, 1}).is_localhost());
}

TEST(IPv4AddressTest, ToIpv6)
{
  IPv6Address actual = IPv4Address({1, 2, 3, 4}).to_ipv6();
  IPv6Address expected = IPv6Address::from_host_hextets({0, 0, 0, 0, 0, 0xffff, 0x0102, 0x0304});
  EXPECT_EQ(expected, actual);
  EXPECT_TRUE(actual.is_ipv4());
}

TEST(IPv4AddressTest, localhost)
{
  auto const localhost = IPv4Address::localhost();
  EXPECT_TRUE(localhost.is_localhost());
  EXPECT_EQ(IPv4Address({127, 0, 0, 1}), localhost);
}

TEST(IPv4AddressTest, localhost_0)
{
  auto const localhost = IPv4Address::localhost(0);
  EXPECT_TRUE(localhost.is_localhost());
  EXPECT_EQ(IPv4Address({127, 0, 0, 0}), localhost);
}

TEST(IPv4AddressTest, localhost_10)
{
  auto const localhost = IPv4Address::localhost(10);
  EXPECT_TRUE(localhost.is_localhost());
  EXPECT_EQ(IPv4Address({127, 0, 0, 10}), localhost);
}

TEST(IPv4AddressTest, localhost_255)
{
  auto const localhost = IPv4Address::localhost(255);
  EXPECT_TRUE(localhost.is_localhost());
  EXPECT_EQ(IPv4Address({127, 0, 0, 255}), localhost);
}

TEST(IPv4AddressTest, localhost_to_ipv6)
{
  auto const ipv4_localhost = IPv4Address::localhost();
  auto const ipv6_localhost = ipv4_localhost.to_ipv6();
  EXPECT_FALSE(ipv6_localhost.is_ipv4());
  EXPECT_TRUE(ipv6_localhost.is_localhost());
}

TEST(IPv4AddressTest, parse_localhost)
{
  auto const result = IPv4Address::parse("localhost");
  EXPECT_FALSE(result);
}

TEST(IPv4AddressTest, parse_127_0_0_1)
{
  auto const result = IPv4Address::parse("127.0.0.1");
  auto const expected = IPv4Address({127, 0, 0, 1});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);

  EXPECT_TRUE(ip.is_localhost());
  EXPECT_EQ(IPv4Address::localhost(), ip);
}

TEST(IPv4AddressTest, parse_127_0_0_0)
{
  auto const result = IPv4Address::parse("127.0.0.0");
  auto const expected = IPv4Address({127, 0, 0, 0});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);

  EXPECT_TRUE(ip.is_localhost());
}

TEST(IPv4AddressTest, parse_127_0_0_255)
{
  auto const result = IPv4Address::parse("127.0.0.255");
  auto const expected = IPv4Address({127, 0, 0, 255});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);

  EXPECT_TRUE(ip.is_localhost());
}

TEST(IPv4AddressTest, parse_192_168_0_1)
{
  auto const result = IPv4Address::parse("192.168.0.1");
  auto const expected = IPv4Address({192, 168, 0, 1});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);
}

TEST(IPv4AddressTest, parse_10_0_0_1)
{
  auto const result = IPv4Address::parse("10.0.0.1");
  auto const expected = IPv4Address({10, 0, 0, 1});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);
}

TEST(IPv4AddressTest, parse_1_2_3_4)
{
  auto const result = IPv4Address::parse("1.2.3.4");
  auto const expected = IPv4Address({1, 2, 3, 4});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);
}

TEST(IPv4AddressTest, parse_255_255_255_255)
{
  auto const result = IPv4Address::parse("255.255.255.255");
  auto const expected = IPv4Address({255, 255, 255, 255});
  ASSERT_TRUE(result);

  auto const ip = result.value();
  EXPECT_EQ(expected, ip);
}

TEST(IPv4AddressTest, parse_123_456_789_0)
{
  auto const result = IPv4Address::parse("123.456.789.0");
  EXPECT_FALSE(result);
}

TEST(IPv4AddressTest, parse_hello_world)
{
  auto const result = IPv4Address::parse("hello, world!");
  EXPECT_FALSE(result);
}

} // namespace
} // namespace server
