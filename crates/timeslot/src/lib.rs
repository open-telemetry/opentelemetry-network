//! timeslot crate
//!
//! Exposes `FastDiv` for fast approximate integer division suited for
//! time-slotting use cases.

mod fast_div;
pub mod virtual_clock;

pub use crate::fast_div::FastDiv;
pub use crate::virtual_clock::{UpdateError, VirtualClock};
