//! Protocol metric structs and simple fold helpers.

#[derive(Default, Debug, Clone, PartialEq, Eq)]
pub struct TcpMetrics {
    pub active_sockets: u64,
    pub sum_bytes: u64,
    pub sum_srtt: u64,
    pub sum_delivered: u64,
    pub sum_retrans: u64,
    pub active_rtts: u64,
    pub syn_timeouts: u64,
    pub new_sockets: u64,
    pub tcp_resets: u64,
}

impl TcpMetrics {
    pub fn add_from(&mut self, other: &Self) {
        self.active_sockets += other.active_sockets;
        self.sum_bytes += other.sum_bytes;
        self.sum_srtt += other.sum_srtt;
        self.sum_delivered += other.sum_delivered;
        self.sum_retrans += other.sum_retrans;
        self.active_rtts += other.active_rtts;
        self.syn_timeouts += other.syn_timeouts;
        self.new_sockets += other.new_sockets;
        self.tcp_resets += other.tcp_resets;
    }
    pub fn is_zero(&self) -> bool {
        // Consider only whether there were any samples this window
        self.active_sockets == 0
    }
}

#[derive(Default, Debug, Clone, PartialEq, Eq)]
pub struct UdpMetrics {
    pub active_sockets: u64,
    pub bytes: u64,
    pub addr_changes: u64,
    pub packets: u64,
    pub drops: u64,
}

impl UdpMetrics {
    pub fn add_from(&mut self, other: &Self) {
        self.active_sockets += other.active_sockets;
        self.bytes += other.bytes;
        self.addr_changes += other.addr_changes;
        self.packets += other.packets;
        self.drops += other.drops;
    }
    pub fn is_zero(&self) -> bool {
        self.active_sockets == 0
    }
}

#[derive(Default, Debug, Clone, PartialEq, Eq)]
pub struct HttpMetrics {
    pub active_sockets: u64,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
    pub sum_code_200: u64,
    pub sum_code_400: u64,
    pub sum_code_500: u64,
    pub sum_code_other: u64,
}

impl HttpMetrics {
    pub fn add_from(&mut self, other: &Self) {
        self.active_sockets += other.active_sockets;
        self.sum_total_time_ns += other.sum_total_time_ns;
        self.sum_processing_time_ns += other.sum_processing_time_ns;
        self.sum_code_200 += other.sum_code_200;
        self.sum_code_400 += other.sum_code_400;
        self.sum_code_500 += other.sum_code_500;
        self.sum_code_other += other.sum_code_other;
    }
    pub fn is_zero(&self) -> bool {
        self.active_sockets == 0
    }
}

#[derive(Default, Debug, Clone, PartialEq, Eq)]
pub struct DnsMetrics {
    pub active_sockets: u64,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
    pub requests_a: u64,
    pub requests_aaaa: u64,
    pub responses: u64,
    pub timeouts: u64,
}

impl DnsMetrics {
    pub fn add_from(&mut self, other: &Self) {
        self.active_sockets += other.active_sockets;
        self.sum_total_time_ns += other.sum_total_time_ns;
        self.sum_processing_time_ns += other.sum_processing_time_ns;
        self.requests_a += other.requests_a;
        self.requests_aaaa += other.requests_aaaa;
        self.responses += other.responses;
        self.timeouts += other.timeouts;
    }
    pub fn is_zero(&self) -> bool {
        self.active_sockets == 0
    }
}
