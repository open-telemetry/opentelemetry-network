/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAMESPACE reducer
#define ENUM_NAME CoreType
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(ingest, 2, "")                                                                                                             \
  X(match, 3, "")                                                                                                              \
  X(aggregate, 4, "")
#include <util/enum_operators.inl>
