//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
