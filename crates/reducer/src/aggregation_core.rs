use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

use render_parser::Parser;

// Render-generated perfect hash and metadata for aggregation
use crate::aggregation_message_handler::AggregationMessageHandler;
use crate::aggregator::Aggregator;
use crate::otlp_encoding::OtlpExporter;
use crate::queue_handler::QueueHandler;
use encoder_ebpf_net_aggregation::hash::{aggregation_hash, AGGREGATION_HASH_SIZE};
use std::cell::RefCell;
use std::rc::Rc;

type Handler = Box<dyn Fn(u32, usize, u64, &[u8]) + 'static>;

pub struct AggregationCore {
    queue_handler: QueueHandler,
    parser: Parser<Handler, fn(u32) -> u32>,
    shard: u32,
    stop: Arc<AtomicBool>,
    aggregator: Rc<RefCell<Aggregator>>,
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
        // Shared stop flag and queue handler from descriptors
        let stop = Arc::new(AtomicBool::new(false));
        let queue_handler = QueueHandler::new_from_views(eq_views, stop.clone());

        // Build parser with render-provided perfect hash
        let hash_size = AGGREGATION_HASH_SIZE as usize;
        let mut parser: Parser<Handler, fn(u32) -> u32> = Parser::new(hash_size, aggregation_hash);

        // Create aggregator and handler; closures capture Rc clones
        let aggregator = Rc::new(RefCell::new(Aggregator::new()));
        let _handler = AggregationMessageHandler::new(&mut parser, aggregator.clone());

        let this = Self {
            queue_handler,
            parser,
            shard,
            stop,
            aggregator,
            exporter: if !endpoint.is_empty() {
                OtlpExporter::with_endpoint(endpoint.to_string(), enable_metric_descriptions)
            } else {
                OtlpExporter::new_local(enable_metric_descriptions)
            },
        };

        // Configure aggregator flags
        {
            let mut agg = this.aggregator.borrow_mut();
            agg.enable_id_id = enable_id_id;
            agg.enable_az_id = enable_az_id;
            agg.disable_node_ip_field = disable_node_ip_field;
        }
        this
    }

    pub fn stop(&self) {
        self.stop.store(true, Ordering::Relaxed);
    }

    pub fn run(&mut self) {
        if self.queue_handler.is_empty() {
            return;
        }

        // Bind parser and aggregator/exporter into queue callbacks
        let shard = self.shard;
        let parser = &self.parser;
        let agg = self.aggregator.clone();
        let exporter = &mut self.exporter;

        self.queue_handler.run(
            // Message handler: parse and dispatch
            move |queue_idx, bytes| match parser.handle(bytes) {
                Ok(ok) => (ok.value)(shard, queue_idx, ok.timestamp, ok.message),
                Err(e) => {
                    println!("agg[shard={}] eq={} parse_error={:?}", shard, queue_idx, e);
                }
            },
            // Timeslot end: flush metrics
            move |window_end_ns| {
                agg.borrow_mut().output_metrics(window_end_ns, exporter);
            },
        );
    }
}
