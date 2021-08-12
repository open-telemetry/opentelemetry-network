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

#include <chrono>
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

template <typename Out> Out &operator<<(Out &&out, std::chrono::milliseconds value)
{
  out << value.count() << "ms";
  return out;
}

template <typename Out> Out &operator<<(Out &&out, std::chrono::microseconds value)
{
  out << value.count() << "\u00B5s";
  return out;
}

template <typename Out> Out &operator<<(Out &&out, std::chrono::nanoseconds value)
{
  out << value.count() << "ns";
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
