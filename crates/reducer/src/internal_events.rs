//! Internal events and counters for exceptional conditions.

#[derive(Default, Debug, Clone)]
pub struct Counters {
    pub decode_error_agg_root_start: u64,
    pub decode_error_agg_root_end: u64,
    pub decode_error_update_node: u64,
    pub decode_error_update_tcp: u64,
    pub decode_error_update_udp: u64,
    pub decode_error_update_http: u64,
    pub decode_error_update_dns: u64,
    pub decode_error_pulse: u64,

    pub missing_root_for_metric: u64,
    pub metric_before_sides_resolved: u64,
}

impl Counters {
    pub fn inc_decode_error_agg_root_start(&mut self) {
        self.decode_error_agg_root_start += 1;
    }
    pub fn inc_decode_error_agg_root_end(&mut self) {
        self.decode_error_agg_root_end += 1;
    }
    pub fn inc_decode_error_update_node(&mut self) {
        self.decode_error_update_node += 1;
    }
    pub fn inc_decode_error_update_tcp(&mut self) {
        self.decode_error_update_tcp += 1;
    }
    pub fn inc_decode_error_update_udp(&mut self) {
        self.decode_error_update_udp += 1;
    }
    pub fn inc_decode_error_update_http(&mut self) {
        self.decode_error_update_http += 1;
    }
    pub fn inc_decode_error_update_dns(&mut self) {
        self.decode_error_update_dns += 1;
    }
    pub fn inc_decode_error_pulse(&mut self) {
        self.decode_error_pulse += 1;
    }

    pub fn inc_missing_root_for_metric(&mut self) {
        self.missing_root_for_metric += 1;
    }
    pub fn inc_metric_before_sides_resolved(&mut self) {
        self.metric_before_sides_resolved += 1;
    }
}
