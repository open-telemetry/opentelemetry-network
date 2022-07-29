/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME EntrypointError
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(none, 1)                                                                                                                   \
  X(unsupported_distro, 2)                                                                                                     \
  X(kernel_headers_fetch_error, 3)                                                                                             \
  X(kernel_headers_fetch_refuse, 4)                                                                                            \
  X(kernel_headers_missing_repo, 5)                                                                                            \
  X(kernel_headers_misconfigured_repo, 6)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
