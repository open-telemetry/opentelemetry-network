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

#ifndef INCLUDE_FASTPASS_PLATFORM_TYPES_H_
#define INCLUDE_FASTPASS_PLATFORM_TYPES_H_

#ifndef __KERNEL__

#include <stdbool.h>
#include <stdint.h>

#ifdef __APPLE__
#include <sys/types.h>

typedef unsigned long long __u64;
typedef long long __s64;
typedef uint32_t __u32;
typedef uint32_t __be32;
typedef uint32_t __wsum;
typedef uint16_t __u16;
typedef uint16_t __be16;
typedef uint16_t __sum16;

#else /* #ifdef __APPLE__ */
#include <linux/types.h>
#endif /* else #ifdef __APPLE__ */

typedef unsigned __int128 u128;
typedef __int128 s128;
typedef uint64_t u64;
typedef int64_t s64;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint8_t u8;
typedef int8_t s8;

#endif /* #ifndef __KERNEL__ */

#ifdef __cplusplus

#include <chrono>

using namespace std::literals::chrono_literals;

#include <iomanip>

namespace {
template <typename Out> void print_large_unsigned(Out &out, u128 value)
{
  constexpr u128 mask = 10000000000000ull;
  auto const piece = static_cast<u64>(value % mask);
  u128 const head = value / mask;
  if (head) {
    print_large_unsigned(out, head);
    out << std::setfill('0') << std::setw(13) << piece;
  } else {
    out << piece;
  }
}
} // namespace

template <typename Out, typename T, typename = std::enable_if_t<std::is_same_v<T, s128> || std::is_same_v<T, u128>>>
Out &operator<<(Out &&out, T value)
{
  auto format_flags(out.flags());
  if constexpr (std::is_same_v<T, u128>) {
    print_large_unsigned(out, value);
  } else {
    if (value < 0) {
      out << '-';
      print_large_unsigned(out, static_cast<u128>(-value));
    } else {
      print_large_unsigned(out, static_cast<u128>(value));
    }
  }
  out.flags(format_flags);
  return out;
}

#endif /* __cplusplus */

#endif /* INCLUDE_FASTPASS_PLATFORM_TYPES_H_ */
