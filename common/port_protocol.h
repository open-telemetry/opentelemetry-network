/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME PortProtocol
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(TCP, 1)                                                                                                                    \
  X(UDP, 2)                                                                                                                    \
  X(HTTP, 3)                                                                                                                   \
  X(PROXY, 4)                                                                                                                  \
  X(SCTP, 5)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
