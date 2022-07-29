// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <string>
#include <unordered_map>

#include "gtest/gtest.h"

#include "util/lookup3_hasher.h"

namespace util {
namespace {

TEST(Lookup3HasherTest, Integer)
{
  Lookup3Hasher<u64> h;
  std::hash<u64> s;

  EXPECT_NE(h(0), 0u);
  EXPECT_NE(h(1), 0u);

  EXPECT_NE(h(123), s(123));

  Lookup3Hasher<u32> h32;
  EXPECT_EQ(h32(20), h(20));
}

TEST(Lookup3HasherTest, StringHash)
{
  Lookup3Hasher<std::string> h;
  std::hash<std::string> s;

  EXPECT_NE(h(""), 0u);
  EXPECT_NE(h("hello"), s("hello"));
}

TEST(Lookup3HasherTest, UseInMap)
{
  std::unordered_map<int, int, ::util::Lookup3Hasher<int>> m;
  m[10] = 20;
  m[20] = 30;

  EXPECT_EQ(m[10], 20);
  EXPECT_EQ(m[20], 30);
}

} // namespace
} // namespace util
