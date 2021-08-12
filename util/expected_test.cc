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

#include <util/expected.h>

#include <gtest/gtest.h>

#include <unordered_map>
#include <vector>

struct CustomError : public std::exception {
  CustomError() = default;

  template <typename... Args>
  explicit CustomError(std::string_view message, Args &&... args) : message_(message), code_((0 + ... + args))
  {}

  CustomError(CustomError const &) = default;
  CustomError(CustomError &&) = default;

  bool operator!=(CustomError const &rhs) const { return !(*this == rhs); }
  bool operator==(CustomError const &rhs) const { return message_ == rhs.message_ && code_ == rhs.code_; }

  auto const &message() const { return message_; }

private:
  std::string_view const message_;
  int const code_ = 0;
};

TEST(expected, success_example)
{
  // simulates a function that returns a success value through `expected`
  auto computation = []() -> Expected<int, CustomError> { return 10; };

  EXPECT_NO_THROW({
    auto result = computation();

    if (!result) {
      EXPECT_TRUE(false); // unreachable
    } else {
      EXPECT_EQ(10, result.value());
    }

    result.try_raise();
  });
}

TEST(expected, success_with_arguments_example)
{
  // simulates a function that returns a success value constructed with
  // arguments through `expected`
  auto computation = []() -> Expected<std::string, CustomError> { return {"test string"}; };

  EXPECT_NO_THROW({
    auto result = computation();

    if (!result) {
      EXPECT_TRUE(false); // unreachable
    } else {
      EXPECT_EQ("test string", result.value());
    }

    result.try_raise();
  });
}

TEST(expected, success_with_multiple_arguments_example)
{
  // simulates a function that returns a success value constructed with multiple
  // arguments through `expected`
  auto computation = []() -> Expected<std::pair<std::string, int>, CustomError> { return {"test string", 100}; };

  EXPECT_NO_THROW({
    auto result = computation();

    if (!result) {
      EXPECT_TRUE(false); // unreachable
    } else {
      EXPECT_EQ("test string", result.value().first);
      EXPECT_EQ(100, result.value().second);
    }

    result.try_raise();
  });
}

TEST(expected, failure_code_example)
{
  // simulates a function that returns an error code through `expected`
  auto computation = []() -> Expected<int, long> { return {unexpected, -1}; };

  EXPECT_NO_THROW({
    auto result = computation();

    if (!result) {
      EXPECT_EQ(-1, result.error());
    } else {
      EXPECT_TRUE(false); // unreachable
    }

    // we can't call `result.raise()` nor `result.try_raise()` because that
    // would fail to compile - this is intended because the default policy for
    // non-exception error types is to not allow throwing
  });
}

TEST(expected, default_constructed_failure_example)
{
  // simulates a function that returns a default constructed error through
  // `expected`
  auto computation = []() -> Expected<int, CustomError> { return unexpected; };

  EXPECT_THROW(
      {
        auto result = computation();

        if (!result) {
          EXPECT_EQ(CustomError(), result.error());

          result.raise();
        } else {
          EXPECT_TRUE(false); // unreachable
        }
      },
      CustomError);

  EXPECT_THROW(
      {
        auto result = computation();

        result.try_raise();

        EXPECT_TRUE(false); // unreachable
      },
      CustomError);
}

TEST(expected, default_constructed_success_example)
{
  // simulates a function that returns a default constructed value through
  // `expected`
  auto computation = []() -> Expected<int, CustomError> { return {}; };

  EXPECT_NO_THROW({
    auto result = computation();

    if (!result) {
      EXPECT_TRUE(false); // unreachable
    } else {
      EXPECT_EQ(int{}, result.value());
    }

    result.try_raise();
  });
}

TEST(expected, failure_with_arguments_example)
{
  // simulates a function that returns an error constructed with arguments
  // through `expected`
  auto computation = []() -> Expected<int, CustomError> { return {unexpected, "error message"}; };

  EXPECT_THROW(
      {
        auto result = computation();

        if (!result) {
          EXPECT_EQ(CustomError("error message"), result.error());

          result.raise();
        } else {
          EXPECT_TRUE(false); // unreachable
        }
      },
      CustomError);

  EXPECT_THROW(
      {
        auto result = computation();

        result.try_raise();

        EXPECT_TRUE(false); // unreachable
      },
      CustomError);
}

TEST(expected, failure_with_multiple_arguments_example)
{
  // simulates a function that returns an error constructed with multiple
  // arguments through `expected`
  auto computation = []() -> Expected<int, CustomError> { return {unexpected, "error message", 1, 2, 3}; };

  EXPECT_THROW(
      {
        auto result = computation();

        if (!result) {
          EXPECT_EQ(CustomError("error message", 1, 2, 3), result.error());

          result.raise();
        } else {
          EXPECT_TRUE(false); // unreachable
        }
      },
      CustomError);

  EXPECT_THROW(
      {
        auto result = computation();

        result.try_raise();

        EXPECT_TRUE(false); // unreachable
      },
      CustomError);
}

TEST(expected, default_ctor_int__int)
{
  Expected<int, int> e;

  EXPECT_TRUE(e.has_value());
  EXPECT_NE(nullptr, e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, value_int__int)
{
  Expected<int, int> e(10);

  EXPECT_TRUE(e.has_value());

  EXPECT_EQ(10, e.value());

  ASSERT_NE(nullptr, e.try_value());
  EXPECT_EQ(10, *e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, error_int__int)
{
  Expected<int, int> e(unexpected, 5);

  EXPECT_FALSE(e.has_value());

  ASSERT_EQ(nullptr, e.try_value());

  EXPECT_NE(nullptr, e.try_error());
  EXPECT_EQ(5, *e.try_error());
}

TEST(expected, default_ctor_int__custom_error)
{
  Expected<int, CustomError> e;

  EXPECT_TRUE(e.has_value());
  EXPECT_NE(nullptr, e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, value_int__custom_error)
{
  Expected<int, CustomError> e(10);

  EXPECT_TRUE(e.has_value());

  EXPECT_EQ(10, e.value());

  ASSERT_NE(nullptr, e.try_value());
  EXPECT_EQ(10, *e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, error_int__custom_error)
{
  Expected<int, CustomError> e(unexpected, "message");

  EXPECT_FALSE(e.has_value());

  ASSERT_EQ(nullptr, e.try_value());

  EXPECT_NE(nullptr, e.try_error());
  EXPECT_EQ(CustomError("message"), *e.try_error());

  EXPECT_THROW({ e.raise(); }, CustomError);
}

TEST(expected, default_ctor_custom_error__int)
{
  Expected<CustomError, int> e;

  EXPECT_TRUE(e.has_value());
  EXPECT_NE(nullptr, e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, value_custom_error__int)
{
  Expected<CustomError, int> e("result");

  EXPECT_TRUE(e.has_value());

  EXPECT_EQ(CustomError("result"), e.value());

  ASSERT_NE(nullptr, e.try_value());
  EXPECT_EQ(CustomError("result"), *e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, error_custom_error__int)
{
  Expected<CustomError, int> e(unexpected, 5);

  EXPECT_FALSE(e.has_value());

  ASSERT_EQ(nullptr, e.try_value());

  EXPECT_NE(nullptr, e.try_error());
  EXPECT_EQ(5, *e.try_error());
}

TEST(expected, default_ctor_custom_error__custom_error)
{
  Expected<CustomError, CustomError> e;

  EXPECT_TRUE(e.has_value());
  EXPECT_NE(nullptr, e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, value_custom_error__custom_error)
{
  Expected<CustomError, CustomError> e("result");

  EXPECT_TRUE(e.has_value());

  EXPECT_EQ(CustomError("result"), e.value());

  ASSERT_NE(nullptr, e.try_value());
  EXPECT_EQ(CustomError("result"), *e.try_value());

  EXPECT_EQ(nullptr, e.try_error());
}

TEST(expected, error_custom_error__custom_error)
{
  Expected<CustomError, CustomError> e(unexpected, "message");

  EXPECT_FALSE(e.has_value());

  ASSERT_EQ(nullptr, e.try_value());

  EXPECT_NE(nullptr, e.try_error());
  EXPECT_EQ(CustomError("message"), *e.try_error());

  EXPECT_THROW({ e.raise(); }, CustomError);
}

TEST(expected, on_error_with_value)
{
  auto computation = []() -> Expected<int, CustomError> { return 10; };

  EXPECT_NO_THROW({ computation().on_error([](auto &error) { throw CustomError(); }); });
}

TEST(expected, on_error_with_error__no_arguments)
{
  auto computation = []() -> Expected<int, CustomError> { return {unexpected, "error message"}; };

  EXPECT_THROW({ computation().on_error([](auto &error) { throw CustomError(); }); }, CustomError);
}

TEST(expected, on_error_with_error__error_argument)
{
  auto computation = []() -> Expected<int, CustomError> { return {unexpected, "error message"}; };

  EXPECT_THROW({ computation().on_error([](CustomError const &e) { throw e; }); }, CustomError);
}

TEST(expected, recover_with_value_const)
{
  Expected<int, CustomError> const e{10};

  EXPECT_EQ(10, e.recover([](auto &error) { return 5; }));
}

TEST(expected, recover_with_value)
{
  Expected<int, CustomError> e{10};

  EXPECT_EQ(10, e.recover([](auto &error) { return 5; }));
}

TEST(expected, recover_with_error_const)
{
  Expected<int, CustomError> const e{unexpected, "error message"};

  EXPECT_EQ(5, e.recover([](auto &error) { return 5; }));
}

TEST(expected, recover_with_error)
{
  Expected<int, CustomError> e{unexpected, "error message"};

  EXPECT_EQ(5, e.recover([](auto &error) { return 5; }));
}

TEST(expected, recover_with_with_value_const)
{
  Expected<int, CustomError> const e{10};

  EXPECT_EQ(10, e.recover_with(5));
}

TEST(expected, recover_with_with_value)
{
  Expected<int, CustomError> e{10};

  EXPECT_EQ(10, e.recover_with(5));
}

TEST(expected, recover_with_with_error_const)
{
  Expected<int, CustomError> const e{unexpected, "error message"};

  EXPECT_EQ(5, e.recover_with(5));
}

TEST(expected, recover_with_with_error)
{
  Expected<int, CustomError> e{unexpected, "error message"};

  EXPECT_EQ(5, e.recover_with(5));
}

TEST(expected, try_raise_on_success)
{
  Expected<int, CustomError> e{10};

  EXPECT_EQ(10, e.try_raise().value());
}

TEST(expected, try_raise_on_error)
{
  Expected<int, CustomError> e{unexpected, "error message"};

  EXPECT_THROW({ EXPECT_EQ(10, e.try_raise().value()); }, CustomError);
}

TEST(expected, value_CheckedExpected)
{
  CheckedExpected<int, int> e;

  EXPECT_NO_THROW({ e.value(); });
  EXPECT_THROW({ e.error(); }, std::logic_error);
}

TEST(expected, error_CheckedExpected)
{
  CheckedExpected<int, int> e(unexpected);

  EXPECT_THROW({ e.value(); }, std::logic_error);
  EXPECT_NO_THROW({ e.error(); });
}
