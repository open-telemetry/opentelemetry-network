/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
