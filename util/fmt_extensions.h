/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <spdlog/fmt/ostr.h>
#include <filesystem>
#include <atomic>
#include <cstdint>

// Forward declarations to allow specializing fmt::formatter without including
// the enum headers everywhere.
enum class PortProtocol : std::uint8_t;
namespace reducer {
enum class TsdbFormat : std::uint16_t;
}

// Provide fmt::formatter specializations for our lightweight logging wrappers
// so they are always considered formattable by {fmt}/spdlog, even with
// stricter fmt v12 checks.

namespace fmt {

// waived_t<T>: treat as streamable via ostream_formatter
template <typename T>
struct formatter<logger::impl::waived_t<T>> : ostream_formatter {};

// either_t<...>
template <typename WhenTrue, typename WhenFalse>
struct formatter<logger::impl::either_t<WhenTrue, WhenFalse>> : ostream_formatter {};

// surrounded_t<T, Open, Close>
template <typename T, char Open, char Close>
struct formatter<logger::impl::surrounded_t<T, Open, Close>> : ostream_formatter {};

// kv_pair_t<Key, Value, Separator, Open, Close>
template <typename Key, typename Value, char Separator, char Open, char Close>
struct formatter<logger::impl::kv_pair_t<Key, Value, Separator, Open, Close>> : ostream_formatter {};

// callable_t<Fn>
template <typename Fn>
struct formatter<logger::impl::callable_t<Fn>> : ostream_formatter {};

// ClientType enum: stream using its operator<< (defined by enum utilities)
template <typename Char>
struct formatter<ClientType, Char> : ostream_formatter {};

// PortProtocol enum: stream using its operator<< (defined by enum utilities)
template <typename Char>
struct formatter<PortProtocol, Char> : ostream_formatter {};

// reducer::TsdbFormat enum: stream using its operator<< (defined by enum utilities)
template <typename Char>
struct formatter<reducer::TsdbFormat, Char> : ostream_formatter {};

// std::filesystem::path: stream via operator<<
template <typename Char>
struct formatter<std::filesystem::path, Char> : ostream_formatter {};

// Support formatting std::atomic<T> by formatting the contained value.
// Restrict to arithmetic types to avoid surprising behavior for complex T.
template <typename T, typename Char>
struct formatter<std::atomic<T>, Char> : fmt::formatter<T, Char> {
  template <typename FormatContext>
  auto format(std::atomic<T> const &a, FormatContext &ctx) const {
    if constexpr (std::is_arithmetic_v<T>) {
      T v = a.load(std::memory_order_relaxed);
      return fmt::formatter<T, Char>::format(v, ctx);
    } else {
      // Fallback: print address if non-arithmetic to avoid compile errors
      return fmt::formatter<const void *, Char>::format(static_cast<const void *>(&a), ctx);
    }
  }
};

} // namespace fmt
