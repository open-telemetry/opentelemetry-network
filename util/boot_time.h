/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "platform/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// time in nanoseconds
u64 get_boot_time();

#ifdef __cplusplus
}
#endif
