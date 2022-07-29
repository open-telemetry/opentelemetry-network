/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAMESPACE channel
#define ENUM_NAME Component
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(none, 0)                                                                                                                   \
  X(tls, 1)                                                                                                                    \
  X(reconnecting_channel, 2)                                                                                                   \
  X(tcp, 3)                                                                                                                    \
  X(upstream, 4)
#define ENUM_DEFAULT none
#include <util/enum_operators.inl>
