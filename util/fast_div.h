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
 * 
 * Motivation. Integer division is relatively expensive on modern CPUs. Using Agner Fog's instruction tables [1]
 *   as reference, the 64-bit DIV instruction (unsigned divide, see [2] for instruction documentation) has a latency of
 *   35-88 cycles, and reciprocal throughput of 21-83 cycles/operation on the Skylake architecture (15 and 10 on Ice Lake). 
 *   Code that performs a lot of this type of division in tightly dependent instruction chains can benefit from cheaper 
 *   division.
 * 
 * For example, in some telemetry/monitoring use-cases, there are millions of events per second, each with a nanosecond-scale
 *   timestamp. The monitoring system wants to aggregate to seconds. Given a measurement, the code needs to quickly find which 
 *   1-second bin the measurement belongs to. When the telemetry streams have a minor amount of jitter from collection to aggregation,
 *   say up to 60 seconds (i.e., "close to real time"), the result needs to disambiguate the second a sample belongs to, but 
 *   the result does not need to be unique across all seconds since the epoch. For example, the low 16-bits of the division is 
 *   sufficient. The division can also be approximate: if the division puts every 999,999,998.5 nanoseconds together rather than
 *   exactly 1e9, the system still produces good aggregations in practice.
 * 
 * Other instruction combinations are much cheaper than DIV. For example 64-bit MUL (integer multiplication) and SHRX (shift right 
 *   without affecting flags):
 *    * MUL's latency is 3 cycles and reciprocal throughput is 1 on Skylake and Ice Lake
 *    * SHRX's latency is 1 cycle and reciprocal throughput is 0.5 on Skylake and Ice Lake
 * 
 * Idea. We can approximate the 64-bit division with a multiplication followed by a shift. To divide by D, if we find two 
 *   numbers M and S such that:
 *       D ~= 2^S / M                   (say up to an error of 0.00001%)
 *   then for a given x,
 *       x / D ~=  x * M / 2^S          (up to an error of (1 / (1 +- 0.00001%)), which would be close in practice to 0.00001%).
 * 
 * So how should we choose M and S? Given a value for S, we can compute
 *    M = ((double)2^S / D) (where M is a u32 or u64)
 * With the trucation to make M an integer, we have
 *    M = 2^S / D - epsilon      where epsilon in [0, 1]
 * 
 * Dividing the above by M and shuffling terms around, we can get the relative error:
 *    (2^S / M) / D   =   1 + epsilon / M
 * When 2^S is sufficiently larger than D, then M is large and the relative error (epsilon / M) is small.
 * 
 * So large S is advantageous for precision. However, we cannot choose S to be arbitrarily large, because we are working with 64-bit 
 *   arithmetic. 
 * 
 * The user specifies how many least-significant bits they need from the fast_div, B. After the shift, 64-S bits remain, so we need 
 *   64-S >= B. We could set S = 64 - B to be the largest amount.
 * 
 * The code below also fits M into a 32 bit value (nb, cannot see a reason why a u64 would be significanly less desireable).
 *   To get 2^S / D <= 2^32, we want D >= 2^(S-32), or log_2(D) >= S-32. The code computes
 *      S = min(64-B, floor(log_2(D))+32).    (because floor(log_2(D)) <= log_2(D))
 * 
 * Examples:
 *    Say we want to divide a nanosecond timestamp to timeslots of 5 seconds, i.e., 5e9 nanos, and the 
 *      application needs 16-bits of precision.
 * 
 *    We have D = 5e9, B = 16. 
 *         log_2(D) = 32.219280948873624
 *         floor(log_2(D)) = 32
 *         S = min(64-16, 32+32) = 48
 *         M = u32(2^48 / 5e9) = u32(56294.9953421312) = 56294
 *    The relative error is .9953421312 / 56294 ~ 0.0017%
 *    The division ends up being 2^S/M = 5000088405.703201, i.e., 5 seconds and around 88 microseconds
 * 
 *    Note that in the above example, a lot of precision is lost from casting to u32. If instead the code rounded to the nearest
 *      integer, we would have
 *         M = round(2^48 / 5e9) = round(56294.9953421312) = 56295
 *         2^S/M = 4999999586.29818, i.e., 5 seconds less 414 nanoseconds
 *         and the relative error would be less than one part per million (PPM)
 *  
 *    Also, if the code only required 8 bits from the result:
 *         D = 5e9, B = 8. 
 *         S = min(64-8, 32+32) = 56
 *         M = u32(2^56 / 5e9) = u32(14411518.807585588) = 14411518
 *         relative error is 0.807585588 / 14411518 ~ 5.6e-8, and 2^S/M is 5000000280.1875515, i.e., 5 seconds and 280 nanoseconds
 *         and with round() instead of u32(): 4999999933.242841 i.e., 5 seconds less 77 nanoseconds.
 * 
 * NB when using fast_div with time. With fast_div, the time interval in the divisor is changed up to a small epsilon, for example 
 *   5 seconds versus 5.000089 seconds as above. One side effect is that the boundaries between intervals also change, and do not land
 *   on full seconds. For example, when looking at nanoseconds since the Epoch (midnight, Jan 1, 1970), today's 5 second timeslots
 *   boundaries might land 3.5 seconds into the "round" 5 seconds. This is because the 89 microsecond difference really adds up over decades.
 *   Each application should consider whether this is acceptable.
 * 
 * [1] https://www.agner.org/optimize/instruction_tables.pdf
 * [2] https://cdrdv2-public.intel.com/671110/325383-sdm-vol-2abcd.pdf
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
