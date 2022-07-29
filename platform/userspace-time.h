/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FASTPASS_PLATFORM_USERSPACE_TIME_H_
#define FASTPASS_PLATFORM_USERSPACE_TIME_H_

#include <platform/generic.h>
#include <platform/types.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline __attribute__((always_inline)) u64 fp_get_time_ns(void)
{
  struct timespec tp;

  if (unlikely(clock_gettime(CLOCK_REALTIME, &tp) != 0))
    return -1;

  return (1000 * 1000 * 1000) * (u64)tp.tv_sec + tp.tv_nsec;
}

/**
 * Get monotonic clock
 */
static inline __attribute__((always_inline)) u64 monotonic()
{
  struct timespec ts;
  int res = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (unlikely(res != 0))
    return -1;

  return (u64)ts.tv_sec * (1000 * 1000 * 1000) + ts.tv_nsec;
}

#include <x86intrin.h>
static inline __attribute__((always_inline)) u64 fp_monotonic_time_ns(void)
{
  return __rdtsc();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FASTPASS_PLATFORM_USERSPACE_TIME_H_ */
