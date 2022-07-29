/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * no-dpdk.h
 */

#ifndef INCLUDE_FASTPASS_PLATFORM_NO_DPDK_H_
#define INCLUDE_FASTPASS_PLATFORM_NO_DPDK_H_

#define FASTPASS_PR_DEBUG(enable, fmt, a...)                                                                                   \
  do {                                                                                                                         \
    if (enable)                                                                                                                \
      printf("%s: " fmt, __func__, ##a);                                                                                       \
  } while (0)

#ifndef likely
#define likely(x) __builtin_expect((x), 1)
#endif /* likely */

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif /* unlikely */

#endif /* INCLUDE_FASTPASS_PLATFORM_NO_DPDK_H_ */
