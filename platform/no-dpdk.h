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
