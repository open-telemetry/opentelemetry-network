/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME ClientType
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(kernel, 1)                                                                                                                 \
  X(cloud, 2)                                                                                                                  \
  X(k8s, 3)                                                                                                                    \
  X(ingest, 4)                                                                                                                 \
  X(matching, 5)                                                                                                               \
  X(aggregation, 6)                                                                                                            \
  X(liveness_probe, 7)                                                                                                         \
  X(readiness_probe, 8)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
