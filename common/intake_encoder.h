/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME IntakeEncoder
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(binary, 0)                                                                                                                 \
  X(otlp_log, 1)
#define ENUM_DEFAULT binary
#include <util/enum_operators.inl>
