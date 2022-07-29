/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME AgentLogKind
#define ENUM_TYPE uint32_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(BPF, 0)                                                                                                                    \
  X(UDP, 1)                                                                                                                    \
  X(DNS, 2)                                                                                                                    \
  X(TCP, 3)                                                                                                                    \
  X(HTTP, 4)                                                                                                                   \
  X(NAT, 6)                                                                                                                    \
  X(DOCKER, 7)                                                                                                                 \
  X(FLOW, 8)                                                                                                                   \
  X(CGROUPS, 9)                                                                                                                \
  X(PERF, 10)                                                                                                                  \
  X(PID, 11)                                                                                                                   \
  X(PROTOCOL, 12)                                                                                                              \
  X(TRACKED_PROCESS, 13)                                                                                                       \
  X(NOMAD, 14)                                                                                                                 \
  X(SOCKET, 15)
#define ENUM_DEFAULT BPF
#include <util/enum_operators.inl>
