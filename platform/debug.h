/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * debug.h
 */

#ifndef FASTPASS_DEBUG_H_
#define FASTPASS_DEBUG_H_

/* FASTPASS_PR_DEBUG defined in platform.h */
#if defined __KERNEL__
#define FASTPASS_PR_DEBUG(enable, fmt, a...)                                                                                   \
  do {                                                                                                                         \
    if (enable)                                                                                                                \
      printk(KERN_DEBUG "%s: " fmt, __func__, ##a);                                                                            \
  } while (0)

#elif defined RTE_ARCH
#define FASTPASS_PR_DEBUG(enable, fmt, a...)                                                                                   \
  do {                                                                                                                         \
    if (enable)                                                                                                                \
      COMM_DEBUG("%s: " fmt, __func__, ##a);                                                                                   \
  } while (0)

#else
#define FASTPASS_PR_DEBUG(enable, fmt, a...)                                                                                   \
  do {                                                                                                                         \
    if (enable)                                                                                                                \
      printf("%s: " fmt, __func__, ##a);                                                                                       \
  } while (0)
#endif

/* BUILD_BUG_ON */
#if defined __KERNEL__
#include <linux/bug.h>
#else
#define BUILD_BUG_ON(cond) ((void)sizeof(char[1 - 2 * !!(cond)]))
#endif

#ifdef CONFIG_IP_FASTPASS_DEBUG
extern bool fastpass_debug;
#define fp_debug(format, a...) FASTPASS_PR_DEBUG(1, format, ##a)
#else
#define fp_debug(format, a...)
#endif

#ifdef __KERNEL__
/*
 * Warning and debugging macros, (originally taken from DCCP)
 */
#define FASTPASS_WARN(fmt, a...) net_warn_ratelimited("%s: " fmt, __func__, ##a)
#define FASTPASS_CRIT(fmt, a...) printk(KERN_CRIT fmt " at %s:%d/%s()\n", ##a, __FILE__, __LINE__, __func__)
#define FASTPASS_BUG(a...)                                                                                                     \
  do {                                                                                                                         \
    FASTPASS_CRIT("BUG: " a);                                                                                                  \
    dump_stack();                                                                                                              \
  } while (0)
#define FASTPASS_BUG_ON(cond)                                                                                                  \
  do {                                                                                                                         \
    if (unlikely((cond) != 0))                                                                                                 \
      FASTPASS_BUG("\"%s\" holds (exception!)", __stringify(cond));                                                            \
  } while (0)

#define fp_info(format, a...) pr_info("%s: " format, __func__, ##a)

#ifndef NDEBUG
#define assert(cond) BUG_ON(!(cond))
#else
#define assert(cond)                                                                                                           \
  do {                                                                                                                         \
  } while (0)
#endif

#else

#include <assert.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* based on
 * http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
 */
static inline void fp_backtrace(void)
{
  void *array[50];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 50);

  // print out all the frames to stderr
  backtrace_symbols_fd(array, size, STDERR_FILENO);
}

#ifdef NO_DPDK
#define FASTPASS_BUG_SHOULD_PANIC 1
#else
#define FASTPASS_BUG_SHOULD_PANIC 0
#endif

static inline void panic(void)
{
  exit(-1);
}

#define FASTPASS_CRIT(fmt, a...) printf(fmt " at %s:%d/%s()\n", ##a, __FILE__, __LINE__, __func__)

/** from linux's include/asm-generic/bug.h */
#define FASTPASS_BUG()                                                                                                         \
  do {                                                                                                                         \
    FASTPASS_CRIT("BUG");                                                                                                      \
    fp_backtrace();                                                                                                            \
    if (FASTPASS_BUG_SHOULD_PANIC)                                                                                             \
      panic();                                                                                                                 \
  } while (0)

#define FASTPASS_BUG_ON(condition)                                                                                             \
  do {                                                                                                                         \
    if (unlikely(condition))                                                                                                   \
      FASTPASS_BUG();                                                                                                          \
  } while (0)

#define fp_info(format, a...) printf("%s: " format, __func__, ##a)

#endif

#endif /* FASTPASS_DEBUG_H_ */
