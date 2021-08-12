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

#include <util/bits.h>
#include <util/enum.h>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <list>
#include <string>
#include <string_view>
#include <type_traits>

#include <cstdint>

template <typename Enum> struct enum_traits;

template <typename Which>
void parse_enum_list(std::list<std::string> &name_list, void (*callback_list)(std::list<Which> enum_list));

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>> std::underlying_type_t<T> integer_value(T value)
{
  return static_cast<std::underlying_type_t<T>>(value);
}

// unfortunately `std::is_sorted` isn't `constexpr` on C++17
template <typename Iterator> constexpr bool is_sorted(Iterator begin, Iterator end)
{
  if (begin == end) {
    return true;
  }
  for (auto i = begin + 1; i != end; ++i, ++begin) {
    if (*i < *begin) {
      return false;
    }
  }
  return true;
}

/**
 * Returns a 0-based index for the given enum value.
 *
 * Returns `enum_traits<T>::count` if not found.
 */
template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>> constexpr std::size_t enum_index_of(T value)
{
  using traits = enum_traits<T>;

  if constexpr (traits::is_contiguous) {
    return static_cast<std::size_t>(static_cast<std::intmax_t>(value) - static_cast<std::intmax_t>(traits::min()));
  } else {
    static_assert(is_sorted(traits::values.begin(), traits::values.end()), "enum values were not declared in sorted order");
    auto const i = std::lower_bound(traits::values.begin(), traits::values.end(), value);

    return i != traits::values.end() && *i == value ? std::distance(traits::values.begin(), i) : traits::count;
  }
}

/**
 * A space and time efficient set for storing rich enumerations.
 */
template <typename Enum> class EnumSet {
  static_assert(std::is_enum_v<Enum>);
  using traits = enum_traits<Enum>;
  using int_type = smallest_unsigned_integer<traits::count>;

  struct const_iterator {
    constexpr const_iterator(int_type set = int_type{0}) : set_(set) {}

    constexpr bool operator==(const_iterator const &rhs) const { return set_ == rhs.set_; }
    constexpr bool operator!=(const_iterator const &rhs) const { return set_ != rhs.set_; }
    const_iterator &operator++();
    const_iterator operator++(int);
    constexpr Enum operator*() const;

  private:
    int_type set_;
  };

public:
  constexpr EnumSet() = default;
  constexpr EnumSet(Enum value) : set_(mask(value)) {}

  constexpr EnumSet &add(Enum value);
  constexpr EnumSet &add(EnumSet set);

  constexpr EnumSet &operator+=(Enum value) { return add(value); }
  constexpr EnumSet &operator+=(EnumSet set) { return add(set); }

  constexpr bool contains(Enum value) const;
  constexpr bool contains(EnumSet set) const;

  constexpr std::size_t size() const { return count_bits_set(set_); }
  constexpr bool empty() const { return !set_; }

  void clear() { set_ = 0; }

  constexpr const_iterator begin() const { return const_iterator{set_}; }
  constexpr const_iterator end() const { return const_iterator{}; }

  constexpr int_type bit_mask() const { return set_; }

  constexpr bool operator==(EnumSet rhs) const { return set_ == rhs.set_; }
  constexpr bool operator!=(EnumSet rhs) const { return set_ != rhs.set_; }

  template <typename Out> friend Out &&operator<<(Out &&out, EnumSet set)
  {
    bool first = true;
    for (auto const value : set) {
      if (first) {
        first = false;
      } else {
        out << ',';
      }
      out << value;
    }
    return std::forward<Out>(out);
  }

  static constexpr int_type mask(Enum value);

private:
  int_type set_ = static_cast<int_type>(0);
};

#include <util/enum.inl>
