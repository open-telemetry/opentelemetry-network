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

#pragma once

#include <util/string.h>

#include <string_view>
#include <type_traits>

namespace views {

constexpr std::string_view EOL = "\n\r";
constexpr std::string_view WHITESPACE = " \n\t\r";
constexpr std::string_view NON_EOL_WHITESPACE = " \t";

/**
 * Behavior for functions that perform string lookups with delimiters.
 */
enum class SeekBehavior : std::uint8_t {
  /**
   * Exclude delimiter/needle from result and leave it in the haystack.
   */
  EXCLUDE = 0b00,

  /**
   * Exclude delimiter/needle from result but consume it from the haystack.
   */
  CONSUME = 0b10,

  /**
   * Include delimiter/needle in result and remove it from the haystack.
   */
  INCLUDE = 0b11
};

/**
 * Count how many characters are in the beginning of `input` up until the first
 * occurence of a character not in `what`.
 *
 * Example:
 *
 *  // returns 5
 *  run_length("ABAABCADBEF", "AB");
 *
 *  // returns 0
 *  run_length("ABAABCADBEF", "Z");
 *
 *  // returns 6
 *  run_length("ABAABA", "AB");
 */
std::size_t run_length(std::string_view input, std::string_view what);

/**
 * Trims characters from the beginning of `input` up until the first occurence
 * of a character not in `what`. Returns a string view of the characters
 * trimmed.
 *
 * Example:
 *
 *  // returns "ABAAB", changes `input` to "CADBEF"
 *  trim_run("ABAABCADBEF", "AB");
 *
 *  // returns "", leaves `input` unchanged
 *  trim_run("ABAABCADBEF", "Z");
 *
 *  // returns "ABAABA", changes `input` to ""
 *  trim_run("ABAABA", "AB");
 */
std::string_view trim_run(std::string_view &input, std::string_view what);

/**
 * Count how many characters are in the beginning of `input` up until the first
 * occurence of a character in `what`.
 *
 * `include` tells whether the occurence should be included in the counting
 * or not.
 *
 * Example:
 *
 *  // returns 5
 *  count_up_to("ABAABCADBEF", "CDEF", false);
 *
 *  // returns 0
 *  count_up_to("ABAABCADBEF", "AB", false);
 *
 *  // returns 6
 *  count_up_to("ABAABA", "Z", false);
 */
std::size_t count_up_to(std::string_view input, char what, bool include);
std::size_t count_up_to(std::string_view input, std::string_view what, bool include);

/**
 * Trims characters from the beginning of `input` up until the first occurence
 * of a character in `what`. Returns a string view of the characters trimmed.
 *
 * `behavior` tells whether the occurence should be included in the trimming
 * or not.
 *
 * `what` can be either a character or a string / string view.
 *
 * Example:
 *
 *  // returns "ABAAB", changes `input` to "CADBEF"
 *  trim_up_to("ABAABCADBEF", "CDEF", SeekBehavior::EXCLUDE);
 *
 *  // returns "", leaves `input` unchanged
 *  trim_up_to("ABAABCADBEF", "AB", SeekBehavior::EXCLUDE);
 *
 *  // returns "ABAABA", changes `input` to ""
 *  trim_up_to("ABAABA", "Z", SeekBehavior::EXCLUDE);
 */
std::string_view trim_up_to(std::string_view &input, char what, SeekBehavior behavior);
std::string_view trim_up_to(std::string_view &input, std::string_view what, SeekBehavior behavior);

/**
 * Count how many characters are in the beginning of `input` up until the last
 * occurence of a character in `what`.
 *
 * `include` tells whether the occurence should be included in the counting
 * or not.
 *
 * Returns 0 if no occurences are found.
 *
 * Example:
 *
 *  // returns 11
 *  count_up_to_last("ABAABCADBEF", "CDEF", true);
 *
 *  // returns 9
 *  count_up_to_last("ABAABCADBEF", "AB", true);
 *
 *  // returns 0
 *  count_up_to_last("ABAABA", "Z", true);
 */
std::size_t count_up_to_last(std::string_view input, char what, bool include);
std::size_t count_up_to_last(std::string_view input, std::string_view what, bool include);

/**
 * Trims characters from the beginning of `input` up until the last occurence
 * of a character in `what`. Returns a string view of the characters trimmed.
 *
 * `behavior` tells whether the occurence should be included in the trimming
 * or not.
 *
 * Trims nothing if no occurences are found.
 *
 * `what` can be either a character or a string / string view.
 *
 * Example:
 *
 *  // returns "ABAABCADBEF", changes `input` to ""
 *  trim_up_to_last("ABAABCADBEF", "CDEF", SeekBehavior::EXCLUDE);
 *
 *  // returns "ABAABCADB", changes `input` to "EF"
 *  trim_up_to_last("ABAABCADBEF", "AB", SeekBehavior::EXCLUDE);
 *
 *  // returns "", leaves `input` unchanged
 *  trim_up_to_last("ABAABA", "Z", SeekBehavior::EXCLUDE);
 */
std::string_view trim_up_to_last(std::string_view &input, char what, SeekBehavior behavior);
std::string_view trim_up_to_last(std::string_view &input, std::string_view what, SeekBehavior behavior);

/**
 * Returns the last `size` characters from the string view.
 *
 * If the input is smaller than size, returns the full input.
 */
std::string_view slice_suffix(std::string_view view, std::size_t size);

/**
 * Returns whether the view ends with the specified suffix.
 */
bool ends_with(std::string_view view, std::string_view suffix);

/**
 * A helper to lazily convert a string view to a number.
 *
 * `T` must be either an integer or a floating point type.
 *
 * Note: due to the semantics of the underlying parsing functions being used
 * from the standard library, the string must either be null terminated or the
 * number being parsed must be followed by an invalid character (e.g.: a blank
 * space).
 *
 * Example:
 *
 *  std::string_view str = some_fn();
 *
 *  NumberView<int> view{str};
 *
 *  // parses the string view, return 0 on error
 *  int value = view.value(0);
 *
 *  std::string_view str2 = some_other_fn();
 *  view = str2;
 *
 *  // parses the other string view, return 5 on error
 *  int value2 = view.value(5);
 */
template <typename T> struct NumberView {
  constexpr NumberView() = default;

  constexpr explicit NumberView(std::string_view view) : view_(view) {}

  static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);

  /**
   * Parses the string view and returns the result, or `fallback` on error.
   *
   * NOTE: the underlying string view must be padded with '\0' or another
   * non-numerical character due to limitations in the standard library's
   * number parsing routines.
   */
  constexpr T value(T fallback = {}) const;

  /**
   * Parses the string view and returns the result, or `fallback` on error.
   *
   * This is a slightly less performant alternative to `value()`, but it is
   * guaranteed to work on unpadded string views.
   */
  constexpr T unpadded_value(T fallback = {}) const;

  NumberView &operator=(std::string_view view)
  {
    view_ = view;
    return *this;
  }

  constexpr std::string_view view() const { return view_; }

  bool empty() const { return view_.empty(); }

  explicit constexpr operator T() const { return unpadded_value(); }

  constexpr bool operator==(T rhs) const { return static_cast<T>(*this) == rhs; }
  constexpr bool operator!=(T rhs) const { return static_cast<T>(*this) != rhs; }
  constexpr bool operator<(T rhs) const { return static_cast<T>(*this) < rhs; }
  constexpr bool operator<=(T rhs) const { return static_cast<T>(*this) <= rhs; }
  constexpr bool operator>(T rhs) const { return static_cast<T>(*this) > rhs; }
  constexpr bool operator>=(T rhs) const { return static_cast<T>(*this) >= rhs; }

  friend constexpr bool operator==(T lhs, NumberView rhs) { return lhs == static_cast<T>(rhs); }
  friend constexpr bool operator!=(T lhs, NumberView rhs) { return lhs != static_cast<T>(rhs); }
  friend constexpr bool operator<(T lhs, NumberView rhs) { return lhs < static_cast<T>(rhs); }
  friend constexpr bool operator<=(T lhs, NumberView rhs) { return lhs <= static_cast<T>(rhs); }
  friend constexpr bool operator>(T lhs, NumberView rhs) { return lhs > static_cast<T>(rhs); }
  friend constexpr bool operator>=(T lhs, NumberView rhs) { return lhs >= static_cast<T>(rhs); }

  constexpr explicit operator bool() const { return !empty(); }
  constexpr bool operator!() const { return empty(); }

private:
  std::string_view view_;
};

} // namespace views

#include <util/string_view.inl>
