/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME CollectedBlobType
#define ENUM_TYPE std::uint16_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(proc_pid_stat, 1)                                                                                                          \
  X(proc_pid_io, 2)                                                                                                            \
  X(proc_pid_status, 3)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
