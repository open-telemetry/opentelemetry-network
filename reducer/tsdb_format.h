/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAMESPACE reducer
#define ENUM_NAME TsdbFormat
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(prometheus, 1, "")                                                                                                         \
  X(json, 2, "")                                                                                                               \
  X(otlp_grpc, 3, "")
#include <util/enum_operators.inl>
