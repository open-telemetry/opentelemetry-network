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

#include <util/log_modifiers.h>

#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

TEST(log_modifiers, logging_overhead)
{
  EXPECT_FALSE(logger::is_logging_overhead<bool &>);
  EXPECT_FALSE(logger::is_logging_overhead<bool &&>);
  EXPECT_FALSE(logger::is_logging_overhead<bool const &>);
  EXPECT_FALSE(logger::is_logging_overhead<bool const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<char &>);
  EXPECT_FALSE(logger::is_logging_overhead<char &&>);
  EXPECT_FALSE(logger::is_logging_overhead<char const &>);
  EXPECT_FALSE(logger::is_logging_overhead<char const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<char const *&>);
  EXPECT_FALSE(logger::is_logging_overhead<char const *&&>);
  EXPECT_FALSE(logger::is_logging_overhead<char const *const &>);
  EXPECT_FALSE(logger::is_logging_overhead<char const *const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<int &>);
  EXPECT_FALSE(logger::is_logging_overhead<int &&>);
  EXPECT_FALSE(logger::is_logging_overhead<int const &>);
  EXPECT_FALSE(logger::is_logging_overhead<int const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<long &>);
  EXPECT_FALSE(logger::is_logging_overhead<long &&>);
  EXPECT_FALSE(logger::is_logging_overhead<long const &>);
  EXPECT_FALSE(logger::is_logging_overhead<long const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<float &>);
  EXPECT_FALSE(logger::is_logging_overhead<float &&>);
  EXPECT_FALSE(logger::is_logging_overhead<float const &>);
  EXPECT_FALSE(logger::is_logging_overhead<float const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<double &>);
  EXPECT_FALSE(logger::is_logging_overhead<double &&>);
  EXPECT_FALSE(logger::is_logging_overhead<double const &>);
  EXPECT_FALSE(logger::is_logging_overhead<double const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<std::string &>);
  EXPECT_TRUE(logger::is_logging_overhead<std::string &&>);
  EXPECT_FALSE(logger::is_logging_overhead<std::string const &>);
  EXPECT_TRUE(logger::is_logging_overhead<std::string const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<std::string_view &>);
  EXPECT_FALSE(logger::is_logging_overhead<std::string_view &&>);
  EXPECT_FALSE(logger::is_logging_overhead<std::string_view const &>);
  EXPECT_FALSE(logger::is_logging_overhead<std::string_view const &&>);

  EXPECT_FALSE(logger::is_logging_overhead<std::vector<int> &>);
  EXPECT_FALSE(logger::is_logging_overhead<std::vector<int> &&>);
  EXPECT_FALSE(logger::is_logging_overhead<std::vector<int> const &>);
  EXPECT_FALSE(logger::is_logging_overhead<std::vector<int> const &&>);
}

#define LOG_STR(...) fmt::format("{}", __VA_ARGS__)

TEST(log_modifiers, log_waive)
{
  EXPECT_EQ("10", LOG_STR(log_waive(10)));
  EXPECT_EQ("10", LOG_STR(log_waive(std::string("10"))));

  std::string s("10");
  EXPECT_EQ("10", LOG_STR(log_waive(s)));

  std::string &ref = s;
  EXPECT_EQ("10", LOG_STR(log_waive(ref)));

  std::string const &cref = s;
  EXPECT_EQ("10", LOG_STR(log_waive(cref)));

  EXPECT_EQ("10", LOG_STR(log_waive(std::move(s))));

  EXPECT_EQ("10", LOG_STR(log_waive([] { return std::string("10"); }())));
}

TEST(log_modifiers, log_either)
{
  EXPECT_EQ("111", LOG_STR(log_either(true, 111, 222)));
  EXPECT_EQ("222", LOG_STR(log_either(false, 111, 222)));

  EXPECT_EQ("TTT", LOG_STR(log_either(true, "TTT", "FFF")));
  EXPECT_EQ("FFF", LOG_STR(log_either(false, "TTT", "FFF")));

  EXPECT_EQ("111", LOG_STR(log_either(true, 111, "FFF")));
  EXPECT_EQ("222", LOG_STR(log_either(false, "TTT", 222)));
}

TEST(log_modifiers, log_surround)
{
  EXPECT_EQ("<10>", LOG_STR(log_surround<'<', '>'>(10)));

  std::string s("10");
  EXPECT_EQ("<10>", LOG_STR(log_surround<'<', '>'>(s)));

  std::string &ref = s;
  EXPECT_EQ("<10>", LOG_STR(log_surround<'<', '>'>(ref)));

  std::string const &cref = s;
  EXPECT_EQ("<10>", LOG_STR(log_surround<'<', '>'>(cref)));
}

TEST(log_modifiers, log_quoted)
{
  EXPECT_EQ("\"10\"", LOG_STR(log_quoted(10)));

  std::string s("10");
  EXPECT_EQ("\"10\"", LOG_STR(log_quoted(s)));

  std::string &ref = s;
  EXPECT_EQ("\"10\"", LOG_STR(log_quoted(ref)));

  std::string const &cref = s;
  EXPECT_EQ("\"10\"", LOG_STR(log_quoted(cref)));
}

TEST(log_modifiers, log_single_quoted)
{
  EXPECT_EQ("'10'", LOG_STR(log_single_quoted(10)));

  std::string s("10");
  EXPECT_EQ("'10'", LOG_STR(log_single_quoted(s)));

  std::string &ref = s;
  EXPECT_EQ("'10'", LOG_STR(log_single_quoted(ref)));

  std::string const &cref = s;
  EXPECT_EQ("'10'", LOG_STR(log_single_quoted(cref)));
}

TEST(log_modifiers, log_parens)
{
  EXPECT_EQ("(10)", LOG_STR(log_parens(10)));

  std::string s("10");
  EXPECT_EQ("(10)", LOG_STR(log_parens(s)));

  std::string &ref = s;
  EXPECT_EQ("(10)", LOG_STR(log_parens(ref)));

  std::string const &cref = s;
  EXPECT_EQ("(10)", LOG_STR(log_parens(cref)));
}

TEST(log_modifiers, log_bracket)
{
  EXPECT_EQ("[10]", LOG_STR(log_bracket(10)));

  std::string s("10");
  EXPECT_EQ("[10]", LOG_STR(log_bracket(s)));

  std::string &ref = s;
  EXPECT_EQ("[10]", LOG_STR(log_bracket(ref)));

  std::string const &cref = s;
  EXPECT_EQ("[10]", LOG_STR(log_bracket(cref)));
}

TEST(log_modifiers, log_brace)
{
  EXPECT_EQ("{10}", LOG_STR(log_brace(10)));

  std::string s("10");
  EXPECT_EQ("{10}", LOG_STR(log_brace(s)));

  std::string &ref = s;
  EXPECT_EQ("{10}", LOG_STR(log_brace(ref)));

  std::string const &cref = s;
  EXPECT_EQ("{10}", LOG_STR(log_brace(cref)));
}

TEST(log_modifiers, log_kv_pair)
{
  EXPECT_EQ(R"(key="value")", LOG_STR(log_kv_pair("key", "value")));
  EXPECT_EQ(R"(key="10")", LOG_STR(log_kv_pair("key", 10)));
  EXPECT_EQ(R"(v0|"v1")", LOG_STR(log_kv_pair<'|'>("v0", "v1")));
  EXPECT_EQ(R"(10+(20))", LOG_STR(log_kv_pair<'+', '(', ')'>(10, 20)));

  {
    std::string_view s("10");
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(s, s)));

    std::string_view &ref = s;
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(ref, ref)));

    std::string_view const &cref = s;
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(cref, cref)));

    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(s, ref)));
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(s, cref)));
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(ref, cref)));
  }

  {
    std::string s("10");
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(s, s)));

    std::string &ref = s;
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(ref, ref)));

    std::string const &cref = s;
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(cref, cref)));

    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(s, ref)));
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(s, cref)));
    EXPECT_EQ(R"(10="10")", LOG_STR(log_kv_pair(ref, cref)));
  }
}

TEST(log_modifiers, log_call)
{
  int when_true_count = 0;
  std::string const when_true_value = "TTT";
  auto const when_true = [&] {
    ++when_true_count;
    return when_true_value;
  };

  int when_false_count = 0;
  std::string const when_false_value = "FFF";
  [[maybe_unused]] auto const when_false = [&] {
    ++when_false_count;
    return when_false_value;
  };

  EXPECT_EQ("TTT", LOG_STR(log_call(when_true)));
  EXPECT_EQ(1, when_true_count);
  EXPECT_EQ(0, when_false_count);

  EXPECT_EQ("FFF", LOG_STR(log_call(when_false)));
  EXPECT_EQ(1, when_true_count);
  EXPECT_EQ(1, when_false_count);

  EXPECT_EQ("TTT", LOG_STR(log_either(true, log_call(when_true), log_call(when_false))));
  EXPECT_EQ(2, when_true_count);
  EXPECT_EQ(1, when_false_count);

  EXPECT_EQ("FFF", LOG_STR(log_either(false, log_call(when_true), log_call(when_false))));
  EXPECT_EQ(2, when_true_count);
  EXPECT_EQ(2, when_false_count);
}

TEST(log_modifiers, log_lazy)
{
  int when_true_count = 0;
  std::string_view const when_true_value = "TTT";
  auto const when_true = [&] {
    ++when_true_count;
    return when_true_value;
  };

  int when_false_count = 0;
  std::string_view const when_false_value = "FFF";
  [[maybe_unused]] auto const when_false = [&] {
    ++when_false_count;
    return when_false_value;
  };

  EXPECT_EQ("TTT", LOG_STR(LOG_LAZY(when_true())));
  EXPECT_EQ(1, when_true_count);
  EXPECT_EQ(0, when_false_count);

  EXPECT_EQ("FFF", LOG_STR(LOG_LAZY(when_false())));
  EXPECT_EQ(1, when_true_count);
  EXPECT_EQ(1, when_false_count);

  EXPECT_EQ("TTT", LOG_STR(log_either(true, LOG_LAZY(when_true()), LOG_LAZY(when_false()))));
  EXPECT_EQ(2, when_true_count);
  EXPECT_EQ(1, when_false_count);

  EXPECT_EQ("FFF", LOG_STR(log_either(false, LOG_LAZY(when_true()), LOG_LAZY(when_false()))));
  EXPECT_EQ(2, when_true_count);
  EXPECT_EQ(2, when_false_count);
}

} // namespace
