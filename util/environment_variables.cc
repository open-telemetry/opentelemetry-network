// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/environment_variables.h>

#include <spdlog/fmt/fmt.h>

#include <stdexcept>

#include <cstdlib>

std::string_view try_get_env_var(char const *name, std::string_view fallback)
{
  auto const value = std::getenv(name);
  return value ? value : fallback;
}

std::string get_env_var(char const *name)
{
  if (auto const value = try_get_env_var(name); !value.empty()) {
    return std::string(value.data(), value.size());
  }

  throw std::invalid_argument(fmt::format("missing / empty environment variable {}", name));
}
