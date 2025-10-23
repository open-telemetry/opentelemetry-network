/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <spdlog/fmt/fmt.h>
#include <exception>
#include <system_error>
#include <variant>

#include <cstring>

/////////////////
// error codes //
/////////////////

namespace std {

template <typename Out> Out &operator<<(Out &&out, std::errc error_code)
{
  out << std::strerror(static_cast<int>(error_code));
  return out;
}

template <typename Out> Out &operator<<(Out &&out, std::error_code error)
{
  out << '[' << error.value() << ": " << error.message() << ']';
  return out;
}

} // namespace std

namespace fmt {

template <> struct formatter<std::error_code> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
  template <typename FormatContext> auto format(std::error_code const &error, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "[{}: {}]", error.value(), error.message());
  }
};

template <> struct formatter<std::errc> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
  template <typename FormatContext> auto format(std::errc error, FormatContext &ctx) const
  {
    return fmt::format_to(ctx.out(), "[{}: {}]", static_cast<int>(error), std::strerror(static_cast<int>(error)));
  }
};

} // namespace fmt

////////////////
// exceptions //
////////////////

namespace std {

template <typename Out> Out &operator<<(Out &&out, std::exception const &e)
{
  out << e.what();
  return out;
}

} // namespace std

////////////
// chrono //
////////////

namespace std {

template <typename Out> Out &operator<<(Out &&out, std::chrono::hours value)
{
  out << value.count() << "h";
  return out;
}
template <typename Out> Out &operator<<(Out &&out, std::chrono::minutes value)
{
  out << value.count() << "min";
  return out;
}

template <typename Out> Out &operator<<(Out &&out, std::chrono::seconds value)
{
  out << value.count() << "s";
  return out;
}


} // namespace std

////////////////
// containers //
////////////////

namespace std {

template <std::size_t Offset = 0, typename Out, typename T, typename... Args>
Out &operator<<(Out &&out, std::variant<T, Args...> const &value)
{
  if constexpr (Offset + 4 <= (1 + sizeof...(Args))) {
    switch (value.index()) {
    case Offset + 0: {
      out << std::get<Offset + 0>(value);
      break;
    }
    case Offset + 1: {
      out << std::get<Offset + 1>(value);
      break;
    }
    case Offset + 2: {
      out << std::get<Offset + 2>(value);
      break;
    }
    case Offset + 3: {
      out << std::get<Offset + 3>(value);
      break;
    }
    default: {
      operator<<<Offset + 4>(out, value);
      break;
    }
    }
  } else if constexpr (Offset + 2 <= (1 + sizeof...(Args))) {
    switch (value.index()) {
    case Offset + 0: {
      out << std::get<Offset + 0>(value);
      break;
    }
    case Offset + 1: {
      out << std::get<Offset + 1>(value);
      break;
    }
    default: {
      operator<<<Offset + 2>(out, value);
      break;
    }
    }
  } else if constexpr (Offset + 1 <= (1 + sizeof...(Args))) {
    if (value.index() == Offset + 0) {
      out << std::get<Offset + 0>(value);
    }
  }
  return out;
}

} // namespace std
