/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <string_view>

inline std::string_view last_token(std::string_view s, char delimiter)
{
  auto const i = s.rfind(delimiter);
  return i == std::string_view::npos ? s : s.substr(i + 1);
}

/**
 * Converts the string `in` to type `T`.
 *
 * Returns the converted value on success.
 * On failure, returns `fallback`.
 */
template <typename T> T try_from_string(char const *in, T fallback = {});

/**
 * Converts the null-terminated string `in` to integer type `T` and stores the
 * result in `out`.
 *
 * Returns `true` on success.
 * On failure, returns `false` and `out` is untouched.
 */
template <typename T> bool integer_from_string(char const *in, T &out);

/**
 * Converts the null-terminated string `in` to floating point type `T` and
 * stores the result in `out`.
 *
 * Returns `true` on success.
 * On failure, returns `false` and `out` is untouched.
 */
template <typename T> bool floating_point_from_string(char const *in, T &out);

/**
 * Converts the string `in` to integer type `T`.
 *
 * Returns the converted value on success.
 * On failure, returns `fallback`.
 */

template <typename T> T try_integer_from_string(char const *in, T fallback = {});
/**
 * Converts the string `in` to floating point type `T`.
 *
 * Returns the converted value on success.
 * On failure, returns `fallback`.
 */
template <typename T> T try_floating_point_from_string(char const *in, T fallback = {});

#include <util/string.inl>
