/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAMESPACE common
#define ENUM_NAME Component
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(none, 0)                                                                                                                   \
  X(debug_data, 1)
#define ENUM_DEFAULT none
#include <util/enum_operators.inl>
