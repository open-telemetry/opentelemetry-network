/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME Utility
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(none, 0)                                                                                                                   \
  X(curl, 1)
#define ENUM_DEFAULT none
#include <util/enum_operators.inl>
