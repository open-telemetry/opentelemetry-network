//! Fast approximate division of `u64` by multiplying then right-shifting.
//!
//! Motivation. Integer division is relatively expensive on modern CPUs. Using Agner Fog's instruction tables [1]
//!   as reference, the 64-bit DIV instruction (unsigned divide, see [2] for instruction documentation) has a latency of
//!   35-88 cycles, and reciprocal throughput of 21-83 cycles/operation on the Skylake architecture (15 and 10 on Ice Lake).
//!   Code that performs a lot of this type of division in tightly dependent instruction chains can benefit from cheaper
//!   division.
//!
//! For example, in some telemetry/monitoring use-cases, there are millions of events per second, each with a nanosecond-scale
//!   timestamp. The monitoring system wants to aggregate to seconds. Given a measurement, the code needs to quickly find which
//!   1-second bin the measurement belongs to. When the telemetry streams have a minor amount of jitter from collection to aggregation,
//!   say up to 60 seconds (i.e., "close to real time"), the result needs to disambiguate the second a sample belongs to, but
//!   the result does not need to be unique across all seconds since the epoch. For example, the low 16-bits of the division is
//!   sufficient. The division can also be approximate: if the division puts every 999,999,998.5 nanoseconds together rather than
//!   exactly 1e9, the system still produces good aggregations in practice.
//!
//! Other instruction combinations are much cheaper than DIV. For example 64-bit MUL (integer multiplication) and SHRX (shift right
//!   without affecting flags):
//!    * MUL's latency is 3 cycles and reciprocal throughput is 1 on Skylake and Ice Lake
//!    * SHRX's latency is 1 cycle and reciprocal throughput is 0.5 on Skylake and Ice Lake
//!
//! Idea. We can approximate the 64-bit division with a multiplication followed by a shift. To divide by D, if we find two
//!   numbers M and S such that:
//!       D ~= 2^S / M                   (say up to an error of 0.00001%)
//!   then for a given x,
//!       x / D ~=  x * M / 2^S          (up to an error of (1 / (1 +- 0.00001%)), which would be close in practice to 0.00001%).
//!
//! So how should we choose M and S? Given a value for S, we can compute
//!    M = ((double)2^S / D) (where M is a u32 or u64)
//! With the trucation to make M an integer, we have
//!    M = 2^S / D - epsilon      where epsilon in [0, 1]
//!
//! Dividing the above by M and shuffling terms around, we can get the relative error:
//!    (2^S / M) / D   =   1 + epsilon / M
//! When 2^S is sufficiently larger than D, then M is large and the relative error (epsilon / M) is small.
//!
//! So large S is advantageous for precision. However, we cannot choose S to be arbitrarily large, because we are working with 64-bit
//!   arithmetic.
//!
//! The user specifies how many least-significant bits they need from the fast_div, B. After the shift, 64-S bits remain, so we need
//!   64-S >= B. We could set S = 64 - B to be the largest amount.
//!
//! The code below also fits M into a 32 bit value (nb, cannot see a reason why a u64 would be significanly less desireable).
//!   To get 2^S / D <= 2^32, we want D >= 2^(S-32), or log_2(D) >= S-32. The code computes
//!      S = min(64-B, floor(log_2(D))+32).    (because floor(log_2(D)) <= log_2(D))
//!
//! Examples:
//!    Say we want to divide a nanosecond timestamp to timeslots of 5 seconds, i.e., 5e9 nanos, and the
//!      application needs 16-bits of precision.
//!
//!    We have D = 5e9, B = 16.
//!         log_2(D) = 32.219280948873624
//!         floor(log_2(D)) = 32
//!         S = min(64-16, 32+32) = 48
//!         M = u32(2^48 / 5e9) = u32(56294.9953421312) = 56294
//!    The relative error is .9953421312 / 56294 ~ 0.0017%
//!    The division ends up being 2^S/M = 5000088405.703201, i.e., 5 seconds and around 88 microseconds
//!
//!    Note that in the above example, a lot of precision is lost from casting to u32. If instead the code rounded to the nearest
//!      integer, we would have
//!         M = round(2^48 / 5e9) = round(56294.9953421312) = 56295
//!         2^S/M = 4999999586.29818, i.e., 5 seconds less 414 nanoseconds
//!         and the relative error would be less than one part per million (PPM)
//!  
//!    Also, if the code only required 8 bits from the result:
//!         D = 5e9, B = 8.
//!         S = min(64-8, 32+32) = 56
//!         M = u32(2^56 / 5e9) = u32(14411518.807585588) = 14411518
//!         relative error is 0.807585588 / 14411518 ~ 5.6e-8, and 2^S/M is 5000000280.1875515, i.e., 5 seconds and 280 nanoseconds
//!         and with round() instead of u32(): 4999999933.242841 i.e., 5 seconds less 77 nanoseconds.
//!
//! NB when using fast_div with time. With fast_div, the time interval in the divisor is changed up to a small epsilon, for example
//!   5 seconds versus 5.000089 seconds as above. One side effect is that the boundaries between intervals also change, and do not land
//!   on full seconds. For example, when looking at nanoseconds since the Epoch (midnight, Jan 1, 1970), today's 5 second timeslots
//!   boundaries might land 3.5 seconds into the "round" 5 seconds. This is because the 89 microsecond difference really adds up over decades.
//!   Each application should consider whether this is acceptable.
//!
//! [1] https://www.agner.org/optimize/instruction_tables.pdf
//! [2] https://cdrdv2-public.intel.com/671110/325383-sdm-vol-2abcd.pdf

//! Remainder via low bits.
//!
//! For a number `x`, the low `S` bits of `(x * M)` represent the fixed-point
//! fractional part (with denominator `2^S`) of `x / D` when `M = 2^S / D`.
//! Thus, an approximate remainder can be computed as:
//!   rem ~= (D * ((x * M) & (2^S - 1))) >> S
//!
//! The multiplication by `D` may overflow 64-bit arithmetic. However, `D`
//! fits in 64 bits and `((x * M) & (2^S - 1))` fits in `S` bits which is not
//! more than `64 - B` bits (by construction). We therefore perform the
//! multiplication in Rust `u128` and then shift right by `S`. The final result
//! lies in `[0, D)` and fits in `u64`.

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct FastDiv {
    mul: u32,
    shift: u32,
}

impl FastDiv {
    /// Construct with known `mul` and `shift`.
    pub const fn from_mul_shift(mul: u32, shift: u32) -> Self {
        Self { mul, shift }
    }

    /// Construct by computing the `mul` and `shift` from a division amount.
    ///
    /// - `amt`: the amount by which to divide (e.g., nanoseconds per slot).
    /// - `required_bits`: number of bits retained from the result (1..=64).
    ///
    /// Panics in debug builds if `amt <= 0.0` or `required_bits == 0`.
    pub fn new(amt: f64, required_bits: u32) -> Self {
        debug_assert!(amt > 0.0, "amt must be > 0");
        debug_assert!(required_bits > 0, "required_bits must be >= 1");

        // division by `amt` will remove at least this many bits from the dividend
        let bits_removed = amt.log2().floor() as u32; // floor(log2(amt))

        // most precision obtained if right shift all but the required bits
        let mut shift = 64u32.saturating_sub(required_bits);

        // but the multiplier must fit in u32
        shift = shift.min(32 + bits_removed);

        // Safety note: require shift < 64 because (1 << 64) does not fit in u64.
        debug_assert!(shift < 64, "shift must be < 64");

        // Compute mul = floor((2^shift) / amt), stored as u32
        let two_pow_shift = (1u64 << shift) as f64;
        let mul = (two_pow_shift / amt) as u32; // truncates toward zero for positive values

        Self { mul, shift }
    }

    /// Returns the raw multiplier.
    pub const fn mul(&self) -> u32 {
        self.mul
    }

    /// Returns the right-shift amount.
    pub const fn shift(&self) -> u32 {
        self.shift
    }

    /// Returns the multiplication factor that is the estimated inverse of this division.
    pub fn estimated_reciprocal(&self) -> f64 {
        debug_assert!(self.shift < 64, "shift must be < 64");
        (1u128 << self.shift) as f64 / (self.mul as f64)
    }

    /// Applies the fast divide to `x` producing an approximate `x / amt`.
    ///
    /// This mirrors the C++ operator overload `operator/(uint64_t, fast_div)`.
    pub fn divide_u64(&self, x: u64) -> u32 {
        ((x.wrapping_mul(self.mul as u64)) >> self.shift) as u32
    }

    /// Approximates the remainder of `x` divided by `d` using the fixed-point
    /// low-`S` bits of `(x * M)`.
    ///
    /// Computes: `((d as u128) * (((x * M) & (2^S - 1)) as u128)) >> S`.
    /// The multiply is done in `u128` to avoid overflow; the result is then
    /// narrowed to `u64`, which is safe because it is less than `d`.
    pub fn remainder(&self, x: u64, d: u64) -> u64 {
        debug_assert!(self.shift < 64, "shift must be < 64");
        let s = self.shift;
        // Low S bits of (x * M)
        let xm = x.wrapping_mul(self.mul as u64);
        let mask = if s == 0 { 0 } else { (1u64 << s) - 1 };
        let frac = xm & mask;

        // Multiply in 128-bit to avoid overflow, then shift back by S.
        let prod = (d as u128) * (frac as u128);
        (prod >> s) as u64
    }
}

#[cfg(test)]
mod tests {
    use super::FastDiv;

    #[test]
    fn basic_properties() {
        let fd = FastDiv::from_mul_shift(1, 1);
        assert_eq!(fd.mul(), 1);
        assert_eq!(fd.shift(), 1);
        assert!((fd.estimated_reciprocal() - ((1u128 << 1) as f64) / 1.0).abs() < 1e-9);
        assert_eq!(fd.divide_u64(4), 2);
    }

    #[test]
    fn construct_from_amount_example_like() {
        // Similar to the example: divide nanoseconds by ~5s, require 16 bits
        let fd = FastDiv::new(5e9_f64, 16);
        // estimated_reciprocal should be close to ~5 seconds in nanoseconds
        let r = fd.estimated_reciprocal();
        assert!(r > 4.999e9 && r < 5.001e9);

        // Spot-check divide behavior monotonicity
        let a = 10_000_000_000u64; // 10s in nanos
        let b = 15_000_000_000u64; // 15s in nanos
        let da = fd.divide_u64(a);
        let db = fd.divide_u64(b);
        assert!(db >= da);
    }

    #[test]
    fn remainder_power_of_two_exact_ratio() {
        // Choose D = 2^13, and pick S so that M = 2^(S-13) fits in u32 exactly.
        // Here: S=44 -> M=2^31.
        let d: u64 = 1u64 << 13;
        let fd = FastDiv::from_mul_shift(1u32 << 31, 44);

        // For exact M=2^S/D, the remainder should be exact.
        let xs = [0u64, 1, d - 1, d, d + 1, (1u64 << 40) + 12345];
        for &x in &xs {
            assert_eq!(fd.remainder(x, d), x % d);
        }
    }
}
