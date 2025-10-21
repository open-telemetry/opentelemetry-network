/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME IntakeEncoder
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X) X(binary, 0, "")
#define ENUM_DEFAULT binary
#include <util/enum_operators.inl>

inline std::string_view format_as(IntakeEncoder v)
{
  return to_string(v);
}
