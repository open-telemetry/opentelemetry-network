use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::{Duration, Instant};

use element_queue::ElementQueue;
use render_parser::Parser;
use timeslot::virtual_clock::VirtualClock;
use timeslot::FastDiv;

// Render-generated perfect hash and metadata for aggregation
use crate::aggregation_message_handler::AggregationMessageHandler;
use crate::aggregator::Aggregator;
use crate::otlp_encoding::OtlpExporter;
use encoder_ebpf_net_aggregation::hash::{aggregation_hash, AGGREGATION_HASH_SIZE};
use std::cell::RefCell;
use std::rc::Rc;

// Keep batch size reasonable to avoid starving other work
const K_MAX_RPC_BATCH_PER_QUEUE: usize = 10_000;

type Handler = Box<dyn Fn(u32, usize, u64, &[u8]) + 'static>;

pub struct AggregationCore {
    queues: Vec<ElementQueue>,
    clock: VirtualClock,
    parser: Parser<Handler, fn(u32) -> u32>,
    shard: u32,
    stop: Arc<AtomicBool>,
    aggregator: Rc<RefCell<Aggregator>>,
    handler: std::rc::Rc<AggregationMessageHandler>,
    // Divider for 30s timeslots
    timeslot_div: FastDiv,
    // Last processed message timestamp (ns)
    last_processed_ts: u64,
    window_end_ns: u64,
    exporter: OtlpExporter,
}

impl AggregationCore {
    pub fn new(
        eq_views: &[(usize, u32, u32)],
        shard: u32,
        enable_id_id: bool,
        enable_az_id: bool,
        endpoint: &str,
        disable_node_ip_field: bool,
        enable_metric_descriptions: bool,
    ) -> Self {
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
        let mut parser: Parser<Handler, fn(u32) -> u32> = Parser::new(hash_size, aggregation_hash);

        // Virtual clock configured with 30s timeslots
        let timeslot_div = FastDiv::new(30e9_f64, 16);
        let mut clock = VirtualClock::new(FastDiv::new(30e9_f64, 16));
        clock.add_inputs(queues.len());

        // Create aggregator and handler; closures capture Rc clones
        let aggregator = Rc::new(RefCell::new(Aggregator::new()));
        let handler = AggregationMessageHandler::new(&mut parser, aggregator.clone());

        let mut this = Self {
            queues,
            clock,
            parser,
            shard,
            stop: Arc::new(AtomicBool::new(false)),
            aggregator,
            handler,
            timeslot_div,
            last_processed_ts: 0,
            window_end_ns: 0,
            exporter: if !endpoint.is_empty() {
                OtlpExporter::with_endpoint(endpoint.to_string(), enable_metric_descriptions)
            } else {
                OtlpExporter::new_local(enable_metric_descriptions)
            },
        };

        // Configure aggregator flags and endpoint
        {
            let mut agg = this.aggregator.borrow_mut();
            agg.enable_id_id = enable_id_id;
            agg.enable_az_id = enable_az_id;
            agg.disable_node_ip_field = disable_node_ip_field;
        }
        this
    }

    // exporter held at core level

    pub fn stop(&self) {
        self.stop.store(true, Ordering::Relaxed);
    }

    pub fn run(&mut self) {
        if self.queues.is_empty() {
            return;
        }
        // No TLS required; closures hold Rc clones of the handler
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
                                        (ok.value)(self.shard, i, ok.timestamp, ok.message);
                                        // Track last processed timestamp while in current slot
                                        self.last_processed_ts = ok.timestamp;
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
                // Compute the window end timestamp aligned to the 30s slot
                let slot_ns = self.timeslot_div.estimated_reciprocal().round() as u64;
                let rem = self.timeslot_div.remainder(self.last_processed_ts, slot_ns);
                self.window_end_ns = self
                    .last_processed_ts
                    .saturating_sub(rem)
                    .saturating_add(slot_ns);
                self.aggregator
                    .borrow_mut()
                    .output_metrics(self.window_end_ns, &mut self.exporter);
            }
        }
    }
}
