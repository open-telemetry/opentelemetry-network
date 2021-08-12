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

#include <jitbuf/jb.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

struct short_string_behavior {
  struct truncate_t {
  };
  static constexpr truncate_t truncate = {};
  struct dont_truncate_t {
  };
  static constexpr dont_truncate_t no_truncate = {};
};

template <std::size_t N> struct short_string {
  static constexpr std::size_t max_len = N;

  /**
   * default c'tor
   */
  short_string() = default;

  void set(const char *other, std::size_t other_len)
  {
    assert(other_len <= max_len);
    len = std::min(other_len, max_len);
    std::memcpy(buf, other, len);
  }

  /**
   * Initializer from buffer
   */
  short_string(const char *other, std::size_t other_len) { set(other, other_len); }

  /**
   * Initializer from char array
   */
  template <std::size_t Size> short_string(char const (&data)[Size])
  {
    // exclude null terminator if present
    set(data, strnlen(data, Size / sizeof(*data)));
  }

  short_string(short_string_behavior::truncate_t, std::string_view view) { set(view.data(), std::min(view.size(), max_len)); }

  short_string(short_string_behavior::dont_truncate_t, std::string_view view)
  {
    // TODO: add proper runtime checking and ensure call sites handle error case gracefully
    set(view.data(), view.size());
  }

  short_string(short_string_behavior::truncate_t, jb_blob view)
  {
    set(view.data(), std::min(static_cast<std::size_t>(view.size()), max_len));
  }

  short_string(short_string_behavior::dont_truncate_t, jb_blob view)
  {
    // TODO: add proper runtime checking and ensure call sites handle error case gracefully
    set(view.data(), view.size());
  }

  /**
   * Copy c'tor
   */
  template <std::size_t Size, typename = std::enable_if_t<(Size <= max_len)>>
  short_string(const short_string<Size> &other) : len(other.len)
  {
    std::memcpy(buf, other.buf, other.len);
  }

  template <std::size_t Size, typename = std::enable_if_t<(Size <= max_len)>>
  short_string &operator=(const short_string<Size> &other)
  {
    set(other.buf, other.len);
    return *this;
  }

  template <std::size_t Size, typename = std::enable_if_t<(Size <= max_len)>> short_string &operator=(char const (&data)[Size])
  {
    // exclude null terminator if present
    set(data, Size - static_cast<bool>(Size && !data[Size - 1]));
    return *this;
  }

  short_string &operator=(std::string_view view)
  {
    set(view.data(), view.size());
    return *this;
  }

  short_string &operator=(jb_blob view)
  {
    set(view.data(), view.size());
    return *this;
  }

  bool operator==(const short_string<N> &rhs) const
  {
    if (rhs.len != len)
      return false;
    return (std::memcmp(buf, rhs.buf, len) == 0);
  }

  bool operator!=(const short_string<N> &rhs) const { return !operator==(rhs); }

  operator std::array<char, max_len>() const
  {
    std::array<char, max_len> result = {};
    std::copy(buf, buf + len, result.data());
    return result;
  }

  operator std::array<unsigned char, max_len>() const
  {
    std::array<unsigned char, max_len> result = {};
    std::copy(buf, buf + len, result.data());
    return result;
  }

  static constexpr short_string truncate(std::string_view value)
  {
    return short_string{value.data(), std::min(value.size(), max_len)};
  }

  static constexpr short_string truncate(jb_blob const &value)
  {
    return short_string{value.buf, std::min(static_cast<std::size_t>(value.len), max_len)};
  }

  /* convert to string */
  std::string to_string() const { return std::string(buf, len); }
  constexpr std::string_view to_string_view() const { return {buf, len}; }
  constexpr operator std::string_view() const { return to_string_view(); }

  char buf[N] = {0};
  uint16_t len = 0;

  // duck typing as a string-like thing
  //
  uint16_t size() const { return len; }
  bool empty() const { return !len; }

  unsigned char const *udata() const { return reinterpret_cast<unsigned char const *>(buf); }
  unsigned char *udata() { return reinterpret_cast<unsigned char *>(buf); }
  char const *data() const { return buf; }
  char *data() { return buf; }
  char const *begin() const { return buf; }
  char const *end() const { return buf + len; }

  char operator[](std::size_t i) const
  {
    assert(i < len);
    return buf[i];
  }

  char &operator[](std::size_t i)
  {
    assert(i < len);
    return buf[i];
  }

  template <typename Out> friend Out &operator<<(Out &&out, short_string const &s)
  {
    out.write(s.data(), s.size());
    return out;
  }
};

template <typename Array, typename = std::enable_if_t<std::is_array_v<Array>>, std::size_t Size = std::extent_v<Array>>
short_string<Size> to_short_string(std::string_view s)
{
  static_assert(Size > 0);
  return {s};
}

template <typename Char, std::size_t Size> short_string<Size> to_short_string(Char const (&data)[Size])
{
  static_assert(Size > 0);
  static_assert(sizeof(Char) == sizeof(char));
  return {data, Size};
}

template <typename... Char> auto as_short_string(Char const... c)
{
  constexpr std::size_t size = sizeof...(Char);
  static_assert(size > 0);
  char buffer[size] = {static_cast<char>(c)...};
  return to_short_string(buffer);
}
