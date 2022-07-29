/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME CloudPlatform
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(aws, 1)                                                                                                                    \
  X(gcp, 2)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
