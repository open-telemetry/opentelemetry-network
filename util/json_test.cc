// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <util/json.h>

#include <string>
#include <vector>

#include <cstdint>

using nlohmann::json;

// some of these tests are meant as a quick reference on how to use the JSON API

TEST(json, example_type)
{
  EXPECT_EQ(json::value_t::null, json::parse(R"json(null)json").type());
  EXPECT_EQ(json::value_t::boolean, json::parse(R"json(true)json").type());
  EXPECT_EQ(json::value_t::string, json::parse(R"json("foobar")json").type());
  EXPECT_EQ(json::value_t::number_integer, json::parse(R"json(-10)json").type());
  EXPECT_EQ(json::value_t::number_unsigned, json::parse(R"json(10)json").type());
  EXPECT_EQ(json::value_t::number_float, json::parse(R"json(5.6)json").type());
  EXPECT_EQ(json::value_t::object, json::parse(R"json({})json").type());
  EXPECT_EQ(json::value_t::array, json::parse(R"json([])json").type());
}

TEST(json, example_dictionary)
{
  std::vector<std::pair<std::string, std::int64_t>> const expected{{"a", 10}, {"b", 20}, {"c", 30}, {"d", 40}, {"e", 50}};

  auto const data = json::parse(R"json(
    {
      "a": 10,
      "b": 20,
      "c": 30,
      "d": 40,
      "e": 50
    }
  )json");

  EXPECT_EQ(json::value_t::object, data.type());
  EXPECT_EQ(expected.size(), data.size());

  { // iterate on values
    auto i = expected.begin();
    for (auto const &value : data) {
      ASSERT_NE(i, expected.end());

      EXPECT_EQ(json::value_t::number_unsigned, value.type());
      EXPECT_EQ(i->second, value);

      ++i;
    }
    EXPECT_EQ(i, expected.end());
  }

  { // iterate on key/value pairs
    auto i = expected.begin();
    for (auto const &item : data.items()) {
      ASSERT_NE(i, expected.end());

      EXPECT_EQ(i->first, item.key());

      auto const &value = item.value();
      EXPECT_EQ(json::value_t::number_unsigned, value.type());
      EXPECT_EQ(i->second, value);

      ++i;
    }
    EXPECT_EQ(i, expected.end());
  }
}
