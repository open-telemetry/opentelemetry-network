//! VirtualClock: A clock driven by multiple inputs, discretized into timeslots.
//!
//! Input timestamps are divided into timeslots based on a fast approximate
//! divider. Once all inputs move out of the current timeslot, the clock can
//! advance (possibly skipping multiple slots). Timeslots are `u16` and deltas
//! are computed as `i16` with modular wrap-around semantics.
//!
//! This is a Rust port of reducer/util/virtual_clock.{h,cc} with semantics
//! verified against reducer/util/virtual_clock_test.cc.

use crate::FastDiv;

/// Timeslot index type (16-bit ring).
pub type Timeslot = u16;
/// Signed timeslot delta (wrap-around aware, via narrowing to i16).
type TimeslotDiff = i16;

/// Error codes for `VirtualClock::update`.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum UpdateError {
    /// The specified input cannot be updated at this time (EPERM).
    NotPermitted,
    /// The supplied timestamp points to a past timeslot (EINVAL).
    PastTimeslot,
}

#[derive(Clone, Debug, Default)]
struct Input {
    timeslot: Option<Timeslot>,
}

/// Clock driven by multiple inputs.
///
/// - Input timestamps are divided into timeslots based on `divider`.
/// - Once all inputs move out of the current timeslot, the clock advances.
/// - Inputs are first added using `add_inputs()`.
#[derive(Clone, Debug)]
pub struct VirtualClock {
    inputs: Vec<Input>,
    divider: FastDiv,
    timeslot_duration: f64,
    current_timeslot: Option<Timeslot>,
}

impl Default for VirtualClock {
    /// Constructs the object using the default timestamp divider of
    /// approximately 1 second (1e9) with 16 bits of precision retained.
    fn default() -> Self {
        let divider = FastDiv::new(1e9_f64, 16);
        Self::new(divider)
    }
}

impl VirtualClock {
    /// Constructs the object by using the specified timestamp divider.
    pub fn new(divider: FastDiv) -> Self {
        let timeslot_duration = divider.estimated_reciprocal();
        Self {
            inputs: Vec::new(),
            divider,
            timeslot_duration,
            current_timeslot: None,
        }
    }

    /// Adds `n` additional inputs.
    pub fn add_inputs(&mut self, n: usize) {
        self.inputs
            .resize_with(self.inputs.len() + n, Default::default);
    }

    /// Returns the current number of inputs this clock has.
    pub fn n_inputs(&self) -> usize {
        self.inputs.len()
    }

    /// Returns whether the specified input is current with this clock.
    /// Current means that the input timeslot equals the clock's timeslot.
    /// Panics if `input_index` is out of bounds.
    pub fn is_current(&self, input_index: usize) -> bool {
        self.current_timeslot.is_some()
            && self.inputs[input_index].timeslot == self.current_timeslot
    }

    /// Returns whether the specified input can be updated.
    /// Panics if `input_index` is out of bounds.
    pub fn can_update(&self, input_index: usize) -> bool {
        self.inputs[input_index].timeslot == self.current_timeslot
    }

    /// Updates the specified input with a timestamp.
    ///
    /// Returns Ok(()) on success;
    ///  - Err(NotPermitted) if the specified input can't be updated (`can_update()` would return false).
    ///  - Err(PastTimeslot) if the supplied timestamp points to a past timeslot.
    ///
    /// Panics if `input_index` is out of bounds.
    pub fn update(&mut self, input_index: usize, timestamp: u64) -> Result<(), UpdateError> {
        // Compute new_timeslot before borrowing `input` mutably to avoid borrow conflicts.
        let new_timeslot = self.map_timestamp(timestamp);

        let input = &mut self.inputs[input_index];

        if input.timeslot != self.current_timeslot {
            return Err(UpdateError::NotPermitted);
        }

        if let Some(old) = input.timeslot {
            let diff = signed_delta_u16(new_timeslot, old);
            if diff >= 0 {
                input.timeslot = Some(old.wrapping_add(diff as u16));
            } else {
                return Err(UpdateError::PastTimeslot);
            }
        } else {
            input.timeslot = Some(new_timeslot);
        }

        Ok(())
    }

    /// Duration of time slots, in timestamp units.
    pub fn timeslot_duration(&self) -> f64 {
        self.timeslot_duration
    }

    /// Current timeslot, or None if not yet initialized.
    pub fn current_timeslot(&self) -> Option<Timeslot> {
        self.current_timeslot
    }

    /// Advances this clock's timeslot, if possible. Returns `true` if advanced.
    pub fn advance(&mut self) -> bool {
        if let Some(cur) = self.current_timeslot {
            if let Some(advance_slots) = self.min_input_advance() {
                if advance_slots > 0 {
                    self.current_timeslot = Some(cur.wrapping_add(advance_slots as u16));
                    return true;
                }
            }
        } else {
            // Initialize the current timeslot to the earliest input timeslot, if available.
            self.current_timeslot = self.earliest_input_timeslot();
        }
        false
    }

    // --- helpers ---

    fn map_timestamp(&self, ts: u64) -> Timeslot {
        // Mirroring C++ operator/(uint64_t, fast_div), then truncate to u16.
        self.divider.divide_u64(ts) as u16
    }

    fn earliest_input_timeslot(&self) -> Option<Timeslot> {
        // If any input is unset, cannot determine earliest.
        if self.inputs.iter().any(|i| i.timeslot.is_none()) {
            return None;
        }

        let mut min_timeslot: Option<Timeslot> = None;
        for input in &self.inputs {
            let ts = input.timeslot.unwrap();
            min_timeslot = Some(match min_timeslot {
                Some(m) => m.min(ts),
                None => ts,
            });
        }
        let min_timeslot = min_timeslot.unwrap();

        let mut min_diff: Option<TimeslotDiff> = None;
        for input in &self.inputs {
            let ts = input.timeslot.unwrap();
            let diff = signed_delta_u16(ts, min_timeslot);
            min_diff = Some(match min_diff {
                Some(d) => d.min(diff),
                None => diff,
            });
        }
        let min_diff = min_diff.unwrap();
        Some(min_timeslot.wrapping_add(min_diff as u16))
    }

    fn min_input_advance(&self) -> Option<TimeslotDiff> {
        let cur = self.current_timeslot?;
        let mut min_adv: Option<TimeslotDiff> = None;
        for input in &self.inputs {
            let ts = input.timeslot?;
            let adv = signed_delta_u16(ts, cur);
            min_adv = Some(match min_adv {
                Some(a) => a.min(adv),
                None => adv,
            });
        }
        min_adv
    }
}

#[inline]
fn signed_delta_u16(new: u16, old: u16) -> i16 {
    // Compute modular difference on the u16 ring, then narrow to i16.
    // This exactly matches the C++ approach where narrowing wraps.
    new.wrapping_sub(old) as i16
}

#[cfg(test)]
mod tests {
    use super::{UpdateError, VirtualClock};

    const TIMESLOT_MIN: u16 = u16::MIN;
    const TIMESLOT_MAX: u16 = u16::MAX;

    #[test]
    fn empty() {
        let clock = VirtualClock::default();
        assert_eq!(clock.n_inputs(), 0);
        assert!(clock.current_timeslot().is_none());
    }

    #[test]
    fn add_inputs() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        assert_eq!(clock.n_inputs(), 2);
        assert!(clock.current_timeslot().is_none());
    }

    #[test]
    fn current_timeslot_init() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let timestamp = 0u64;

        assert_eq!(clock.update(0, timestamp), Ok(()));
        assert!(clock.current_timeslot().is_none());

        assert_eq!(clock.update(1, timestamp), Ok(()));
        assert!(clock.current_timeslot().is_none());

        assert_eq!(clock.advance(), false);
        assert!(clock.current_timeslot().is_some());
    }

    #[test]
    fn can_update() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);

        let timestamp = 0u64;
        assert!(clock.can_update(0));
        assert!(clock.can_update(1));

        assert_eq!(clock.update(0, timestamp), Ok(()));
        assert_eq!(clock.update(1, timestamp), Ok(()));

        assert!(!clock.can_update(0));
        assert!(!clock.can_update(1));

        assert_eq!(clock.advance(), false);
        assert!(clock.can_update(0));
        assert!(clock.can_update(1));

        // update input 1 past the current slot
        let ts_step = clock.timeslot_duration().ceil() as u64;
        assert_eq!(clock.update(1, timestamp + ts_step), Ok(()));

        // can't advance until input 0 advances
        assert_eq!(clock.advance(), false);

        assert!(clock.can_update(0));
        assert!(!clock.can_update(1));
        assert_eq!(
            clock.update(1, timestamp + ts_step),
            Err(UpdateError::NotPermitted)
        );

        // advance input 0 and the clock
        assert_eq!(clock.update(0, timestamp + ts_step), Ok(()));
        assert_eq!(clock.advance(), true);

        // inputs are in sync
        assert!(clock.is_current(0));
        assert!(clock.is_current(1));
        assert!(clock.can_update(0));
        assert!(clock.can_update(1));
    }

    #[test]
    fn initial_slot_min() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;

        assert_eq!(
            clock.update(0, ts_step * (TIMESLOT_MIN as u64 + 42)),
            Ok(())
        );
        assert_eq!(
            clock.update(1, ts_step * (TIMESLOT_MIN as u64 + 43)),
            Ok(())
        );
        assert_eq!(clock.advance(), false);

        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), TIMESLOT_MIN + 42);
    }

    #[test]
    fn initial_slot_mid() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;

        let timeslot_mid = TIMESLOT_MAX / 2;
        assert_eq!(
            clock.update(0, ts_step * ((timeslot_mid - 10) as u64)),
            Ok(())
        );
        assert_eq!(
            clock.update(1, ts_step * ((timeslot_mid + 10) as u64)),
            Ok(())
        );
        assert_eq!(clock.advance(), false);

        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), timeslot_mid - 10);
    }

    #[test]
    fn initial_slot_wrap() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;

        assert_eq!(clock.update(0, ts_step * (TIMESLOT_MAX as u64)), Ok(()));
        assert_eq!(
            clock.update(1, ts_step * ((TIMESLOT_MAX as u64) + 1)),
            Ok(())
        );
        assert_eq!(clock.advance(), false);

        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), TIMESLOT_MAX);
    }

    #[test]
    fn initial_slot_wrap_2() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;
        let timeslot_mid = TIMESLOT_MAX / 2;

        assert_eq!(
            clock.update(0, ts_step * ((timeslot_mid + 10) as u64)),
            Ok(())
        );
        assert_eq!(
            clock.update(1, ts_step * ((TIMESLOT_MAX as u64) + 1)),
            Ok(())
        );
        assert_eq!(clock.advance(), false);

        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), timeslot_mid + 10);
    }

    #[test]
    fn initial_slot_wrap_3() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;
        let timeslot_mid = TIMESLOT_MAX / 2;

        assert_eq!(
            clock.update(0, ts_step * ((timeslot_mid - 10) as u64)),
            Ok(())
        );
        assert_eq!(
            clock.update(1, ts_step * ((TIMESLOT_MAX as u64) + 1)),
            Ok(())
        );
        assert_eq!(clock.advance(), false);

        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), TIMESLOT_MIN);
    }

    #[test]
    fn advance_basic() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);

        let mut timestamp = 0u64;
        assert_eq!(clock.update(0, timestamp), Ok(()));
        assert_eq!(clock.update(1, timestamp), Ok(()));

        assert_eq!(clock.advance(), false);

        let ts_step = clock.timeslot_duration().ceil() as u64;
        timestamp += ts_step;

        assert_eq!(clock.update(0, timestamp), Ok(()));
        assert_eq!(clock.update(1, timestamp), Ok(()));

        assert_eq!(clock.advance(), true);
    }

    #[test]
    fn advance_catchup() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;

        let timestamp = ts_step * 42;
        assert_eq!(clock.update(0, timestamp), Ok(()));
        assert_eq!(clock.update(1, timestamp + 2 * ts_step), Ok(()));

        assert_eq!(clock.advance(), false);
        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), 42);

        assert_eq!(clock.update(0, timestamp + ts_step), Ok(()));
        assert_eq!(clock.advance(), true);
        assert_eq!(clock.current_timeslot().unwrap(), 43);

        assert_eq!(clock.update(0, timestamp + 2 * ts_step), Ok(()));
        assert_eq!(clock.advance(), true);
        assert_eq!(clock.current_timeslot().unwrap(), 44);

        assert!(clock.is_current(0));
        assert!(clock.is_current(1));
    }

    #[test]
    fn advance_skipslots() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;

        let timestamp = ts_step * 42;
        assert_eq!(clock.update(0, timestamp), Ok(()));
        assert_eq!(clock.update(1, timestamp + 2 * ts_step), Ok(()));
        assert_eq!(clock.advance(), false);

        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), 42);

        assert_eq!(clock.update(0, timestamp + 2 * ts_step), Ok(()));
        assert_eq!(clock.advance(), true);
        assert_eq!(clock.current_timeslot().unwrap(), 44);

        assert!(clock.is_current(0));
        assert!(clock.is_current(1));
    }

    #[test]
    fn advance_wraparound() {
        let mut clock = VirtualClock::default();
        clock.add_inputs(2);
        let ts_step = clock.timeslot_duration().ceil() as u64;

        let timestamp = ts_step * (TIMESLOT_MAX as u64);
        assert_eq!(clock.update(0, timestamp), Ok(()));
        assert_eq!(clock.update(1, timestamp), Ok(()));
        assert_eq!(clock.advance(), false);

        assert!(clock.current_timeslot().is_some());
        assert_eq!(clock.current_timeslot().unwrap(), TIMESLOT_MAX);

        let timestamp2 = timestamp + ts_step;
        assert_eq!(clock.update(0, timestamp2), Ok(()));
        assert_eq!(clock.advance(), false);
        assert_eq!(clock.current_timeslot().unwrap(), TIMESLOT_MAX);

        assert_eq!(clock.update(1, timestamp2), Ok(()));
        assert_eq!(clock.advance(), true);
        assert_eq!(clock.current_timeslot().unwrap(), TIMESLOT_MIN);

        assert!(clock.is_current(0));
        assert!(clock.is_current(1));
    }
}
