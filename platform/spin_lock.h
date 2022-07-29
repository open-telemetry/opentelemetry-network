/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_FASTPASS_PLATFORM_SPIN_LOCK_H_
#define INCLUDE_FASTPASS_PLATFORM_SPIN_LOCK_H_

#ifdef __KERNEL__

#include <linux/spinlock.h>

#define fp_spinlock_t spinlock_t
#define fp_spin_init(lock)                                                                                                     \
  ({                                                                                                                           \
    spin_lock_init(lock);                                                                                                      \
    0;                                                                                                                         \
  })
#define fp_spin_lock(lock)                                                                                                     \
  ({                                                                                                                           \
    spin_lock_bh(lock);                                                                                                        \
    0;                                                                                                                         \
  })
#define fp_spin_unlock(lock) spin_unlock_bh(lock)
#define fp_spin_destroy(lock)                                                                                                  \
  do {                                                                                                                         \
  } while (0)

#else

#include <pthread.h>

#define fp_spinlock_t pthread_spinlock_t
#define fp_spin_init(lock) pthread_spin_init(lock, PTHREAD_PROCESS_SHARED)
#define fp_spin_lock(lock) pthread_spin_lock(lock)
#define fp_spin_unlock(lock) pthread_spin_unlock(lock)
#define fp_spin_destroy(lock) pthread_spin_destroy(lock)

#endif

#endif /* INCLUDE_FASTPASS_PLATFORM_SPIN_LOCK_H_ */
