/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAMESPACE reducer::ingest
#define ENUM_NAME Component
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(none, 0, "")                                                                                                               \
  X(udp, 1, "")                                                                                                                \
  X(socket, 2, "")                                                                                                             \
  X(process, 3, "")                                                                                                            \
  X(k8s_pod, 4, "")                                                                                                            \
  X(flow_update, 5, "")                                                                                                        \
  X(cgroup, 6, "")                                                                                                             \
  X(aws_network_interface, 7, "")                                                                                              \
  X(udp_drops, 8, "")                                                                                                          \
  X(dns, 9, "")                                                                                                                \
  X(agent, 10, "")                                                                                                             \
  X(heartbeat, 11, "")                                                                                                         \
  X(worker, 12, "")
#define ENUM_DEFAULT none
#include <util/enum_operators.inl>
