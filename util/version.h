/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>

#include <string>
#include <string_view>
#include <tuple>
#include <utility>

class VersionInfo {
public:
  constexpr explicit VersionInfo(u32 major = 0, u32 minor = 0, u32 patch = 0, std::string_view signature = {})
      : version_(major, minor, patch, signature)
  {}

  constexpr VersionInfo(VersionInfo const &) = default;
  constexpr VersionInfo(VersionInfo &&) = default;

  void set(u32 major, u32 minor, u32 patch = 0, std::string_view signature = {})
  {
    version_ = std::make_tuple(major, minor, patch, signature);
  }

  constexpr u32 major() const { return std::get<0>(version_); }
  constexpr u32 minor() const { return std::get<1>(version_); }
  constexpr u32 patch() const { return std::get<2>(version_); }
  constexpr std::string_view signature() const { return std::get<3>(version_); }

  constexpr bool operator==(VersionInfo const &rhs) const { return version_ == rhs.version_; }
  constexpr bool operator!=(VersionInfo const &rhs) const { return !(*this == rhs); }

  constexpr bool operator<(VersionInfo const &rhs) const { return version_ < rhs.version_; }
  constexpr bool operator<=(VersionInfo const &rhs) const { return !(rhs < *this); }
  constexpr bool operator>(VersionInfo const &rhs) const { return rhs < *this; }
  constexpr bool operator>=(VersionInfo const &rhs) const { return !(*this < rhs); }

  VersionInfo &operator=(VersionInfo const &) = default;
  VersionInfo &operator=(VersionInfo &&) = default;

  template <typename Out> friend Out &&operator<<(Out &&out, VersionInfo const &value)
  {
    out << value.major() << '.' << value.minor() << '.' << value.patch();
    if (auto signature = value.signature(); !signature.empty()) {
      out << '-' << signature;
    }
    return std::forward<Out>(out);
  }

private:
  std::tuple<u32, u32, u32, std::string_view> version_;
};

// Provide an fmt-friendly customization without requiring fmt headers here.
// fmt v10+ will detect this via ADL and format VersionInfo as the returned type.
inline std::string format_as(VersionInfo const &value)
{
  std::string s = std::to_string(value.major());
  s.push_back('.');
  s += std::to_string(value.minor());
  s.push_back('.');
  s += std::to_string(value.patch());
  if (auto sig = value.signature(); !sig.empty()) {
    s.push_back('-');
    s.append(sig.begin(), sig.end());
  }
  return s;
}
