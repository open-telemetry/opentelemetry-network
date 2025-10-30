//! OTLP encoding helpers (stubbed in this iteration).

use crate::aggregator::{AllMetrics, Az, Node};
use crate::metrics::{DnsMetrics, HttpMetrics, TcpMetrics, UdpMetrics};
use otlp_export::ffi::{Label as OLabel, MetricKind, PublisherStats};
use otlp_export::{otlp_publisher_new, Publisher};

// Canonical metric descriptions (parity with C++ MetricInfo).
const DESC_TCP_BYTES: &str =
    "The total number of TCP bytes between the source and destination measured for the prior thirty seconds.";
const DESC_TCP_RTT_NUM: &str =
    "The number of measurements made in calculating the current RTT average value.";
const DESC_TCP_ACTIVE: &str =
    "The number of TCP connections considered to be open and alive between the source and destination at the point the measurement was taken.";
const DESC_TCP_RTT_AVG: &str =
    "The computed average round trip time between the source and destination as measured in microseconds.";
const DESC_TCP_PACKETS: &str =
    "The total number of TCP packets between the source and destination measured for the prior thirty seconds.";
const DESC_TCP_RETRANS: &str =
    "The total number of TCP retransmission requests between the source and destination measured for the prior thirty seconds.";
const DESC_TCP_SYN_TIMEOUTS: &str =
    "The total number of TCP SYN timeouts between the source and destination measured for the prior thirty seconds.";
const DESC_TCP_NEW_SOCKETS: &str =
    "The total number of new TCP sockets opened between the source and destination measured for the prior thirty seconds.";
const DESC_TCP_RESETS: &str =
    "The total number of TCP resets sent between the source and destination measured for the prior thirty seconds.";

const DESC_UDP_BYTES: &str =
    "The total number of UDP bytes between the source and destination measured for the prior thirty seconds.";
const DESC_UDP_PACKETS: &str =
    "The total number of UDP packets between the source and destination measured for the prior thirty seconds.";
const DESC_UDP_ACTIVE: &str =
    "The number of UDP connections considered to be open and alive between the source and destination at the point the measurement was taken.";
const DESC_UDP_DROPS: &str =
    "The total number of UDP connections dropped between the source and destination measured for the prior thirty seconds.";

const DESC_DNS_CLIENT_AVG: &str =
    "This metric is the average duration in microseconds from when the client sends a DNS request, until the response is received back from the server. As such, it includes the communication round-trip times, plus the server processing latency. Computed by the summation of all times, divided by dns.responses.";
const DESC_DNS_SERVER_AVG: &str =
    "This metric is the average duration in microseconds for the server to respond to a request received locally. Thus, it does not include the network latency from or to the client. Computed by the summation of all times, divided by dns.responses.";
const DESC_DNS_ACTIVE: &str =
    "The number of DNS connections for which measurements were taken in the prior thirty seconds.";
const DESC_DNS_RESPONSES: &str =
    "The total number of DNS responses sent between the source and destination measured for the prior thirty seconds.";
const DESC_DNS_TIMEOUTS: &str =
    "The total number of DNS timeouts between the source and destination measured for the prior thirty seconds.";

const DESC_HTTP_CLIENT_AVG: &str =
    "This metric is the average duration in microseconds from when the client sends an HTTP request, until the response is received back from the server. As such, it includes the communication round-trip times, plus the server processing latency. Computed by summation of all times, divided by http.active_sockets.";
const DESC_HTTP_SERVER_AVG: &str =
    "This metric is the average duration in microseconds for the server to respond to a request received locally. Thus, it does not include the network latency from or to the client. Computed by summation of all times, divided by http.active_sockets.";
const DESC_HTTP_ACTIVE: &str =
    "The number of unencrypted HTTPv1 connections for which measurements were taken in the prior thirty seconds.";
const DESC_HTTP_STATUS: &str =
    "For a given class of response code (see 'response_code' dimension), the number of times an unencrypted server sent an HTTPv1 status code between the source and destination measured for the prior thirty seconds.";

// Emit-ready labels vector; build these in aggregator to avoid per-call conversions.
pub type Labels = Vec<OLabel>;
// Map numeric node_type to C++ NodeResolutionType strings.
pub fn resolution_type_string(node_type: u8) -> &'static str {
    match node_type {
        0 => "NONE",
        1 => "IP",
        2 => "DNS",
        3 => "AWS",
        4 => "INSTANCE_METADATA",
        5 => "PROCESS",
        6 => "LOCALHOST",
        7 => "K8S_CONTAINER",
        8 => "CONTAINER",
        9 => "NOMAD",
        _ => "",
    }
}

// Build "source." or "dest." labels union for a Node+Az pair (some fields may be missing).
pub fn add_node_labels(prefix: &str, node: Option<&Node>, az: Option<&Az>, out: &mut Labels) {
    let p = |k: &str| format!("{}{}", prefix, k);
    if let Some(az) = az {
        out.push(OLabel {
            key: p("workload.name"),
            value: az.role.clone(),
        });
        out.push(OLabel {
            key: p("workload.uid"),
            value: az.role_uid.clone(),
        });
        out.push(OLabel {
            key: p("availability_zone"),
            value: az.az.clone(),
        });
        out.push(OLabel {
            key: p("resolution_type"),
            value: resolution_type_string(az.node_type).to_string(),
        });
        out.push(OLabel {
            key: p("image_version"),
            value: az.version.clone(),
        });
        out.push(OLabel {
            key: p("environment"),
            value: az.env.clone(),
        });
        out.push(OLabel {
            key: p("namespace.name"),
            value: az.ns.clone(),
        });
        out.push(OLabel {
            key: p("process.name"),
            value: az.process.clone(),
        });
        out.push(OLabel {
            key: p("container.name"),
            value: az.container.clone(),
        });
    }
    if let Some(node) = node {
        out.push(OLabel {
            key: p("id"),
            value: node.id.clone(),
        });
        out.push(OLabel {
            key: p("ip"),
            value: node.address.clone(),
        });
        out.push(OLabel {
            key: p("pod"),
            value: node.pod_name.clone(),
        });
    }
}

pub const LABEL_SF_PRODUCT: &str = "sf_product";
pub const LABEL_SF_PRODUCT_VALUE: &str = "network-explorer";
pub const LABEL_AGGREGATION: &str = "aggregation";

// Minimal exporter stub for this iteration.
pub struct OtlpExporter {
    pub enable_descriptions: bool,
    pub endpoint: String,
    pub publisher: Box<Publisher>,
}

impl Default for OtlpExporter {
    fn default() -> Self {
        // Default to localhost:4317 as in default reducer config.
        let endpoint = "http://localhost:4317".to_string();
        let publisher = otlp_publisher_new(&endpoint);
        Self {
            enable_descriptions: false,
            endpoint,
            publisher,
        }
    }
}

impl OtlpExporter {
    pub fn with_endpoint(endpoint: String, enable_descriptions: bool) -> Self {
        let publisher = otlp_publisher_new(&endpoint);
        Self {
            enable_descriptions,
            endpoint,
            publisher,
        }
    }
    pub fn new_local(enable_descriptions: bool) -> Self {
        // Same endpoint as Default but allows control over descriptions at construction.
        let endpoint = "http://localhost:4317".to_string();
        let publisher = otlp_publisher_new(&endpoint);
        Self {
            enable_descriptions,
            endpoint,
            publisher,
        }
    }
    fn publish_u64(
        &mut self,
        name: &str,
        unit: &str,
        desc: &str,
        kind: MetricKind,
        labels: &Labels,
        ts_ns: i64,
        v: u64,
    ) {
        self.publisher.publish_metric_u64(
            name,
            unit,
            if self.enable_descriptions { desc } else { "" },
            kind,
            labels,
            ts_ns,
            v,
        );
    }
    fn publish_f64(
        &mut self,
        name: &str,
        unit: &str,
        desc: &str,
        kind: MetricKind,
        labels: &Labels,
        ts_ns: i64,
        v: f64,
    ) {
        self.publisher.publish_metric_f64(
            name,
            unit,
            if self.enable_descriptions { desc } else { "" },
            kind,
            labels,
            ts_ns,
            v,
        );
    }

    // New per-protocol emitters
    pub fn emit_tcp(&mut self, ts: i64, labels: &Labels, tcp: &TcpMetrics) {
        self.publish_u64(
            "tcp.bytes",
            "By",
            DESC_TCP_BYTES,
            MetricKind::Sum,
            labels,
            ts,
            tcp.sum_bytes,
        );
        self.publish_u64(
            "tcp.rtt.num_measurements",
            "1",
            DESC_TCP_RTT_NUM,
            MetricKind::Gauge,
            labels,
            ts,
            tcp.active_rtts,
        );
        self.publish_u64(
            "tcp.active",
            "1",
            DESC_TCP_ACTIVE,
            MetricKind::Gauge,
            labels,
            ts,
            tcp.active_sockets,
        );
        let avg_us = if tcp.active_rtts > 0 {
            (tcp.sum_srtt as f64) / 8.0 / 1_000_000.0 / (tcp.active_rtts as f64)
        } else {
            0.0
        };
        self.publish_f64(
            "tcp.rtt.average",
            "us",
            DESC_TCP_RTT_AVG,
            MetricKind::Gauge,
            labels,
            ts,
            avg_us,
        );
        self.publish_u64(
            "tcp.packets",
            "1",
            DESC_TCP_PACKETS,
            MetricKind::Sum,
            labels,
            ts,
            tcp.sum_delivered,
        );
        self.publish_u64(
            "tcp.retrans",
            "1",
            DESC_TCP_RETRANS,
            MetricKind::Sum,
            labels,
            ts,
            tcp.sum_retrans,
        );
        self.publish_u64(
            "tcp.syn_timeouts",
            "1",
            DESC_TCP_SYN_TIMEOUTS,
            MetricKind::Sum,
            labels,
            ts,
            tcp.syn_timeouts,
        );
        self.publish_u64(
            "tcp.new_sockets",
            "1",
            DESC_TCP_NEW_SOCKETS,
            MetricKind::Sum,
            labels,
            ts,
            tcp.new_sockets,
        );
        self.publish_u64(
            "tcp.resets",
            "1",
            DESC_TCP_RESETS,
            MetricKind::Sum,
            labels,
            ts,
            tcp.tcp_resets,
        );
    }

    pub fn emit_udp(&mut self, ts: i64, labels: &Labels, udp: &UdpMetrics) {
        self.publish_u64(
            "udp.bytes",
            "By",
            DESC_UDP_BYTES,
            MetricKind::Sum,
            labels,
            ts,
            udp.bytes,
        );
        self.publish_u64(
            "udp.packets",
            "1",
            DESC_UDP_PACKETS,
            MetricKind::Sum,
            labels,
            ts,
            udp.packets,
        );
        self.publish_u64(
            "udp.active",
            "1",
            DESC_UDP_ACTIVE,
            MetricKind::Gauge,
            labels,
            ts,
            udp.active_sockets,
        );
        self.publish_u64(
            "udp.drops",
            "1",
            DESC_UDP_DROPS,
            MetricKind::Sum,
            labels,
            ts,
            udp.drops,
        );
    }

    pub fn emit_dns(&mut self, ts: i64, labels: &Labels, dns: &DnsMetrics) {
        self.publish_u64(
            "dns.active_sockets",
            "1",
            DESC_DNS_ACTIVE,
            MetricKind::Gauge,
            labels,
            ts,
            dns.active_sockets,
        );
        self.publish_u64(
            "dns.responses",
            "1",
            DESC_DNS_RESPONSES,
            MetricKind::Sum,
            labels,
            ts,
            dns.responses,
        );
        let avg_client_us = if dns.responses > 0 {
            (dns.sum_total_time_ns as f64) / 1_000_000_000.0 / (dns.responses as f64)
        } else {
            0.0
        };
        let avg_server_us = if dns.responses > 0 {
            (dns.sum_processing_time_ns as f64) / 1_000_000_000.0 / (dns.responses as f64)
        } else {
            0.0
        };
        self.publish_f64(
            "dns.client.duration.average",
            "us",
            DESC_DNS_CLIENT_AVG,
            MetricKind::Gauge,
            labels,
            ts,
            avg_client_us,
        );
        self.publish_f64(
            "dns.server.duration.average",
            "us",
            DESC_DNS_SERVER_AVG,
            MetricKind::Gauge,
            labels,
            ts,
            avg_server_us,
        );
        self.publish_u64(
            "dns.timeouts",
            "1",
            DESC_DNS_TIMEOUTS,
            MetricKind::Sum,
            labels,
            ts,
            dns.timeouts,
        );
    }

    pub fn emit_http(&mut self, ts: i64, labels: &Labels, http: &HttpMetrics) {
        self.publish_u64(
            "http.active_sockets",
            "1",
            DESC_HTTP_ACTIVE,
            MetricKind::Gauge,
            labels,
            ts,
            http.active_sockets,
        );
        // Requests (sum of codes)
        let total = http.sum_code_200 + http.sum_code_400 + http.sum_code_500 + http.sum_code_other;
        self.publish_u64("http.requests", "1", "", MetricKind::Sum, labels, ts, total);
        // Average durations per request (0 when no requests)
        let avg_client_us = if total > 0 {
            (http.sum_total_time_ns as f64) / 1_000_000.0 / (total as f64)
        } else {
            0.0
        };
        let avg_server_us = if total > 0 {
            (http.sum_processing_time_ns as f64) / 1_000_000.0 / (total as f64)
        } else {
            0.0
        };
        self.publish_f64(
            "http.client.duration.average",
            "us",
            DESC_HTTP_CLIENT_AVG,
            MetricKind::Gauge,
            labels,
            ts,
            avg_client_us,
        );
        self.publish_f64(
            "http.server.duration.average",
            "us",
            DESC_HTTP_SERVER_AVG,
            MetricKind::Gauge,
            labels,
            ts,
            avg_server_us,
        );
        // Status codes with label
        let mut l2 = labels.clone();
        l2.push(OLabel {
            key: "status_code".to_string(),
            value: "200".to_string(),
        });
        self.publish_u64(
            "http.status_code",
            "1",
            DESC_HTTP_STATUS,
            MetricKind::Sum,
            &l2,
            ts,
            http.sum_code_200,
        );
        let mut l3 = labels.clone();
        l3.push(OLabel {
            key: "status_code".to_string(),
            value: "400".to_string(),
        });
        self.publish_u64(
            "http.status_code",
            "1",
            DESC_HTTP_STATUS,
            MetricKind::Sum,
            &l3,
            ts,
            http.sum_code_400,
        );
        let mut l4 = labels.clone();
        l4.push(OLabel {
            key: "status_code".to_string(),
            value: "500".to_string(),
        });
        self.publish_u64(
            "http.status_code",
            "1",
            DESC_HTTP_STATUS,
            MetricKind::Sum,
            &l4,
            ts,
            http.sum_code_500,
        );
        let mut l5 = labels.clone();
        l5.push(OLabel {
            key: "status_code".to_string(),
            value: "other".to_string(),
        });
        self.publish_u64(
            "http.status_code",
            "1",
            DESC_HTTP_STATUS,
            MetricKind::Sum,
            &l5,
            ts,
            http.sum_code_other,
        );
    }

    // Emit a full AllMetrics, gating per protocol using prev-window flags.
    pub fn emit_all_metrics(&mut self, ts: i64, labels: &Labels, m: &AllMetrics) {
        if !m.tcp.is_zero() || m.tcp_prev_window_had_samples {
            self.emit_tcp(ts, labels, &m.tcp);
        }
        if !m.udp.is_zero() || m.udp_prev_window_had_samples {
            self.emit_udp(ts, labels, &m.udp);
        }
        if !m.http.is_zero() || m.http_prev_window_had_samples {
            self.emit_http(ts, labels, &m.http);
        }
        if !m.dns.is_zero() || m.dns_prev_window_had_samples {
            self.emit_dns(ts, labels, &m.dns);
        }
    }

    pub fn flush(&mut self) {
        self.publisher.flush();
    }
    pub fn stats(&self) -> PublisherStats {
        self.publisher.stats()
    }
}
