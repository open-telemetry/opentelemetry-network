/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Platform-supplied API
 */

#ifndef PROTOCOL_PLATFORM_H_
#define PROTOCOL_PLATFORM_H_

#include "platform/bitops.h"
#include "platform/debug.h"
#include "platform/generic.h"

#ifdef __KERNEL__
#include "platform/linux-platform.h"
#else /* __KERNEL__ */
#include "platform/userspace-time.h"
#ifdef RTE_ARCH
#include "../src/arbiter/dpdk-platform.h"
#else /* #ifndef NO_DPDK */
#include "platform/no-dpdk.h"
#endif /* #ifndef NO_DPDK */
#endif /* __KERNEL__ */

/** FUNCTIONS IN PLATFORM.H **/

/**
 * static inline u64 fp_get_time_ns(void)
 *
 * returns the current real time (the time that is used to determine timeslots)
 */

#endif /* PROTOCOL_PLATFORM_H_ */
