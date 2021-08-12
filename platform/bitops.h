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

#ifndef INCLUDE_FASTPASS_PLATFORM_BITOPS_H_
#define INCLUDE_FASTPASS_PLATFORM_BITOPS_H_

/**
 * Loops i over all the indices of 1 bits in mask.
 * @note: changes mask, it will be zero at the end of the loop.
 */
#define mask_foreach(i, mask) for (i = __builtin_ctzll(mask); mask; mask &= (mask - 1), i = __builtin_ctzll(mask))

/**
 * Sets bit 'index' of mask 'mask' to 1 if 'cond' != 0
 * @assumes cond, when cast to signed 64 bit, is non-negative.
 */
#define set_bit_if_nz(mask, index, cond)                                                                                       \
  do {                                                                                                                         \
    mask |= (((u64)(-(s64)(cond))) >> 63) << (index);                                                                          \
  } while (0)

/**
 * returns 1 if (a < b), 0 otherwise
 */
#define nz_if_lt(a, b) ((u64)((s64)(a) - (b))) >> 63

#endif /* INCLUDE_FASTPASS_PLATFORM_BITOPS_H_ */
