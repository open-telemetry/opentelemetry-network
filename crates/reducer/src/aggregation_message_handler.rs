//! Message decoding and dispatch into the Aggregator using a handler struct.

use std::cell::RefCell;
use std::rc::Rc;

use render_parser::Parser;

use crate::aggregator::{AggRootKey, Aggregator, Az, Direction, Node, Side};
use crate::metrics::{DnsMetrics, HttpMetrics, TcpMetrics, UdpMetrics};
use encoder_ebpf_net_aggregation::parsed_message;
use encoder_ebpf_net_aggregation::wire_messages;

type Handler = Box<dyn Fn(u32, usize, u64, &[u8]) + 'static>;

#[derive(Clone)]
pub struct AggregationMessageHandler {
    agg: Rc<RefCell<Aggregator>>,
}

impl AggregationMessageHandler {
    pub fn new(
        parser: &mut Parser<Handler, fn(u32) -> u32>,
        agg: Rc<RefCell<Aggregator>>,
    ) -> Rc<Self> {
        let rc = Rc::new(Self { agg });

        // Register closures that capture Rc<Self>
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__agg_root_start::metadata(),
                Box::new(move |_shard, q, _ts, buf| h.on_agg_root_start(q, buf)),
            );
        }
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__agg_root_end::metadata(),
                Box::new(move |_shard, q, _ts, buf| h.on_agg_root_end(q, buf)),
            );
        }
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__update_node::metadata(),
                Box::new(move |_shard, q, _ts, buf| h.on_update_node(q, buf)),
            );
        }
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__update_tcp_metrics::metadata(),
                Box::new(move |_shard, q, _ts, buf| h.on_update_tcp(q, buf)),
            );
        }
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__update_udp_metrics::metadata(),
                Box::new(move |_shard, q, _ts, buf| h.on_update_udp(q, buf)),
            );
        }
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__update_http_metrics::metadata(),
                Box::new(move |_shard, q, _ts, buf| h.on_update_http(q, buf)),
            );
        }
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__update_dns_metrics::metadata(),
                Box::new(move |_shard, q, _ts, buf| h.on_update_dns(q, buf)),
            );
        }
        {
            let h = rc.clone();
            let _ = parser.add_message(
                wire_messages::jb_aggregation__pulse::metadata(),
                Box::new(move |_shard, _q, _ts, buf| h.on_pulse(buf)),
            );
        }

        rc
    }

    fn on_agg_root_start(&self, queue_idx: usize, buf: &[u8]) {
        match parsed_message::agg_root_start::decode(buf) {
            Ok(msg) => {
                let key: AggRootKey = (queue_idx, msg._ref);
                self.agg.borrow_mut().agg_root_start(key);
            }
            Err(_e) => self
                .agg
                .borrow_mut()
                .events
                .inc_decode_error_agg_root_start(),
        }
    }

    fn on_agg_root_end(&self, queue_idx: usize, buf: &[u8]) {
        match parsed_message::agg_root_end::decode(buf) {
            Ok(msg) => {
                let key: AggRootKey = (queue_idx, msg._ref);
                self.agg.borrow_mut().agg_root_end(key);
            }
            Err(_e) => self.agg.borrow_mut().events.inc_decode_error_agg_root_end(),
        }
    }

    fn on_update_node(&self, queue_idx: usize, buf: &[u8]) {
        match parsed_message::update_node::decode(buf) {
            Ok(msg) => {
                let key: AggRootKey = (queue_idx, msg._ref);
                let az = Az {
                    az: msg.az.to_string(),
                    role: msg.role.to_string(),
                    version: msg.version.to_string(),
                    env: msg.env.to_string(),
                    ns: msg.ns.to_string(),
                    node_type: msg.node_type as u8,
                    process: msg.process.to_string(),
                    container: msg.container.to_string(),
                    role_uid: msg.role_uid.to_string(),
                };
                let node = Node {
                    id: msg.id.to_string(),
                    address: msg.address.to_string(),
                    pod_name: msg.pod_name.to_string(),
                };
                self.agg.borrow_mut().update_node(
                    key,
                    if msg.side == 0 { Side::A } else { Side::B },
                    az,
                    node,
                );
            }
            Err(_e) => self.agg.borrow_mut().events.inc_decode_error_update_node(),
        }
    }

    fn on_update_tcp(&self, queue_idx: usize, buf: &[u8]) {
        match parsed_message::update_tcp_metrics::decode(buf) {
            Ok(msg) => {
                let key: AggRootKey = (queue_idx, msg._ref);
                let m = TcpMetrics {
                    active_sockets: msg.active_sockets as u64,
                    sum_bytes: msg.sum_bytes as u64,
                    sum_srtt: msg.sum_srtt as u64,
                    sum_delivered: msg.sum_delivered as u64,
                    sum_retrans: msg.sum_retrans as u64,
                    active_rtts: msg.active_rtts as u64,
                    syn_timeouts: msg.syn_timeouts as u64,
                    new_sockets: msg.new_sockets as u64,
                    tcp_resets: msg.tcp_resets as u64,
                };
                let dir = if msg.direction == 0 {
                    Direction::AtoB
                } else {
                    Direction::BtoA
                };
                self.agg.borrow_mut().add_tcp(key, dir, m);
            }
            Err(_e) => self.agg.borrow_mut().events.inc_decode_error_update_tcp(),
        }
    }

    fn on_update_udp(&self, queue_idx: usize, buf: &[u8]) {
        match parsed_message::update_udp_metrics::decode(buf) {
            Ok(msg) => {
                let key: AggRootKey = (queue_idx, msg._ref);
                let m = UdpMetrics {
                    active_sockets: msg.active_sockets as u64,
                    bytes: msg.bytes as u64,
                    addr_changes: msg.addr_changes as u64,
                    packets: msg.packets as u64,
                    drops: msg.drops as u64,
                };
                let dir = if msg.direction == 0 {
                    Direction::AtoB
                } else {
                    Direction::BtoA
                };
                self.agg.borrow_mut().add_udp(key, dir, m);
            }
            Err(_e) => self.agg.borrow_mut().events.inc_decode_error_update_udp(),
        }
    }

    fn on_update_http(&self, queue_idx: usize, buf: &[u8]) {
        match parsed_message::update_http_metrics::decode(buf) {
            Ok(msg) => {
                let key: AggRootKey = (queue_idx, msg._ref);
                let m = HttpMetrics {
                    active_sockets: msg.active_sockets as u64,
                    sum_total_time_ns: msg.sum_total_time_ns as u64,
                    sum_processing_time_ns: msg.sum_processing_time_ns as u64,
                    sum_code_200: msg.sum_code_200 as u64,
                    sum_code_400: msg.sum_code_400 as u64,
                    sum_code_500: msg.sum_code_500 as u64,
                    sum_code_other: msg.sum_code_other as u64,
                };
                let dir = if msg.direction == 0 {
                    Direction::AtoB
                } else {
                    Direction::BtoA
                };
                self.agg.borrow_mut().add_http(key, dir, m);
            }
            Err(_e) => self.agg.borrow_mut().events.inc_decode_error_update_http(),
        }
    }

    fn on_update_dns(&self, queue_idx: usize, buf: &[u8]) {
        match parsed_message::update_dns_metrics::decode(buf) {
            Ok(msg) => {
                let key: AggRootKey = (queue_idx, msg._ref);
                let m = DnsMetrics {
                    active_sockets: msg.active_sockets as u64,
                    sum_total_time_ns: msg.sum_total_time_ns as u64,
                    sum_processing_time_ns: msg.sum_processing_time_ns as u64,
                    requests_a: msg.requests_a as u64,
                    requests_aaaa: msg.requests_aaaa as u64,
                    responses: msg.responses as u64,
                    timeouts: msg.timeouts as u64,
                };
                let dir = if msg.direction == 0 {
                    Direction::AtoB
                } else {
                    Direction::BtoA
                };
                self.agg.borrow_mut().add_dns(key, dir, m);
            }
            Err(_e) => self.agg.borrow_mut().events.inc_decode_error_update_dns(),
        }
    }

    fn on_pulse(&self, buf: &[u8]) {
        match parsed_message::pulse::decode(buf) {
            Ok(_msg) => {}
            Err(_e) => self.agg.borrow_mut().events.inc_decode_error_pulse(),
        }
    }
}
