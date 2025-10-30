use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::{Duration, Instant};

use element_queue::{ElementQueue, EqError};
use render_parser::Parser;
use timeslot::virtual_clock::VirtualClock;

// Render-generated perfect hash and metadata for aggregation
use encoder_ebpf_net_aggregation::hash::{aggregation_hash, AGGREGATION_HASH_SIZE};
use encoder_ebpf_net_aggregation::wire_messages;

// Keep batch size reasonable to avoid starving other work
const K_MAX_RPC_BATCH_PER_QUEUE: usize = 10_000;

pub struct AggregationCore {
    queues: Vec<ElementQueue>,
    clock: VirtualClock,
    parser: Parser<(), fn(u32) -> u32>,
    shard: u32,
    stop: Arc<AtomicBool>,
}

impl AggregationCore {
    pub fn new(eq_views: &[(usize, u32, u32)], shard: u32) -> Self {
        // Build queues from contiguous storage descriptors
        let mut queues = Vec::with_capacity(eq_views.len());
        for (data, n_elems, buf_len) in eq_views.iter().cloned() {
            let ptr = data as *mut u8;
            let q = unsafe { ElementQueue::new_from_contiguous(n_elems, buf_len, ptr) }
                .expect("failed to create ElementQueue from contiguous storage");
            queues.push(q);
        }

        // Build parser with render-provided perfect hash
        let hash_size = AGGREGATION_HASH_SIZE as usize;
        let mut parser: Parser<(), fn(u32) -> u32> = Parser::new(hash_size, aggregation_hash);
        for md in wire_messages::all_message_metadata().into_iter() {
            // Insert with unit value; collisions are unexpected with perfect hash
            let _ = parser.add_message(md, ());
        }

        // Virtual clock with as many inputs as queues
        let mut clock = VirtualClock::default();
        clock.add_inputs(queues.len());

        Self {
            queues,
            clock,
            parser,
            shard,
            stop: Arc::new(AtomicBool::new(false)),
        }
    }

    pub fn stop(&self) {
        self.stop.store(true, Ordering::Relaxed);
    }

    pub fn run(&mut self) {
        if self.queues.is_empty() {
            return;
        }
        let mut next_idx: usize = 0;
        // Soft time budget to avoid long stalls; not strictly needed
        let _time_budget = Duration::from_millis(20);

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
                                // Parser expects the full buffer: ts(8) + message
                                match self.parser.handle(bytes) {
                                    Ok(ok) => {
                                        let msg = ok.message;
                                        if msg.len() >= 2 {
                                            let rpc_id = u16::from_ne_bytes([msg[0], msg[1]]);
                                            println!(
                                                "agg[shard={}] eq={} ts={} rpc_id={} size={}",
                                                self.shard,
                                                i,
                                                ok.timestamp,
                                                rpc_id,
                                                msg.len()
                                            );
                                        } else {
                                            println!(
                                                "agg[shard={}] eq={} ts={} rpc_id=? size={}",
                                                self.shard,
                                                i,
                                                ts,
                                                msg.len()
                                            );
                                        }
                                    }
                                    Err(e) => {
                                        println!(
                                            "agg[shard={}] eq={} ts={} parse_error={:?}",
                                            self.shard, i, ts, e
                                        );
                                    }
                                }
                            }
                            Err(_e) => break,
                        }
                        handled_in_queue += 1;
                    }

                    if start_cycle.elapsed() >= _time_budget {
                        break; // yield and rotate queues
                    }
                }

                // Publish read heads
                let _ = rb.finish();
            }

            if self.clock.advance() {
                println!("agg[shard={}] timeslot_advanced", self.shard);
            }
        }
    }
}
