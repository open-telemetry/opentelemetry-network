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

#include <util/fixed_hash.h>

#include <gtest/gtest.h>

#include <set>

template <typename Hash, typename... Values> void test_values_iteration(Values... values)
{
  Hash hash;
  std::set<typename Hash::value_type> inserted;

  // trick to expand the variadic argument in insert() calls using fold expressions
  std::size_t const n = ((hash.insert(values, values), inserted.insert(values), 1) + ... + 0);
  EXPECT_EQ(sizeof...(values), n);

  EXPECT_EQ(sizeof...(values), hash.size());
  EXPECT_EQ(sizeof...(values), inserted.size());

  std::set<typename Hash::value_type> found;
  for (auto const &value : hash.values()) {
    found.insert(value);
  }

  EXPECT_EQ(inserted.size(), found.size());
  auto f = found.begin();
  for (auto i = inserted.begin(); i != inserted.end(); ++i) {
    ASSERT_NE(f, found.end());
    EXPECT_EQ(*i, *f);
    ++f;
  }
  EXPECT_EQ(f, found.end());
}

TEST(fixed_hash, values)
{
  test_values_iteration<FixedHash<int, int, 100, std::hash<int>>>();

  test_values_iteration<FixedHash<int, int, 100, std::hash<int>>>(0, 1, 2, 3);

  test_values_iteration<FixedHash<int, int, 100, std::hash<int>>>(0, 10, 20, 30, 40, 50, 60, 70);
}
