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

#include <platform/types.h>

#include <type_traits>

#include <cassert>
#include <climits>

namespace impl {

/**
 * Resolves to the underlying type of the enum, or to the type itself it it's
 * not an enum.
 *
 * Needed because `underlying_type_t` doesn't "just work" for non-enums.
 */
template <typename T, bool = std::is_enum_v<T>> struct enum_integer {
  using type = T;
};

template <typename T> struct enum_integer<T, true> {
  using type = std::underlying_type_t<T>;
};

template <typename T> using enum_integer_t = typename enum_integer<T>::type;

// implementation of `smallest_unsigned_integer`
template <std::size_t BitCount> constexpr auto smallest_unsigned_integer_impl()
{
  if constexpr (BitCount <= 1) {
    return bool{};
  } else if constexpr (BitCount <= (sizeof(u8) * CHAR_BIT)) {
    return u8{};
  } else if constexpr (BitCount <= (sizeof(u16) * CHAR_BIT)) {
    return u16{};
  } else if constexpr (BitCount <= (sizeof(u32) * CHAR_BIT)) {
    return u32{};
  } else if constexpr (BitCount <= (sizeof(u64) * CHAR_BIT)) {
    return u64{};
  } else if constexpr (BitCount <= (sizeof(u128) * CHAR_BIT)) {
    return u128{};
  } else {
    static_assert(BitCount <= (sizeof(u128) * CHAR_BIT), "bit count is too large for any known integer");
  }
}

} // namespace impl

/**
 * Resolves to the smallest unsigned integer type capable of holding `BitCount` bits.
 */
template <std::size_t BitCount> using smallest_unsigned_integer = decltype(impl::smallest_unsigned_integer_impl<BitCount>());

/**
 * Counts the number of `on` bits in the given value.
 *
 * Example:
 *
 *  count_bits_set(0b0000); // returns 0
 *  count_bits_set(0b0001); // returns 1
 *  count_bits_set(0b0010); // returns 1
 *  count_bits_set(0b1011); // returns 3
 *  count_bits_set(0b1100); // returns 2
 */
template <typename T> constexpr std::size_t count_bits_set(T value)
{
  static_assert(std::is_integral_v<T>);
  if constexpr (sizeof(T) <= (sizeof(unsigned int))) {
    return __builtin_popcount(static_cast<unsigned int>(static_cast<std::make_unsigned_t<T>>(value)));
  } else if constexpr (sizeof(T) <= (sizeof(unsigned long))) {
    return __builtin_popcountl(static_cast<unsigned long>(static_cast<std::make_unsigned_t<T>>(value)));
  } else {
    static_assert(sizeof(T) <= (sizeof(unsigned long long)));
    return __builtin_popcountll(static_cast<unsigned long long>(static_cast<std::make_unsigned_t<T>>(value)));
  }
}

/**
 * Counts the 0-based index of the least significant bit
 * Result is undefined if `value` is `0`
 *
 * Example:
 *
 *  least_significant_bit_index(0b0001); // returns 0
 *  least_significant_bit_index(0b0010); // returns 1
 *  least_significant_bit_index(0b1011); // returns 0
 *  least_significant_bit_index(0b1000); // returns 3
 */
template <typename T> constexpr std::size_t least_significant_bit_index(T value)
{
  static_assert(std::is_integral_v<T>);
  if constexpr (sizeof(T) <= (sizeof(unsigned int))) {
    return __builtin_ctz(static_cast<unsigned int>(static_cast<std::make_unsigned_t<T>>(value)));
  } else if constexpr (sizeof(T) <= (sizeof(unsigned long))) {
    return __builtin_ctzl(static_cast<unsigned long>(static_cast<std::make_unsigned_t<T>>(value)));
  } else {
    static_assert(sizeof(T) <= (sizeof(unsigned long long)));
    return __builtin_ctzll(static_cast<unsigned long long>(static_cast<std::make_unsigned_t<T>>(value)));
  }
}

/**
 * Returns the given value with its least significant bit set to `off`.
 *
 * Takes care of proper type conversion, enum class support and avoids problems
 * with implicit type promotions.
 *
 * Example:
 *
 *  disable_least_significant_bit(0b0000); // returns 0b0000
 *  disable_least_significant_bit(0b0001); // returns 0b0000
 *  disable_least_significant_bit(0b0010); // returns 0b0000
 *  disable_least_significant_bit(0b1011); // returns 0b1010
 *  disable_least_significant_bit(0b1100); // returns 0b1000
 */
template <typename T> constexpr T disable_least_significant_bit(T value)
{
  using type = impl::enum_integer_t<T>;
  return static_cast<T>(static_cast<type>(value) & (static_cast<type>(value) - type{1}));
}

/**
 * Returns the least significant bit only, in its original position.
 *
 * Takes care of proper type conversion, enum class support and avoids problems
 * with implicit type promotions.
 *
 * Example:
 *
 *  least_significant_bit(0b0000); // returns 0b0000
 *  least_significant_bit(0b0001); // returns 0b0001
 *  least_significant_bit(0b0010); // returns 0b0010
 *  least_significant_bit(0b1011); // returns 0b0001
 *  least_significant_bit(0b1100); // returns 0b0100
 */
template <typename T> constexpr T least_significant_bit(T value)
{
  using type = impl::enum_integer_t<T>;
  return static_cast<T>((static_cast<type>(value) ^ (static_cast<type>(value) - type{1})) & static_cast<type>(value));
}

/**
 * Returns a value of type `T` with the bit at 0-based position `index` set to
 * `on` and all other bits `off`.
 *
 * Takes care of proper type conversion, enum class support and avoids problems
 * with implicit type promotions.
 *
 * Example:
 *
 *  make_bit<int>(0); // returns 0b0001
 *  make_bit<int>(1); // returns 0b0010
 *  make_bit<int>(2); // returns 0b0100
 *  make_bit<int>(3); // returns 0b1000
 */
template <typename T> constexpr T make_bit(std::size_t bit_index)
{
  using type = impl::enum_integer_t<T>;
  // ensure that the given bit fits in the result type
  assert(bit_index < (sizeof(T) * CHAR_BIT));
  return static_cast<T>(type{1} << static_cast<type>(bit_index));
}

/**
 * Returns the result of `value << shift`, taking care of implicit type
 * promotions which could go unnoticed.
 */
template <typename T, typename LHS, typename RHS> constexpr T shift_left(LHS value, RHS shift)
{
  return static_cast<T>(static_cast<T>(value) << static_cast<T>(shift));
}

/**
 * Returns the result of `value >> shift`, taking care of implicit type
 * promotions which could go unnoticed.
 */
template <typename T, typename LHS, typename RHS> constexpr T shift_right(LHS value, RHS shift)
{
  return static_cast<T>(static_cast<T>(value) >> static_cast<T>(shift));
}
