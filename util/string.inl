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

#include <type_traits>

#include <cerrno>
#include <cstdlib>

template <typename T> bool integer_from_string(char const *in, T &out)
{
  using type = std::decay_t<T>;

  if constexpr (std::is_enum_v<type>) {
    if (std::underlying_type_t<type> underlying = {}; integer_from_string<std::underlying_type_t<type>>(in, underlying)) {
      out = static_cast<type>(underlying);
      return true;
    } else {
      return false;
    }
  } else {
    // because C APIs suck regarding constness
    char *end = const_cast<char *>(in);
    errno = 0;

    if constexpr (std::is_same_v<type, bool>) {
      std::string_view s{in};
      if (s == "0" || s == "false" || s == "False" || s == "FALSE" || s == "no" || s == "No" || s == "NO") {
        out = false;
      } else if (s == "1" || s == "true" || s == "True" || s == "TRUE" || s == "yes" || s == "Yes" || s == "YES") {
        out = true;
      } else {
        return false;
      }

      return true;
    } else if constexpr (
        std::is_same_v<type, char> || std::is_same_v<type, std::int8_t> || std::is_same_v<type, short> ||
        std::is_same_v<type, int> || std::is_same_v<type, long>) {
      auto const result = std::strtol(in, &end, 10);
      if (in == end || errno == ERANGE) {
        return false;
      }

      if constexpr (!std::is_same_v<type, long>) {
        if (result < static_cast<long>(std::numeric_limits<type>::min()) ||
            result > static_cast<long>(std::numeric_limits<type>::max())) {
          return false;
        }
      }

      out = static_cast<type>(result);
      return true;
    } else if constexpr (
        std::is_same_v<type, unsigned char> || std::is_same_v<type, std::uint8_t> || std::is_same_v<type, unsigned short> ||
        std::is_same_v<type, unsigned int> || std::is_same_v<type, unsigned long>) {
      auto const result = std::strtoul(in, &end, 10);
      if (in == end || errno == ERANGE) {
        return false;
      }

      if constexpr (!std::is_same_v<type, unsigned long>) {
        if (result > static_cast<unsigned long>(std::numeric_limits<type>::max())) {
          return false;
        }
      }

      out = static_cast<type>(result);
      return true;
    } else if constexpr (std::is_same_v<type, long long>) {
      auto const result = std::strtoll(in, &end, 10);
      if (in == end || errno == ERANGE) {
        return false;
      }
      out = result;
      return true;
    } else if constexpr (std::is_same_v<type, unsigned long long>) {
      auto const result = std::strtoull(in, &end, 10);
      if (in == end || errno == ERANGE) {
        return false;
      }
      out = result;
      return true;
    } else {
      static_assert(
          // this big list is needed because gcc is not smart enough to realize
          // that the static assert should only be evaluated if all other
          // conditionals failed
          std::is_same_v<type, bool> || std::is_same_v<type, char> || std::is_same_v<type, std::int8_t> ||
              std::is_same_v<type, short> || std::is_same_v<type, int> || std::is_same_v<type, long> ||
              std::is_same_v<type, unsigned char> || std::is_same_v<type, unsigned short> ||
              std::is_same_v<type, unsigned int> || std::is_same_v<type, unsigned long> || std::is_same_v<type, long long> ||
              std::is_same_v<type, unsigned long long>,
          "integer_from_string: unsupported type");
    }
  }
}

template <typename T> T try_integer_from_string(char const *in, T fallback)
{
  integer_from_string(in, fallback);
  return fallback;
}

template <typename T> bool floating_point_from_string(char const *in, T &out)
{
  using type = std::decay_t<T>;

  // because C APIs suck regarding constness
  char *end = const_cast<char *>(in);
  errno = 0;

  if constexpr (std::is_same_v<type, float>) {
    auto const result = std::strtof(in, &end);
    if (in == end || errno == ERANGE) {
      return false;
    }
    out = result;
    return true;
  } else if constexpr (std::is_same_v<type, double>) {
    auto const result = std::strtod(in, &end);
    if (in == end || errno == ERANGE) {
      return false;
    }
    out = result;
    return true;
  } else if constexpr (std::is_same_v<type, long double>) {
    auto const result = std::strtold(in, &end);
    if (in == end || errno == ERANGE) {
      return false;
    }
    out = result;
    return true;
  } else {
    static_assert(
        // this big list is needed because gcc is not smart enough to realize
        // that the static assert should only be evaluated if all other
        // conditionals failed
        std::is_same_v<type, float> || std::is_same_v<type, double> || std::is_same_v<type, long double>,
        "floating_point_from_string: unsupported type");
    return false;
  }
}

template <typename T> T try_floating_point_from_string(char const *in, T fallback)
{
  floating_point_from_string(in, fallback);
  return fallback;
}

template <typename T> bool from_string(char const *in, T &out)
{
  if constexpr (std::is_integral_v<T>) {
    return integer_from_string(in, out);
  } else if constexpr (std::is_floating_point_v<T>) {
    return floating_point_from_string(in, out);
  } else if constexpr (std::is_enum_v<T>) {
    return enum_from_string(in, out);
  } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
    out = in;
    return true;
  } else {
    static_assert(
        // this big list is needed because gcc is not smart enough to realize
        // that the static assert should only be evaluated if all other
        // conditionals failed
        std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>,
        "from_string: unsupported type");
  }
}

template <typename T> T try_from_string(char const *in, T fallback)
{
  from_string(in, fallback);
  return fallback;
}
