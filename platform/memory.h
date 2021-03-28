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
