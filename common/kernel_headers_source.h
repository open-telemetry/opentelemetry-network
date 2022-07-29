/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME KernelHeadersSource
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(pre_installed, 1)                                                                                                          \
  X(pre_fetched, 2)                                                                                                            \
  X(dont_fetch, 3)                                                                                                             \
  X(fetched, 4)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
