/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME CollectorStatus
#define ENUM_NAMESPACE collector
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(healthy, 0)                                                                                                                \
  X(unknown, 1)                                                                                                                \
  X(aws_describe_regions_error, 2)                                                                                             \
  X(aws_describe_network_interfaces_error, 3)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
