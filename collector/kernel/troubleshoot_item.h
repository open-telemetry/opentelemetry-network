/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

#define ENUM_NAME TroubleshootItem
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(none, 0)                                                                                                                   \
  X(bpf_compilation_failed, 1)                                                                                                 \
  X(bpf_load_probes_failed, 2)                                                                                                 \
  X(operation_not_permitted, 3)                                                                                                \
  X(unexpected_exception, 4)
#define ENUM_DEFAULT none
#include <util/enum_operators.inl>
