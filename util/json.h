/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <utility>

#include <cstdlib>

inline nlohmann::json const *follow_path(nlohmann::json const &object)
{
  return &object;
}

// nullptr if not an object or not found
template <typename... Path, typename Key>
nlohmann::json const *follow_path(nlohmann::json const &object, Key &&key, Path &&... path)
{
  if (!object.is_object()) {
    return nullptr;
  }

  if (auto i = object.find(key); i != object.end()) {
    return follow_path(*i, std::forward<Path>(path)...);
  }

  return nullptr;
}

// nullptr if not found
template <typename... Path> std::string const *try_get_string(nlohmann::json const &object, Path &&... path)
{
  auto value = follow_path(object, std::forward<Path>(path)...);

  if (!value) {
    return nullptr;
  }

  return value->template get_ptr<std::string const *>();
}

// empty if not found
template <typename... Path> std::string_view get_string_view(nlohmann::json const &object, Path &&... path)
{
  if (auto const value = try_get_string(object, std::forward<Path>(path)...)) {
    return *value;
  } else {
    return {};
  }
}

// empty if not found
template <typename... Path> std::string_view get_zstring_view(nlohmann::json const &object, Path &&... path)
{
  if (auto const value = try_get_string(object, std::forward<Path>(path)...)) {
    return {value->c_str(), value->size()};
  } else {
    return {};
  }
}

// std::nullopt if not found
template <typename T = std::int64_t, typename... Path>
std::optional<T> try_get_int(nlohmann::json const &object, Path &&... path)
{
  static_assert(std::is_integral_v<T>);

  auto value = follow_path(object, std::forward<Path>(path)...);

  if (!value || !value->is_number_integer()) {
    return std::nullopt;
  }

  return static_cast<T>(value->template get<std::int64_t>());
}

// converts string value to integer
// std::nullopt if not found or if value doesn't represent an integer
template <typename... Path> std::optional<std::int64_t> try_get_as_int(nlohmann::json const &object, Path &&... path)
{
  auto const value = try_get_string(object, std::forward<Path>(path)...);

  if (!value) {
    return std::nullopt;
  }

  auto const begin = value->c_str();
  char *end = nullptr;
  auto const result = std::strtoll(begin, &end, 10);
  if (!end || begin == end) {
    return std::nullopt;
  }

  return result;
}
