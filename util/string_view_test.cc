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

#include <platform/types.h>
#include <util/string_view.h>

namespace views {

TEST(string_view, run_length)
{
  EXPECT_EQ(5u, run_length("ABAABCADBEF", "AB"));
  EXPECT_EQ(0u, run_length("ABAABCADBEF", "Z"));
  EXPECT_EQ(6u, run_length("ABAABA", "AB"));
}

TEST(string_view, trim_run)
{
  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAAB", trim_run(data, "AB"));
    EXPECT_EQ(data, "CADBEF");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("", trim_run(data, "Z"));
    EXPECT_EQ(data, "ABAABCADBEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("ABAABA", trim_run(data, "AB"));
    EXPECT_EQ(data, "");
  }
}

TEST(string_view, count_up_to_char)
{
  EXPECT_EQ(5u, count_up_to("ABAABCADBEF", 'C', false));
  EXPECT_EQ(0u, count_up_to("ABAABCADBEF", 'A', false));
  EXPECT_EQ(6u, count_up_to("ABAABA", 'Z', false));

  EXPECT_EQ(6u, count_up_to("ABAABCADBEF", 'C', true));
  EXPECT_EQ(1u, count_up_to("ABAABCADBEF", 'A', true));
  EXPECT_EQ(6u, count_up_to("ABAABA", 'Z', true));
}

TEST(string_view, count_up_to_string_view)
{
  EXPECT_EQ(5u, count_up_to("ABAABCADBEF", "CDEF", false));
  EXPECT_EQ(0u, count_up_to("ABAABCADBEF", "AB", false));
  EXPECT_EQ(6u, count_up_to("ABAABA", "Z", false));

  EXPECT_EQ(6u, count_up_to("ABAABCADBEF", "CDEF", true));
  EXPECT_EQ(1u, count_up_to("ABAABCADBEF", "AB", true));
  EXPECT_EQ(6u, count_up_to("ABAABA", "Z", true));
}

TEST(string_view, trim_up_to_char)
{
  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAAB", trim_up_to(data, 'C', SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "CADBEF");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("", trim_up_to(data, 'A', SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "ABAABCADBEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("ABAABA", trim_up_to(data, 'Z', SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAAB", trim_up_to(data, 'C', SeekBehavior::CONSUME));
    EXPECT_EQ(data, "ADBEF");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("", trim_up_to(data, 'A', SeekBehavior::CONSUME));
    EXPECT_EQ(data, "BAABCADBEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("ABAABA", trim_up_to(data, 'Z', SeekBehavior::CONSUME));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABC", trim_up_to(data, 'C', SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "ADBEF");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("A", trim_up_to(data, 'A', SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "BAABCADBEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("ABAABA", trim_up_to(data, 'Z', SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "");
  }
}

TEST(string_view, trim_up_to_string_view)
{
  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAAB", trim_up_to(data, "CDEF", SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "CADBEF");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("", trim_up_to(data, "AB", SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "ABAABCADBEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("ABAABA", trim_up_to(data, "Z", SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAAB", trim_up_to(data, "CDEF", SeekBehavior::CONSUME));
    EXPECT_EQ(data, "ADBEF");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("", trim_up_to(data, "AB", SeekBehavior::CONSUME));
    EXPECT_EQ(data, "BAABCADBEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("ABAABA", trim_up_to(data, "Z", SeekBehavior::CONSUME));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABC", trim_up_to(data, "CDEF", SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "ADBEF");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("A", trim_up_to(data, "AB", SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "BAABCADBEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("ABAABA", trim_up_to(data, "Z", SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "");
  }
}

TEST(string_view, count_up_to_last_char)
{
  EXPECT_EQ(10u, count_up_to_last("ABAABCADBEF", 'F', false));
  EXPECT_EQ(8u, count_up_to_last("ABAABCADBEF", 'B', false));
  EXPECT_EQ(0u, count_up_to_last("ABAABA", 'Z', false));

  EXPECT_EQ(11u, count_up_to_last("ABAABCADBEF", 'F', true));
  EXPECT_EQ(9u, count_up_to_last("ABAABCADBEF", 'B', true));
  EXPECT_EQ(0u, count_up_to_last("ABAABA", 'Z', true));
}

TEST(string_view, count_up_to_last_string_view)
{
  EXPECT_EQ(10u, count_up_to_last("ABAABCADBEF", "CDEF", false));
  EXPECT_EQ(8u, count_up_to_last("ABAABCADBEF", "AB", false));
  EXPECT_EQ(0u, count_up_to_last("ABAABA", "Z", false));

  EXPECT_EQ(11u, count_up_to_last("ABAABCADBEF", "CDEF", true));
  EXPECT_EQ(9u, count_up_to_last("ABAABCADBEF", "AB", true));
  EXPECT_EQ(0u, count_up_to_last("ABAABA", "Z", true));
}

TEST(string_view, trim_up_to_last_char)
{
  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADBE", trim_up_to_last(data, 'F', SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "F");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCAD", trim_up_to_last(data, 'B', SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "BEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("", trim_up_to_last(data, 'Z', SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "ABAABA");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADBE", trim_up_to_last(data, 'F', SeekBehavior::CONSUME));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCAD", trim_up_to_last(data, 'B', SeekBehavior::CONSUME));
    EXPECT_EQ(data, "EF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("", trim_up_to_last(data, 'Z', SeekBehavior::CONSUME));
    EXPECT_EQ(data, "ABAABA");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADBEF", trim_up_to_last(data, 'F', SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADB", trim_up_to_last(data, 'B', SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "EF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("", trim_up_to_last(data, 'Z', SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "ABAABA");
  }
}

TEST(string_view, trim_up_to_last_string_view)
{
  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADBE", trim_up_to_last(data, "CDEF", SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "F");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCAD", trim_up_to_last(data, "AB", SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "BEF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("", trim_up_to_last(data, "Z", SeekBehavior::EXCLUDE));
    EXPECT_EQ(data, "ABAABA");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADBE", trim_up_to_last(data, "CDEF", SeekBehavior::CONSUME));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCAD", trim_up_to_last(data, "AB", SeekBehavior::CONSUME));
    EXPECT_EQ(data, "EF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("", trim_up_to_last(data, "Z", SeekBehavior::CONSUME));
    EXPECT_EQ(data, "ABAABA");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADBEF", trim_up_to_last(data, "CDEF", SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "");
  }

  {
    std::string_view data = "ABAABCADBEF";
    EXPECT_EQ("ABAABCADB", trim_up_to_last(data, "AB", SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "EF");
  }

  {
    std::string_view data = "ABAABA";
    EXPECT_EQ("", trim_up_to_last(data, "Z", SeekBehavior::INCLUDE));
    EXPECT_EQ(data, "ABAABA");
  }
}

TEST(NumberView, integer_u8)
{
  EXPECT_EQ(0, NumberView<u8>("0").value());
  EXPECT_EQ(1, NumberView<u8>("1").value());
  EXPECT_EQ(10, NumberView<u8>("10").value());
  EXPECT_EQ(200, NumberView<u8>("200").value());
  EXPECT_EQ(255, NumberView<u8>("255").value());
}

TEST(NumberView, integer_s8)
{
  EXPECT_EQ(0, NumberView<s8>("0").value());
  EXPECT_EQ(1, NumberView<s8>("1").value());
  EXPECT_EQ(10, NumberView<s8>("10").value());
  EXPECT_EQ(-100, NumberView<s8>("-100").value());
  EXPECT_EQ(100, NumberView<s8>("100").value());
  EXPECT_EQ(100, NumberView<s8>("+100").value());
  EXPECT_EQ(127, NumberView<s8>("127").value());
  EXPECT_EQ(-128, NumberView<s8>("-128").value());
}

TEST(NumberView, integer_u16)
{
  EXPECT_EQ(0, NumberView<u16>("0").value());
  EXPECT_EQ(1, NumberView<u16>("1").value());
  EXPECT_EQ(10, NumberView<u16>("10").value());
  EXPECT_EQ(200, NumberView<u16>("200").value());
  EXPECT_EQ(255, NumberView<u16>("255").value());
  EXPECT_EQ(10000, NumberView<u16>("10000").value());
  EXPECT_EQ(1000, NumberView<u16>("+1000").value());
  EXPECT_EQ(31127, NumberView<u16>("31127").value());
  EXPECT_EQ(61128, NumberView<u16>("61128").value());
}

TEST(NumberView, integer_s16)
{
  EXPECT_EQ(0, NumberView<s16>("0").value());
  EXPECT_EQ(1, NumberView<s16>("1").value());
  EXPECT_EQ(10, NumberView<s16>("10").value());
  EXPECT_EQ(-100, NumberView<s16>("-100").value());
  EXPECT_EQ(100, NumberView<s16>("100").value());
  EXPECT_EQ(100, NumberView<s16>("+100").value());
  EXPECT_EQ(127, NumberView<s16>("127").value());
  EXPECT_EQ(-128, NumberView<s16>("-128").value());
  EXPECT_EQ(-1000, NumberView<s16>("-1000").value());
  EXPECT_EQ(10000, NumberView<s16>("10000").value());
  EXPECT_EQ(1000, NumberView<s16>("+1000").value());
  EXPECT_EQ(31127, NumberView<s16>("31127").value());
  EXPECT_EQ(-29128, NumberView<s16>("-29128").value());
}

TEST(NumberView, integer_u32)
{
  EXPECT_EQ(0u, NumberView<u32>("0").value());
  EXPECT_EQ(1u, NumberView<u32>("1").value());
  EXPECT_EQ(10u, NumberView<u32>("10").value());
  EXPECT_EQ(200u, NumberView<u32>("200").value());
  EXPECT_EQ(255u, NumberView<u32>("255").value());
  EXPECT_EQ(10000u, NumberView<u32>("10000").value());
  EXPECT_EQ(1000u, NumberView<u32>("+1000").value());
  EXPECT_EQ(31127u, NumberView<u32>("31127").value());
  EXPECT_EQ(61128u, NumberView<u32>("61128").value());
  EXPECT_EQ(+751000u, NumberView<u32>("+751000").value());
  EXPECT_EQ(81000u, NumberView<u32>("+81000").value());
  EXPECT_EQ(9931127u, NumberView<u32>("9931127").value());
  EXPECT_EQ(+19229128u, NumberView<u32>("+19229128").value());
}

TEST(NumberView, integer_s32)
{
  EXPECT_EQ(0, NumberView<s32>("0").value());
  EXPECT_EQ(1, NumberView<s32>("1").value());
  EXPECT_EQ(10, NumberView<s32>("10").value());
  EXPECT_EQ(-100, NumberView<s32>("-100").value());
  EXPECT_EQ(100, NumberView<s32>("100").value());
  EXPECT_EQ(100, NumberView<s32>("+100").value());
  EXPECT_EQ(127, NumberView<s32>("127").value());
  EXPECT_EQ(-128, NumberView<s32>("-128").value());
  EXPECT_EQ(-751000, NumberView<s32>("-751000").value());
  EXPECT_EQ(10000, NumberView<s32>("10000").value());
  EXPECT_EQ(81000, NumberView<s32>("+81000").value());
  EXPECT_EQ(9931127, NumberView<s32>("9931127").value());
  EXPECT_EQ(-19229128, NumberView<s32>("-19229128").value());
}

TEST(NumberView, integer_u64)
{
  EXPECT_EQ(0ul, NumberView<u64>("0").value());
  EXPECT_EQ(1ul, NumberView<u64>("1").value());
  EXPECT_EQ(10ul, NumberView<u64>("10").value());
  EXPECT_EQ(200ul, NumberView<u64>("200").value());
  EXPECT_EQ(255ul, NumberView<u64>("255").value());
  EXPECT_EQ(10000ul, NumberView<u64>("10000").value());
  EXPECT_EQ(1000ul, NumberView<u64>("+1000").value());
  EXPECT_EQ(31127ul, NumberView<u64>("31127").value());
  EXPECT_EQ(61128ul, NumberView<u64>("61128").value());
  EXPECT_EQ(+751000ul, NumberView<u64>("+751000").value());
  EXPECT_EQ(81000ul, NumberView<u64>("+81000").value());
  EXPECT_EQ(9931127ul, NumberView<u64>("9931127").value());
  EXPECT_EQ(+19229128ul, NumberView<u64>("+19229128").value());
}

TEST(NumberView, integer_s64)
{
  EXPECT_EQ(0l, NumberView<s64>("0").value());
  EXPECT_EQ(1l, NumberView<s64>("1").value());
  EXPECT_EQ(10l, NumberView<s64>("10").value());
  EXPECT_EQ(-100l, NumberView<s64>("-100").value());
  EXPECT_EQ(100l, NumberView<s64>("100").value());
  EXPECT_EQ(100l, NumberView<s64>("+100").value());
  EXPECT_EQ(127l, NumberView<s64>("127").value());
  EXPECT_EQ(-128l, NumberView<s64>("-128").value());
  EXPECT_EQ(-751000l, NumberView<s64>("-751000").value());
  EXPECT_EQ(10000l, NumberView<s64>("10000").value());
  EXPECT_EQ(81000l, NumberView<s64>("+81000").value());
  EXPECT_EQ(9931127l, NumberView<s64>("9931127").value());
  EXPECT_EQ(-19229128l, NumberView<s64>("-19229128").value());
}

enum class my_enum : u64 {};

TEST(NumberView, my_enum)
{
  EXPECT_EQ(static_cast<my_enum>(0ul), NumberView<my_enum>("0").value());
  EXPECT_EQ(static_cast<my_enum>(1ul), NumberView<my_enum>("1").value());
  EXPECT_EQ(static_cast<my_enum>(10ul), NumberView<my_enum>("10").value());
  EXPECT_EQ(static_cast<my_enum>(200ul), NumberView<my_enum>("200").value());
  EXPECT_EQ(static_cast<my_enum>(255ul), NumberView<my_enum>("255").value());
  EXPECT_EQ(static_cast<my_enum>(10000ul), NumberView<my_enum>("10000").value());
  EXPECT_EQ(static_cast<my_enum>(1000ul), NumberView<my_enum>("+1000").value());
  EXPECT_EQ(static_cast<my_enum>(31127ul), NumberView<my_enum>("31127").value());
  EXPECT_EQ(static_cast<my_enum>(61128ul), NumberView<my_enum>("61128").value());
  EXPECT_EQ(static_cast<my_enum>(+751000ul), NumberView<my_enum>("+751000").value());
  EXPECT_EQ(static_cast<my_enum>(81000ul), NumberView<my_enum>("+81000").value());
  EXPECT_EQ(static_cast<my_enum>(9931127ul), NumberView<my_enum>("9931127").value());
  EXPECT_EQ(static_cast<my_enum>(+19229128ul), NumberView<my_enum>("+19229128").value());
}

} // namespace views
