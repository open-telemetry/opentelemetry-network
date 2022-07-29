/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_JITBUF_JB_H_
#define INCLUDE_JITBUF_JB_H_

#ifdef __cplusplus
#include <array>
#include <limits>
#include <string>
#include <string_view>

#include <cassert>
#include <cstdint>
#include <cstring>

extern "C" {
#endif /* __cplusplus */

struct jb_descriptor {
  int size;
  const char *buf;
};

struct jb_package {
  const struct jb_descriptor **descriptors;
  const struct jb_descriptor **ext_descriptors;
  int size;
};

/**
 * RPC description
 * @rpc_id: the RPC ID
 * @size: the length of an encoded message
 * @descriptor: the short descriptor
 * @ext_descriptor: the extended descriptor
 */
struct jb_rpc {
  unsigned int rpc_id;
  int size;
  const struct jb_descriptor *descriptor;
  const struct jb_descriptor *ext_descriptor;
};

/**
 * Information about a blob (chunk of bytes) adjacent to a jb message
 */
struct jb_blob {
  const char *buf;
  unsigned short len;

#ifdef __cplusplus
  constexpr jb_blob() : buf(nullptr), len(0) {}

  constexpr jb_blob(const char *buf, unsigned short len) : buf(buf), len(len) {}

  explicit constexpr jb_blob(std::string_view from) : jb_blob(from.data(), static_cast<unsigned short>(from.size()))
  {
    assert(from.size() <= std::numeric_limits<unsigned short>::max());
  }

  explicit jb_blob(std::string const &from) : jb_blob(from.data(), static_cast<unsigned short>(from.size()))
  {
    assert(from.size() <= std::numeric_limits<unsigned short>::max());
  }

  std::string to_string() const { return std::string(buf, len); }

  constexpr std::string_view string_view() const { return {buf, len}; }

  constexpr operator std::string_view() const { return string_view(); }

  constexpr bool operator==(std::string_view rhs) const { return string_view() == rhs; }

  constexpr bool operator!=(std::string_view rhs) const { return string_view() != rhs; }

  jb_blob &operator=(std::string_view from)
  {
    assert(from.size() <= std::numeric_limits<unsigned short>::max());
    buf = from.data();
    len = from.size();
    return *this;
  }

  constexpr char const *data() const { return buf; }
  constexpr unsigned short size() const { return len; }
  constexpr bool empty() const { return !len; }

#endif /* __cplusplus */
};

#ifdef __cplusplus
}

template <std::size_t Size> constexpr jb_blob to_jb_blob(std::uint8_t const (&data)[Size])
{
  static_assert(Size <= std::numeric_limits<unsigned short>::max());
  auto const length = strnlen(reinterpret_cast<char const *>(data), Size / sizeof(*data));
  return {reinterpret_cast<char const *>(data), static_cast<unsigned short>(length)};
}

template <std::size_t Size> constexpr jb_blob to_jb_blob(std::array<std::uint8_t, Size> const &data)
{
  static_assert(Size <= std::numeric_limits<unsigned short>::max());
  auto const length = strnlen(reinterpret_cast<char const *>(data.data()), Size);
  return {reinterpret_cast<char const *>(data.data()), static_cast<unsigned short>(length)};
}

template <typename Out> Out &operator<<(Out &&out, jb_blob const &blob)
{
  out << std::string_view(blob.buf, blob.len);
  return out;
}

inline std::string &assign_jb(std::string &lhs, jb_blob const &rhs)
{
  lhs.assign(rhs.buf, rhs.len);
  return lhs;
}

template <std::size_t Size> std::string &assign_render_array(std::string &lhs, std::uint8_t const (&rhs)[Size])
{
  lhs.assign(reinterpret_cast<char const *>(rhs), strnlen(reinterpret_cast<char const *>(rhs), Size / sizeof(*rhs)));
  return lhs;
}

template <std::size_t Size> std::string render_array_to_string(std::uint8_t const (&data)[Size])
{
  return {reinterpret_cast<char const *>(data), strnlen(reinterpret_cast<char const *>(data), Size / sizeof(*data))};
}

template <std::size_t Size> std::string_view render_array_to_string_view(std::uint8_t const (&data)[Size])
{
  return {reinterpret_cast<char const *>(data), strnlen(reinterpret_cast<char const *>(data), Size / sizeof(*data))};
}

#endif /* __cplusplus */

#endif /* INCLUDE_JITBUF_JB_H_ */
