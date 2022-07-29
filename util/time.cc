// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/time.h>

#include <sys/time.h>
#include <unistd.h>

#include <stdexcept>
#include <system_error>

#include <cerrno>

std::uint64_t const clock_ticks_per_second = [] {
  if (auto const result = sysconf(_SC_CLK_TCK); result != -1) {
    return static_cast<std::uint64_t>(result);
  }

  throw std::system_error{errno, std::generic_category()};
}();
