/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <string>
#include <type_traits>

#include "lookup3.h"
#include "platform/platform.h"

// Hasher functor that is compatible with std::hash, but uses lookup3 hashing
// function instead.

namespace util {

template <class T, class T2 = void> struct Lookup3Hasher {
  // Default implementation, fall-back to std::hash
  std::size_t operator()(T const &t) const noexcept { return std::hash<T>{}(t); }
};

// Integer cases
// TODO: use "if constexpr" to simplify the code once switch to C++17.
template <class T> struct Lookup3Hasher<T, typename std::enable_if<std::is_integral<T>::value>::type> {
  std::size_t operator()(T const &t) const noexcept
  {
    u64 val = static_cast<u64>(t);
    u32 pc = 0;
    u32 pb = 0;
    lookup3_hashword2((const uint32_t *)(&val), 2, &pc, &pb);

    return (static_cast<std::size_t>(pb) << 32) | static_cast<std::size_t>(pc);
  }
};

template <> struct Lookup3Hasher<std::string> {
  std::size_t operator()(std::string const &t) const noexcept
  {
    u32 pc = 0;
    u32 pb = 0;
    lookup3_hashlittle2((const void *)(t.c_str()), t.size(), &pc, &pb);

    return (static_cast<std::size_t>(pb) << 32) | static_cast<std::size_t>(pc);
  }
};

} // namespace util
