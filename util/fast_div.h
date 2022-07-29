/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <math.h>

/**
 * Fast approximate division of uint64_t's by multiplying then right-shifting.
 */
class fast_div {
public:
  /**
   * C'tor with known mul, shift
   */
  fast_div(uint32_t mul, uint32_t shift);

  /**
   * C'tor that computes the mul and shift from the division amount
   * @param amt: the amount by which to divide
   * @param required_bits: the number of bits to be used from result
   */
  fast_div(double amt, uint32_t required_bits);

  uint32_t mul() { return mul_; }
  uint32_t shift() { return shift_; }

  /**
   * Returns the multiplication factor that is the estimated inverse of this
   * division.
   */
  double estimated_reciprocal() const;

private:
  friend uint32_t operator/(uint64_t x, const fast_div &divisor);

  uint32_t mul_;
  uint32_t shift_;
};

inline fast_div::fast_div(uint32_t _mul, uint32_t _shift) : mul_(_mul), shift_(_shift) {}

inline fast_div::fast_div(double amt, uint32_t required_bits)
{
  /* division by @amt will remove at least this many bits from the dividend */
  uint32_t bits_removed = (uint32_t)log2(amt);

  /* most precision obtained if right shift all but the required bits */
  shift_ = 64 - required_bits;

  /* but the multiplier must fit in uint32_t */
  shift_ = std::min(shift_, 32 + bits_removed);

  mul_ = (uint32_t)(((double)(1ull << shift_)) / amt);
}

inline double fast_div::estimated_reciprocal() const
{
  return (double)(1ull << shift_) / mul_;
}

inline uint32_t operator/(uint64_t x, const fast_div &divisor)
{
  return (uint32_t)((x * divisor.mul_) >> divisor.shift_);
}
