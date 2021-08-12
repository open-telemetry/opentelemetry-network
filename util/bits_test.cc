//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <util/bits.h>

#include <gtest/gtest.h>

#include <type_traits>

#define EXPECT_SAME(Expected, ...) EXPECT_TRUE((std::is_same_v<Expected, __VA_ARGS__>))

TEST(bits, smallest_unsigned_integer)
{
  EXPECT_SAME(bool, smallest_unsigned_integer<0>);
  EXPECT_SAME(bool, smallest_unsigned_integer<1>);

  EXPECT_SAME(u8, smallest_unsigned_integer<2>);
  EXPECT_SAME(u8, smallest_unsigned_integer<3>);
  EXPECT_SAME(u8, smallest_unsigned_integer<4>);
  EXPECT_SAME(u8, smallest_unsigned_integer<5>);
  EXPECT_SAME(u8, smallest_unsigned_integer<6>);
  EXPECT_SAME(u8, smallest_unsigned_integer<7>);
  EXPECT_SAME(u8, smallest_unsigned_integer<8>);

  EXPECT_SAME(u16, smallest_unsigned_integer<9>);
  EXPECT_SAME(u16, smallest_unsigned_integer<10>);
  EXPECT_SAME(u16, smallest_unsigned_integer<11>);
  EXPECT_SAME(u16, smallest_unsigned_integer<12>);
  EXPECT_SAME(u16, smallest_unsigned_integer<13>);
  EXPECT_SAME(u16, smallest_unsigned_integer<14>);
  EXPECT_SAME(u16, smallest_unsigned_integer<15>);
  EXPECT_SAME(u16, smallest_unsigned_integer<16>);

  EXPECT_SAME(u32, smallest_unsigned_integer<17>);
  EXPECT_SAME(u32, smallest_unsigned_integer<24>);
  EXPECT_SAME(u32, smallest_unsigned_integer<32>);

  EXPECT_SAME(u64, smallest_unsigned_integer<33>);
  EXPECT_SAME(u64, smallest_unsigned_integer<48>);
  EXPECT_SAME(u64, smallest_unsigned_integer<64>);

  EXPECT_SAME(u128, smallest_unsigned_integer<65>);
  EXPECT_SAME(u128, smallest_unsigned_integer<99>);
  EXPECT_SAME(u128, smallest_unsigned_integer<128>);
}

TEST(bits, count_bits_set)
{
  EXPECT_EQ(0u, count_bits_set(0b0000));
  EXPECT_EQ(1u, count_bits_set(0b0001));
  EXPECT_EQ(1u, count_bits_set(0b0010));
  EXPECT_EQ(3u, count_bits_set(0b1011));
  EXPECT_EQ(2u, count_bits_set(0b1100));
}

TEST(bits, least_significant_bit_index)
{
  EXPECT_EQ(0u, least_significant_bit_index(0b0001));
  EXPECT_EQ(1u, least_significant_bit_index(0b0010));
  EXPECT_EQ(0u, least_significant_bit_index(0b1011));
  EXPECT_EQ(3u, least_significant_bit_index(0b1000));
}

TEST(bits, disable_least_significant_bit)
{
  EXPECT_EQ(0b0000, disable_least_significant_bit(0b0000));
  EXPECT_EQ(0b0000, disable_least_significant_bit(0b0001));
  EXPECT_EQ(0b0000, disable_least_significant_bit(0b0010));
  EXPECT_EQ(0b1010, disable_least_significant_bit(0b1011));
  EXPECT_EQ(0b1000, disable_least_significant_bit(0b1100));
}

TEST(bits, least_significant_bit)
{
  EXPECT_EQ(0b0000, least_significant_bit(0b0000));
  EXPECT_EQ(0b0001, least_significant_bit(0b0001));
  EXPECT_EQ(0b0010, least_significant_bit(0b0010));
  EXPECT_EQ(0b0001, least_significant_bit(0b1011));
  EXPECT_EQ(0b0100, least_significant_bit(0b1100));
}

TEST(bits, make_bits)
{
  EXPECT_EQ(0b0001, make_bit<int>(0));
  EXPECT_EQ(0b0010, make_bit<int>(1));
  EXPECT_EQ(0b0100, make_bit<int>(2));
  EXPECT_EQ(0b1000, make_bit<int>(3));
}
