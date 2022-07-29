// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/string.h>

template <typename T> T try_get_env_value(char const *name, T fallback)
{
  if (auto const value = std::getenv(name)) {
    return try_from_string<T>(value, fallback);
  }

  return fallback;
}
