/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_FASTPASS_PLATFORM_MEMORY_H_
#define INCLUDE_FASTPASS_PLATFORM_MEMORY_H_

#if defined __KERNEL__

#define fp_free(ptr) kfree(ptr)
#define fp_calloc(typestr, num, size) kcalloc(num, size, GFP_KERNEL)
#define fp_malloc(typestr, size) kmalloc(size, GFP_KERNEL)

#elif defined RTE_ARCH

#include <rte_malloc.h>

#define fp_free(ptr) rte_free(ptr)
#define fp_calloc(typestr, num, size) rte_calloc(typestr, num, size, 0)
#define fp_malloc(typestr, size) rte_malloc(typestr, size, 0)

#else

#define fp_free(ptr) free(ptr)
#define fp_calloc(typestr, num, size) calloc(num, size)
#define fp_malloc(typestr, size) malloc(size)

#endif

#endif /* INCLUDE_FASTPASS_PLATFORM_MEMORY_H_ */
