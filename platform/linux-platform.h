/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * linux-platform.h
 */

#ifndef FASTPASS_LINUX_PLATFORM_H_
#define FASTPASS_LINUX_PLATFORM_H_

static inline u64 fp_get_time_ns(void)
{
  return ktime_to_ns(ktime_get_real());
}

static inline u64 fp_monotonic_time_ns(void)
{
  return ktime_to_ns(ktime_get());
}

#endif /* FASTPASS_LINUX_PLATFORM_H_ */
