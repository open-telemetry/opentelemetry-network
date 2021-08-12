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

#include <gtest/gtest.h>

#include <util/enum.h>

#include <sstream>
#include <vector>

#define ENUM_NAME DenseEnum
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(a, 0)                                                                                                                      \
  X(b, 1)                                                                                                                      \
  X(c, 2)                                                                                                                      \
  X(d, 3)                                                                                                                      \
  X(e, 4)
#define ENUM_DEFAULT a
#include <util/enum_operators.inl>

#define ENUM_NAME DenseEnum1Base
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(a, 1)                                                                                                                      \
  X(b, 2)                                                                                                                      \
  X(c, 3)                                                                                                                      \
  X(d, 4)                                                                                                                      \
  X(e, 5)
#define ENUM_DEFAULT a
#include <util/enum_operators.inl>

#define ENUM_NAME SparseEnum
#define ENUM_TYPE std::int8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(a, -20)                                                                                                                    \
  X(b, -17)                                                                                                                    \
  X(c, 0)                                                                                                                      \
  X(d, 1)                                                                                                                      \
  X(e, 18)                                                                                                                     \
  X(f, 90)
#define ENUM_DEFAULT a
#include <util/enum_operators.inl>

TEST(enum_traits_test, min)
{
  EXPECT_EQ(static_cast<int>(DenseEnum::a), static_cast<int>(enum_traits<DenseEnum>::min()));
  EXPECT_EQ(static_cast<int>(DenseEnum1Base::a), static_cast<int>(enum_traits<DenseEnum1Base>::min()));
  EXPECT_EQ(static_cast<int>(SparseEnum::a), static_cast<int>(enum_traits<SparseEnum>::min()));
}

TEST(enum_traits_test, constexpr_min)
{
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<DenseEnum, DenseEnum::a>,
               std::integral_constant<DenseEnum, enum_traits<DenseEnum>::min()>>));
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<DenseEnum1Base, DenseEnum1Base::a>,
               std::integral_constant<DenseEnum1Base, enum_traits<DenseEnum1Base>::min()>>));
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<SparseEnum, SparseEnum::a>,
               std::integral_constant<SparseEnum, enum_traits<SparseEnum>::min()>>));
}

TEST(enum_traits_test, max)
{
  EXPECT_EQ(
      static_cast<enum_traits<DenseEnum>::int_type>(DenseEnum::e),
      static_cast<enum_traits<DenseEnum>::int_type>(enum_traits<DenseEnum>::max()));
  EXPECT_EQ(
      static_cast<enum_traits<DenseEnum1Base>::int_type>(DenseEnum1Base::e),
      static_cast<enum_traits<DenseEnum1Base>::int_type>(enum_traits<DenseEnum1Base>::max()));
  EXPECT_EQ(
      static_cast<enum_traits<SparseEnum>::int_type>(SparseEnum::f),
      static_cast<enum_traits<SparseEnum>::int_type>(enum_traits<SparseEnum>::max()));
}

TEST(enum_traits_test, constexpr_max)
{
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<DenseEnum, DenseEnum::e>,
               std::integral_constant<DenseEnum, enum_traits<DenseEnum>::max()>>));
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<DenseEnum1Base, DenseEnum1Base::e>,
               std::integral_constant<DenseEnum1Base, enum_traits<DenseEnum1Base>::max()>>));
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<SparseEnum, SparseEnum::f>,
               std::integral_constant<SparseEnum, enum_traits<SparseEnum>::max()>>));
}

TEST(enum_traits_test, count)
{
  EXPECT_EQ(5, enum_traits<DenseEnum>::count);
  EXPECT_EQ(5, enum_traits<DenseEnum1Base>::count);
  EXPECT_EQ(6, enum_traits<SparseEnum>::count);
}

TEST(enum_traits_test, constexpr_count)
{
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<std::size_t, 5>,
               std::integral_constant<std::size_t, enum_traits<DenseEnum>::count>>));
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<std::size_t, 5>,
               std::integral_constant<std::size_t, enum_traits<DenseEnum1Base>::count>>));
  EXPECT_TRUE((std::is_same_v<
               std::integral_constant<std::size_t, 6>,
               std::integral_constant<std::size_t, enum_traits<SparseEnum>::count>>));
}

TEST(enum_traits_test, values)
{
  constexpr std::array<DenseEnum, enum_traits<DenseEnum>::count> dense_values = {
      DenseEnum::a, DenseEnum::b, DenseEnum::c, DenseEnum::d, DenseEnum::e};
  EXPECT_TRUE(dense_values == enum_traits<DenseEnum>::values);

  constexpr std::array<DenseEnum1Base, enum_traits<DenseEnum1Base>::count> dense1base_values = {
      DenseEnum1Base::a, DenseEnum1Base::b, DenseEnum1Base::c, DenseEnum1Base::d, DenseEnum1Base::e};
  EXPECT_TRUE(dense1base_values == enum_traits<DenseEnum1Base>::values);

  constexpr std::array<SparseEnum, enum_traits<SparseEnum>::count> sparse_values = {
      SparseEnum::a, SparseEnum::b, SparseEnum::c, SparseEnum::d, SparseEnum::e, SparseEnum::f};
  EXPECT_TRUE(sparse_values == enum_traits<SparseEnum>::values);
}

TEST(enum_traits_test, is_contiguous)
{
  EXPECT_TRUE(enum_traits<DenseEnum>::is_contiguous);
  EXPECT_TRUE(enum_traits<DenseEnum1Base>::is_contiguous);
  EXPECT_FALSE(enum_traits<SparseEnum>::is_contiguous);
}

TEST(enum_traits_test, index_of)
{
  EXPECT_EQ(0u, enum_index_of(DenseEnum::a));
  EXPECT_EQ(1u, enum_index_of(DenseEnum::b));
  EXPECT_EQ(2u, enum_index_of(DenseEnum::c));
  EXPECT_EQ(3u, enum_index_of(DenseEnum::d));
  EXPECT_EQ(4u, enum_index_of(DenseEnum::e));

  EXPECT_EQ(0u, enum_index_of(DenseEnum1Base::a));
  EXPECT_EQ(1u, enum_index_of(DenseEnum1Base::b));
  EXPECT_EQ(2u, enum_index_of(DenseEnum1Base::c));
  EXPECT_EQ(3u, enum_index_of(DenseEnum1Base::d));
  EXPECT_EQ(4u, enum_index_of(DenseEnum1Base::e));

  EXPECT_EQ(0u, enum_index_of(SparseEnum::a));
  EXPECT_EQ(1u, enum_index_of(SparseEnum::b));
  EXPECT_EQ(2u, enum_index_of(SparseEnum::c));
  EXPECT_EQ(3u, enum_index_of(SparseEnum::d));
  EXPECT_EQ(4u, enum_index_of(SparseEnum::e));
}

TEST(enum_traits_test, array_map)
{
  EXPECT_TRUE((std::is_same_v<enum_traits<DenseEnum>::array_map<int>, std::array<int, 5>>));
  EXPECT_TRUE((std::is_same_v<enum_traits<DenseEnum>::array_map<std::string>, std::array<std::string, 5>>));

  EXPECT_TRUE((std::is_same_v<enum_traits<DenseEnum1Base>::array_map<int>, std::array<int, 5>>));
  EXPECT_TRUE((std::is_same_v<enum_traits<DenseEnum1Base>::array_map<std::string>, std::array<std::string, 5>>));
}

TEST(enum_test, to_string)
{
  EXPECT_EQ("a", to_string(DenseEnum::a));
  EXPECT_EQ("b", to_string(DenseEnum::b));
  EXPECT_EQ("c", to_string(DenseEnum::c));
  EXPECT_EQ("d", to_string(DenseEnum::d));
  EXPECT_EQ("e", to_string(DenseEnum::e));
  EXPECT_EQ("a", to_string(static_cast<DenseEnum>(9999)));
  EXPECT_EQ("invalid", to_string(static_cast<DenseEnum>(9999), "invalid"));

  EXPECT_EQ("a", to_string(DenseEnum1Base::a));
  EXPECT_EQ("b", to_string(DenseEnum1Base::b));
  EXPECT_EQ("c", to_string(DenseEnum1Base::c));
  EXPECT_EQ("d", to_string(DenseEnum1Base::d));
  EXPECT_EQ("e", to_string(DenseEnum1Base::e));
  EXPECT_EQ("a", to_string(static_cast<DenseEnum1Base>(9999)));
  EXPECT_EQ("invalid", to_string(static_cast<DenseEnum1Base>(9999), "invalid"));

  EXPECT_EQ("a", to_string(SparseEnum::a));
  EXPECT_EQ("b", to_string(SparseEnum::b));
  EXPECT_EQ("c", to_string(SparseEnum::c));
  EXPECT_EQ("d", to_string(SparseEnum::d));
  EXPECT_EQ("e", to_string(SparseEnum::e));
  EXPECT_EQ("f", to_string(SparseEnum::f));
  EXPECT_EQ("a", to_string(static_cast<SparseEnum>(9999)));
  EXPECT_EQ("invalid", to_string(static_cast<SparseEnum>(9999), "invalid"));
}

TEST(enum_traits_test, is_valid)
{
  EXPECT_TRUE(enum_traits<DenseEnum>::is_valid(DenseEnum::a));
  EXPECT_TRUE(enum_traits<DenseEnum>::is_valid(DenseEnum::b));
  EXPECT_TRUE(enum_traits<DenseEnum>::is_valid(DenseEnum::c));
  EXPECT_TRUE(enum_traits<DenseEnum>::is_valid(DenseEnum::d));
  EXPECT_TRUE(enum_traits<DenseEnum>::is_valid(DenseEnum::e));
  EXPECT_FALSE(enum_traits<DenseEnum>::is_valid(static_cast<DenseEnum>(9999)));

  EXPECT_TRUE(enum_traits<DenseEnum1Base>::is_valid(DenseEnum1Base::a));
  EXPECT_TRUE(enum_traits<DenseEnum1Base>::is_valid(DenseEnum1Base::b));
  EXPECT_TRUE(enum_traits<DenseEnum1Base>::is_valid(DenseEnum1Base::c));
  EXPECT_TRUE(enum_traits<DenseEnum1Base>::is_valid(DenseEnum1Base::d));
  EXPECT_TRUE(enum_traits<DenseEnum1Base>::is_valid(DenseEnum1Base::e));
  EXPECT_FALSE(enum_traits<DenseEnum1Base>::is_valid(static_cast<DenseEnum1Base>(9999)));

  EXPECT_TRUE(enum_traits<SparseEnum>::is_valid(SparseEnum::a));
  EXPECT_TRUE(enum_traits<SparseEnum>::is_valid(SparseEnum::b));
  EXPECT_TRUE(enum_traits<SparseEnum>::is_valid(SparseEnum::c));
  EXPECT_TRUE(enum_traits<SparseEnum>::is_valid(SparseEnum::d));
  EXPECT_TRUE(enum_traits<SparseEnum>::is_valid(SparseEnum::e));
  EXPECT_TRUE(enum_traits<SparseEnum>::is_valid(SparseEnum::f));
  EXPECT_FALSE(enum_traits<SparseEnum>::is_valid(static_cast<SparseEnum>(9999)));
}

TEST(enum_test, sanitize_enum)
{
  EXPECT_TRUE(DenseEnum::a == sanitize_enum(DenseEnum::a));
  EXPECT_TRUE(DenseEnum::b == sanitize_enum(DenseEnum::b));
  EXPECT_TRUE(DenseEnum::c == sanitize_enum(DenseEnum::c));
  EXPECT_TRUE(DenseEnum::d == sanitize_enum(DenseEnum::d));
  EXPECT_TRUE(DenseEnum::e == sanitize_enum(DenseEnum::e));
  EXPECT_TRUE(DenseEnum::a == sanitize_enum(static_cast<DenseEnum>(9999)));

  EXPECT_TRUE(DenseEnum1Base::a == sanitize_enum(DenseEnum1Base::a));
  EXPECT_TRUE(DenseEnum1Base::b == sanitize_enum(DenseEnum1Base::b));
  EXPECT_TRUE(DenseEnum1Base::c == sanitize_enum(DenseEnum1Base::c));
  EXPECT_TRUE(DenseEnum1Base::d == sanitize_enum(DenseEnum1Base::d));
  EXPECT_TRUE(DenseEnum1Base::e == sanitize_enum(DenseEnum1Base::e));
  EXPECT_TRUE(DenseEnum1Base::a == sanitize_enum(static_cast<DenseEnum1Base>(9999)));

  EXPECT_TRUE(SparseEnum::a == sanitize_enum(SparseEnum::a));
  EXPECT_TRUE(SparseEnum::b == sanitize_enum(SparseEnum::b));
  EXPECT_TRUE(SparseEnum::c == sanitize_enum(SparseEnum::c));
  EXPECT_TRUE(SparseEnum::d == sanitize_enum(SparseEnum::d));
  EXPECT_TRUE(SparseEnum::e == sanitize_enum(SparseEnum::e));
  EXPECT_TRUE(SparseEnum::f == sanitize_enum(SparseEnum::f));
  EXPECT_TRUE(SparseEnum::a == sanitize_enum(static_cast<SparseEnum>(9999)));
}

template <typename Enum> void test_enum_set_ostream_operator(EnumSet<Enum> const &set, std::string_view expected)
{
  std::ostringstream actual;

  actual << set;

  EXPECT_EQ(expected, actual.str());
}

template <typename Enum> void test_enum_set_iterator(EnumSet<Enum> const &set, std::vector<Enum> expected)
{
  EXPECT_EQ(set.begin() == set.end(), expected.empty());
  EXPECT_EQ(set.begin() != set.end(), !expected.empty());

  {
    auto i = set.begin();

    std::size_t count = 0;
    for (auto const e : expected) {
      ASSERT_NE(set.end(), i);
      EXPECT_TRUE(e == *i);
      ++i;
      ++count;
      EXPECT_EQ(i == set.end(), count == expected.size());
    }

    EXPECT_EQ(set.end(), i);
  }

  {
    auto i = set.begin();

    for (auto const e : expected) {
      ASSERT_NE(set.end(), i);
      EXPECT_TRUE(e == *i);
      EXPECT_NE(set.end(), i++);
    }

    EXPECT_EQ(set.end(), i);
  }
}

template <typename Enum> void test_enum_set()
{
  EnumSet<Enum> set;
  EXPECT_TRUE(set.empty());
  EXPECT_EQ(0u, set.size());
  EXPECT_FALSE(set.contains(Enum::a));
  EXPECT_FALSE(set.contains(Enum::b));
  EXPECT_FALSE(set.contains(Enum::c));
  EXPECT_FALSE(set.contains(Enum::d));
  EXPECT_FALSE(set.contains(static_cast<Enum>(9999)));
  EXPECT_EQ(0b00000u, set.bit_mask());
  EXPECT_TRUE(set == EnumSet<Enum>{});
  EXPECT_FALSE(set != EnumSet<Enum>{});
  EXPECT_FALSE(set == EnumSet<Enum>{}.add(Enum::d));
  EXPECT_TRUE(set != EnumSet<Enum>{}.add(Enum::d));
  test_enum_set_iterator(set, {});
  test_enum_set_ostream_operator(set, "");

  set.add(Enum::a);
  EXPECT_FALSE(set.empty());
  EXPECT_EQ(1u, set.size());
  EXPECT_TRUE(set.contains(Enum::a));
  EXPECT_FALSE(set.contains(Enum::b));
  EXPECT_FALSE(set.contains(Enum::c));
  EXPECT_FALSE(set.contains(Enum::d));
  EXPECT_FALSE(set.contains(Enum::e));
  EXPECT_FALSE(set.contains(static_cast<Enum>(9999)));
  EXPECT_EQ(0b00001u, set.bit_mask());
  EXPECT_TRUE(set == EnumSet<Enum>{}.add(Enum::a));
  EXPECT_FALSE(set != EnumSet<Enum>{}.add(Enum::a));
  EXPECT_FALSE(set == EnumSet<Enum>{}.add(Enum::d));
  EXPECT_TRUE(set != EnumSet<Enum>{}.add(Enum::d));
  test_enum_set_iterator(set, {Enum::a});
  test_enum_set_ostream_operator(set, "a");

  set.add(Enum::b);
  EXPECT_FALSE(set.empty());
  EXPECT_EQ(2u, set.size());
  EXPECT_TRUE(set.contains(Enum::a));
  EXPECT_TRUE(set.contains(Enum::b));
  EXPECT_FALSE(set.contains(Enum::c));
  EXPECT_FALSE(set.contains(Enum::d));
  EXPECT_FALSE(set.contains(Enum::e));
  EXPECT_FALSE(set.contains(static_cast<Enum>(9999)));
  EXPECT_EQ(0b00011u, set.bit_mask());
  EXPECT_TRUE(set == EnumSet<Enum>{}.add(Enum::a).add(Enum::b));
  EXPECT_FALSE(set != EnumSet<Enum>{}.add(Enum::a).add(Enum::b));
  EXPECT_FALSE(set == EnumSet<Enum>{}.add(Enum::d));
  EXPECT_TRUE(set != EnumSet<Enum>{}.add(Enum::d));
  test_enum_set_iterator(set, {Enum::a, Enum::b});
  test_enum_set_ostream_operator(set, "a,b");

  set.add(Enum::a);
  EXPECT_FALSE(set.empty());
  EXPECT_EQ(2u, set.size());
  EXPECT_TRUE(set.contains(Enum::a));
  EXPECT_TRUE(set.contains(Enum::b));
  EXPECT_FALSE(set.contains(Enum::c));
  EXPECT_FALSE(set.contains(Enum::d));
  EXPECT_FALSE(set.contains(Enum::e));
  EXPECT_FALSE(set.contains(static_cast<Enum>(9999)));
  EXPECT_EQ(0b00011u, set.bit_mask());
  EXPECT_TRUE(set == EnumSet<Enum>{}.add(Enum::a).add(Enum::b));
  EXPECT_FALSE(set != EnumSet<Enum>{}.add(Enum::a).add(Enum::b));
  EXPECT_FALSE(set == EnumSet<Enum>{}.add(Enum::d));
  EXPECT_TRUE(set != EnumSet<Enum>{}.add(Enum::d));
  test_enum_set_iterator(set, {Enum::a, Enum::b});
  test_enum_set_ostream_operator(set, "a,b");

  set.add(Enum::e);
  EXPECT_FALSE(set.empty());
  EXPECT_EQ(3u, set.size());
  EXPECT_TRUE(set.contains(Enum::a));
  EXPECT_TRUE(set.contains(Enum::b));
  EXPECT_FALSE(set.contains(Enum::c));
  EXPECT_FALSE(set.contains(Enum::d));
  EXPECT_TRUE(set.contains(Enum::e));
  EXPECT_FALSE(set.contains(static_cast<Enum>(9999)));
  EXPECT_EQ(0b10011u, set.bit_mask());
  EXPECT_TRUE(set == EnumSet<Enum>{}.add(Enum::a).add(Enum::b).add(Enum::e));
  EXPECT_FALSE(set != EnumSet<Enum>{}.add(Enum::a).add(Enum::b).add(Enum::e));
  EXPECT_FALSE(set == EnumSet<Enum>{}.add(Enum::d));
  EXPECT_TRUE(set != EnumSet<Enum>{}.add(Enum::d));
  test_enum_set_iterator(set, {Enum::a, Enum::b, Enum::e});
  test_enum_set_ostream_operator(set, "a,b,e");

  set.add(static_cast<Enum>(9999));
  EXPECT_FALSE(set.empty());
  EXPECT_EQ(3u, set.size());
  EXPECT_TRUE(set.contains(Enum::a));
  EXPECT_TRUE(set.contains(Enum::b));
  EXPECT_FALSE(set.contains(Enum::c));
  EXPECT_FALSE(set.contains(Enum::d));
  EXPECT_TRUE(set.contains(Enum::e));
  EXPECT_FALSE(set.contains(static_cast<Enum>(9999)));
  EXPECT_EQ(0b10011u, set.bit_mask());
  EXPECT_TRUE(set == EnumSet<Enum>{}.add(Enum::a).add(Enum::b).add(Enum::e));
  EXPECT_FALSE(set != EnumSet<Enum>{}.add(Enum::a).add(Enum::b).add(Enum::e));
  EXPECT_FALSE(set == EnumSet<Enum>{}.add(Enum::d));
  EXPECT_TRUE(set != EnumSet<Enum>{}.add(Enum::d));
  test_enum_set_iterator(set, {Enum::a, Enum::b, Enum::e});
  test_enum_set_ostream_operator(set, "a,b,e");
}

TEST(enum_set_test, DenseEnum)
{
  test_enum_set<DenseEnum>();
}

TEST(enum_set_test, DenseEnum1Base)
{
  test_enum_set<DenseEnum1Base>();
}

TEST(enum_set_test, SparseEnum)
{
  test_enum_set<SparseEnum>();
}
