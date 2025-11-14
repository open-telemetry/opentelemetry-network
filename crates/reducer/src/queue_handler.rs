use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::{Duration, Instant};

use element_queue::ElementQueue;
use timeslot::virtual_clock::VirtualClock;
use timeslot::FastDiv;

// Keep batch size reasonable to avoid starving other work
const K_MAX_RPC_BATCH_PER_QUEUE: usize = 10_000;

/// Drives reading from element queues and advancing a virtual clock, invoking
/// user-provided callbacks for each message and at the end of each timeslot.
pub struct QueueHandler {
    queues: Vec<ElementQueue>,
    clock: VirtualClock,
    timeslot_div: FastDiv,
    stop: Arc<AtomicBool>,
    last_processed_ts: u64,
}

impl QueueHandler {
    /// Construct from contiguous element-queue descriptors and a shared stop flag.
    pub fn new_from_views(eq_views: &[(usize, u32, u32)], stop: Arc<AtomicBool>) -> Self {
        // Build queues from contiguous storage descriptors
        let mut queues = Vec::with_capacity(eq_views.len());
        for (data, n_elems, buf_len) in eq_views.iter().cloned() {
            let ptr = data as *mut u8;
            let q = unsafe { ElementQueue::new_from_contiguous(n_elems, buf_len, ptr) }
                .expect("failed to create ElementQueue from contiguous storage");
            queues.push(q);
        }

        // Virtual clock configured with 30s timeslots (approximate)
        let timeslot_div = FastDiv::new(30e9_f64, 16);
        let mut clock = VirtualClock::new(timeslot_div.clone());
        clock.add_inputs(queues.len());

        Self {
            queues,
            clock,
            timeslot_div,
            stop,
            last_processed_ts: 0,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.queues.is_empty()
    }

    /// Run the queue handling loop until `stop` is set.
    ///
    /// - `handle_message(queue_idx, bytes)` is invoked for each element that
    ///   falls into the current timeslot.
    /// - `handle_timeslot_end(window_end_ns)` is invoked every time the clock
    ///   advances past the current timeslot; `window_end_ns` is aligned to the
    ///   configured timeslot size using an approximate divider.
    pub fn run<HM, HT>(&mut self, mut handle_message: HM, mut handle_timeslot_end: HT)
    where
        HM: FnMut(usize, &[u8]),
        HT: FnMut(u64),
    {
        if self.queues.is_empty() {
            return;
        }

        let mut next_idx: usize = 0;
        let time_budget = Duration::from_millis(20);

        while !self.stop.load(Ordering::Relaxed) {
            let start_cycle = Instant::now();

            for _ in 0..self.queues.len() {
                let i = next_idx;
                next_idx = (next_idx + 1) % self.queues.len();

                if !self.clock.can_update(i) {
                    continue;
                }

                // RAII read guard
                let rb = self.queues[i].start_read();
                let mut handled_in_queue = 0usize;

                while handled_in_queue < K_MAX_RPC_BATCH_PER_QUEUE
                    && self.clock.can_update(i)
                    && rb.peek_len().is_ok()
                {
                    // Peek timestamp (native-endian u64 at start of element)
                    let ts = match rb.peek_value::<u64>() {
                        Ok(v) => v,
                        Err(_e) => {
                            // Drain malformed element and continue
                            let _ = rb.read();
                            continue;
                        }
                    };

                    // Update clock for this input
                    match self.clock.update(i, ts) {
                        Ok(()) => {}
                        Err(timeslot::virtual_clock::UpdateError::PastTimeslot) => {
                            // Drain and continue
                            let _ = rb.read();
                            continue;
                        }
                        Err(timeslot::virtual_clock::UpdateError::NotPermitted) => {
                            break;
                        }
                    }

                    if self.clock.is_current(i) {
                        match rb.read() {
                            Ok(bytes) => {
                                handle_message(i, bytes);
                                // Track last processed timestamp while in current slot
                                self.last_processed_ts = ts;
                            }
                            Err(_e) => break,
                        }
                        handled_in_queue += 1;
                    }

                    if start_cycle.elapsed() >= time_budget {
                        break; // yield and rotate queues
                    }
                }

                // Publish read heads
                let _ = rb.finish();
            }

            if self.clock.advance() {
                // Compute the window end timestamp aligned to the 30s slot
                let slot_ns = self.timeslot_div.estimated_reciprocal().round() as u64;
                let rem = self.timeslot_div.remainder(self.last_processed_ts, slot_ns);
                let window_end_ns = self
                    .last_processed_ts
                    .saturating_sub(rem)
                    .saturating_add(slot_ns);
                handle_timeslot_end(window_end_ns);
            }
        }
    }
}
