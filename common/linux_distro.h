/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME LinuxDistro
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(debian, 1)                                                                                                                 \
  X(ubuntu, 2)                                                                                                                 \
  X(rhel, 3)                                                                                                                   \
  X(centos, 4)                                                                                                                 \
  X(amazon, 5)                                                                                                                 \
  X(gcp_cos, 6)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
