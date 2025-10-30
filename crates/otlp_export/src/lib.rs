#![allow(clippy::new_without_default)]

#[cxx::bridge]
pub mod ffi {
    /// Lightweight key/value label representation.
    #[derive(Debug, Clone)]
    pub struct Label {
        pub key: String,
        pub value: String,
    }

    /// Statistics mirroring the counters tracked by the C++ OTLP client.
    #[derive(Debug)]
    pub struct PublisherStats {
        pub bytes_sent: u64,
        pub bytes_failed: u64,
        pub data_points_sent: u64,
        pub data_points_failed: u64,
        pub requests_sent: u64,
        pub requests_failed: u64,
        pub unknown_response_tags: u64,
    }

    /// Metric kind to encode.
    #[repr(i32)]
    #[derive(Debug)]
    pub enum MetricKind {
        Sum = 0,
        Gauge = 1,
    }

    extern "Rust" {
        type Publisher;

        /// Create a new OTLP publisher. `endpoint` is host:port or full endpoint string.
        fn otlp_publisher_new(endpoint: &str) -> Box<Publisher>;

        /// Publish a u64 metric point.
        fn publish_metric_u64(
            self: &mut Publisher,
            name: &str,
            unit: &str,
            description: &str,
            kind: MetricKind,
            labels: &Vec<Label>,
            timestamp_unix_nano: i64,
            value: u64,
        );

        /// Publish a f64 metric point.
        fn publish_metric_f64(
            self: &mut Publisher,
            name: &str,
            unit: &str,
            description: &str,
            kind: MetricKind,
            labels: &Vec<Label>,
            timestamp_unix_nano: i64,
            value: f64,
        );

        /// Publish a TCP flow log equivalent (kept minimal to match reducer fields).
        fn publish_flow_log(
            self: &mut Publisher,
            labels: &Vec<Label>,
            timestamp_unix_nano: i64,
            tcp_sum_bytes: u64,
            tcp_active_rtts: u32,
            tcp_active_sockets: u32,
            tcp_sum_srtt: u64,
            tcp_sum_delivered: u64,
            tcp_sum_retrans: u64,
            tcp_syn_timeouts: u64,
            tcp_new_sockets: u64,
            tcp_resets: u64,
        );

        /// Flush any in-flight batches and process responses.
        fn flush(self: &mut Publisher);

        /// Shut down the publisher.
        fn shutdown(self: &mut Publisher);

        /// Read current counters/statistics.
        fn stats(self: &Publisher) -> PublisherStats;
    }
}

use ffi::{Label, MetricKind, PublisherStats};
use opentelemetry_proto::tonic::collector::metrics::v1 as otlp_collector;
use opentelemetry_proto::tonic::collector::metrics::v1::metrics_service_client::MetricsServiceClient;
use opentelemetry_proto::tonic::common::v1 as otlp_common;
use opentelemetry_proto::tonic::metrics::v1 as otlp_metrics;
use opentelemetry_proto::tonic::resource::v1 as otlp_resource;
use tokio::runtime::Runtime;
use tonic::Request;

/// Minimal placeholder publisher. This crate defines the FFI surface and basic accounting.
/// Internals can be extended to perform real async OTLP export.
pub struct Publisher {
    endpoint: String,
    runtime: Runtime,
    resource_attributes: Vec<(String, String)>,
    scope_name: String,
    // Buffered metrics and logs to export on flush.
    buf: Vec<PendingMetric>,
    // Simple counters following the C++ client semantics.
    bytes_sent: u64,
    bytes_failed: u64,
    data_points_sent: u64,
    data_points_failed: u64,
    requests_sent: u64,
    requests_failed: u64,
    unknown_response_tags: u64,
    // Simple buffered counters to roll up into a "request" on flush.
    buffered_points: u64,
    buffered_bytes: u64,
}

enum PointValue {
    U64(u64),
    F64(f64),
}

struct PendingMetric {
    name: String,
    unit: String,
    description: String,
    kind: MetricKind,
    labels: Vec<Label>,
    timestamp_unix_nano: i64,
    value: PointValue,
}

pub fn otlp_publisher_new(endpoint: &str) -> Box<Publisher> {
    // Resolve endpoint: accept full URL or host:port and default to http with no path for gRPC.
    let endpoint_resolved = normalize_grpc_endpoint(endpoint);

    // Static resource/scope metadata for now; can be extended via FFI later.
    let resource_attributes = vec![
        ("service.name".to_string(), "reducer".to_string()),
        ("telemetry.sdk.language".to_string(), "rust".to_string()),
    ];
    let scope_name = "reducer-ffi".to_string();

    // Create a small Tokio runtime for async export.
    let runtime = Runtime::new().expect("failed to create Tokio runtime");

    Box::new(Publisher {
        endpoint: endpoint_resolved,
        runtime,
        resource_attributes,
        scope_name,
        buf: Vec::new(),
        bytes_sent: 0,
        bytes_failed: 0,
        data_points_sent: 0,
        data_points_failed: 0,
        requests_sent: 0,
        requests_failed: 0,
        unknown_response_tags: 0,
        buffered_points: 0,
        buffered_bytes: 0,
    })
}

impl Publisher {
    pub fn publish_metric_u64(
        &mut self,
        name: &str,
        unit: &str,
        description: &str,
        kind: MetricKind,
        labels: &Vec<Label>,
        timestamp_unix_nano: i64,
        value: u64,
    ) {
        self.buf.push(PendingMetric {
            name: name.to_string(),
            unit: unit.to_string(),
            description: description.to_string(),
            kind,
            labels: labels
                .iter()
                .map(|l| Label {
                    key: l.key.clone(),
                    value: l.value.clone(),
                })
                .collect(),
            timestamp_unix_nano,
            value: PointValue::U64(value),
        });

        let approx = approx_bytes_metric(name, labels);
        self.buffered_points += 1;
        self.buffered_bytes = self.buffered_bytes.saturating_add(approx);
    }

    pub fn publish_metric_f64(
        &mut self,
        name: &str,
        unit: &str,
        description: &str,
        kind: MetricKind,
        labels: &Vec<Label>,
        timestamp_unix_nano: i64,
        value: f64,
    ) {
        self.buf.push(PendingMetric {
            name: name.to_string(),
            unit: unit.to_string(),
            description: description.to_string(),
            kind,
            labels: labels
                .iter()
                .map(|l| Label {
                    key: l.key.clone(),
                    value: l.value.clone(),
                })
                .collect(),
            timestamp_unix_nano,
            value: PointValue::F64(value),
        });

        let approx = approx_bytes_metric(name, labels);
        self.buffered_points += 1;
        self.buffered_bytes = self.buffered_bytes.saturating_add(approx);
    }
}

impl Publisher {
    pub fn publish_flow_log(
        &mut self,
        _labels: &Vec<Label>,
        _timestamp_unix_nano: i64,
        _tcp_sum_bytes: u64,
        _tcp_active_rtts: u32,
        _tcp_active_sockets: u32,
        _tcp_sum_srtt: u64,
        _tcp_sum_delivered: u64,
        _tcp_sum_retrans: u64,
        _tcp_syn_timeouts: u64,
        _tcp_new_sockets: u64,
        _tcp_resets: u64,
    ) {
        // Future: publish logs via opentelemetry logs SDK. For now, account as one data point.
        self.buffered_points += 1;
        // Approximate fixed size for a log line.
        self.buffered_bytes = self.buffered_bytes.saturating_add(128);
    }

    pub fn flush(&mut self) {
        if self.buf.is_empty() {
            return;
        }

        // Build OTLP ResourceMetrics -> ScopeMetrics -> Metric with one datapoint each.
        let mut metrics: Vec<otlp_metrics::Metric> = Vec::with_capacity(self.buf.len());
        for pm in &self.buf {
            let attrs = labels_to_otlp_kv(&pm.labels);

            // For sums, set start_time to slot start (30s window) to match reducer semantics.
            let time_unix_nano = pm.timestamp_unix_nano as u64;
            let start_time_unix_nano = pm.timestamp_unix_nano.saturating_sub(30_000_000_000) as u64;

            let metric = match pm.kind {
                MetricKind::Sum => {
                    let ndp = match pm.value {
                        PointValue::U64(v) => otlp_metrics::NumberDataPoint {
                            attributes: attrs,
                            start_time_unix_nano,
                            time_unix_nano,
                            exemplars: vec![],
                            flags: 0,
                            value: Some(otlp_metrics::number_data_point::Value::AsInt(
                                saturating_u64_to_i64(v),
                            )),
                        },
                        PointValue::F64(v) => otlp_metrics::NumberDataPoint {
                            attributes: attrs,
                            start_time_unix_nano,
                            time_unix_nano,
                            exemplars: vec![],
                            flags: 0,
                            value: Some(otlp_metrics::number_data_point::Value::AsDouble(v)),
                        },
                    };

                    let sum = otlp_metrics::Sum {
                        data_points: vec![ndp],
                        aggregation_temporality: otlp_metrics::AggregationTemporality::Delta as i32,
                        is_monotonic: true,
                    };

                    otlp_metrics::Metric {
                        name: pm.name.clone(),
                        description: pm.description.clone(),
                        unit: pm.unit.clone(),
                        metadata: vec![],
                        data: Some(otlp_metrics::metric::Data::Sum(sum)),
                    }
                }
                MetricKind::Gauge => {
                    let ndp = match pm.value {
                        PointValue::U64(v) => otlp_metrics::NumberDataPoint {
                            attributes: attrs,
                            start_time_unix_nano: 0, // ignored for Gauge
                            time_unix_nano,
                            exemplars: vec![],
                            flags: 0,
                            value: Some(otlp_metrics::number_data_point::Value::AsInt(
                                saturating_u64_to_i64(v),
                            )),
                        },
                        PointValue::F64(v) => otlp_metrics::NumberDataPoint {
                            attributes: attrs,
                            start_time_unix_nano: 0,
                            time_unix_nano,
                            exemplars: vec![],
                            flags: 0,
                            value: Some(otlp_metrics::number_data_point::Value::AsDouble(v)),
                        },
                    };
                    let gauge = otlp_metrics::Gauge {
                        data_points: vec![ndp],
                    };
                    otlp_metrics::Metric {
                        name: pm.name.clone(),
                        description: pm.description.clone(),
                        unit: pm.unit.clone(),
                        metadata: vec![],
                        data: Some(otlp_metrics::metric::Data::Gauge(gauge)),
                    }
                }
                _ => {
                    let ndp = match pm.value {
                        PointValue::U64(v) => otlp_metrics::NumberDataPoint {
                            attributes: attrs,
                            start_time_unix_nano: 0, // ignored for Gauge
                            time_unix_nano,
                            exemplars: vec![],
                            flags: 0,
                            value: Some(otlp_metrics::number_data_point::Value::AsInt(
                                saturating_u64_to_i64(v),
                            )),
                        },
                        PointValue::F64(v) => otlp_metrics::NumberDataPoint {
                            attributes: attrs,
                            start_time_unix_nano: 0,
                            time_unix_nano,
                            exemplars: vec![],
                            flags: 0,
                            value: Some(otlp_metrics::number_data_point::Value::AsDouble(v)),
                        },
                    };
                    let gauge = otlp_metrics::Gauge {
                        data_points: vec![ndp],
                    };
                    otlp_metrics::Metric {
                        name: pm.name.clone(),
                        description: pm.description.clone(),
                        unit: pm.unit.clone(),
                        metadata: vec![],
                        data: Some(otlp_metrics::metric::Data::Gauge(gauge)),
                    }
                }
            };

            metrics.push(metric);
        }

        let scope_metrics = otlp_metrics::ScopeMetrics {
            scope: Some(otlp_common::InstrumentationScope {
                name: self.scope_name.clone(),
                version: String::new(),
                attributes: vec![],
                dropped_attributes_count: 0,
            }),
            metrics,
            schema_url: String::new(),
        };

        let resource = otlp_resource::Resource {
            attributes: self
                .resource_attributes
                .iter()
                .map(|(k, v)| otlp_common::KeyValue {
                    key: k.clone(),
                    value: Some(otlp_common::AnyValue {
                        value: Some(otlp_common::any_value::Value::StringValue(v.clone())),
                    }),
                })
                .collect(),
            dropped_attributes_count: 0,
            entity_refs: vec![],
        };

        let rm = otlp_metrics::ResourceMetrics {
            resource: Some(resource),
            scope_metrics: vec![scope_metrics],
            schema_url: String::new(),
        };

        let req = otlp_collector::ExportMetricsServiceRequest {
            resource_metrics: vec![rm],
        };

        // Send synchronously on our runtime.
        let endpoint = self.endpoint.clone();
        let buffered_points = self.buffered_points;
        let buffered_bytes = self.buffered_bytes;
        let export_res = self.runtime.block_on(async move {
            match MetricsServiceClient::connect(endpoint).await {
                Ok(mut client) => client.export(Request::new(req)).await,
                Err(e) => Err(tonic::Status::unknown(format!("connect error: {}", e))),
            }
        });

        match export_res {
            Ok(resp) => {
                self.requests_sent = self.requests_sent.saturating_add(1);
                let mut accepted = buffered_points;
                if let Some(ps) = resp.into_inner().partial_success {
                    if ps.rejected_data_points > 0 {
                        let rej = ps.rejected_data_points as u64;
                        let acc = accepted.saturating_sub(rej);
                        self.data_points_failed = self.data_points_failed.saturating_add(rej);
                        accepted = acc;
                    }
                }
                self.data_points_sent = self.data_points_sent.saturating_add(accepted);
                self.bytes_sent = self.bytes_sent.saturating_add(buffered_bytes);
            }
            Err(_e) => {
                self.requests_failed = self.requests_failed.saturating_add(1);
                self.data_points_failed = self.data_points_failed.saturating_add(buffered_points);
                self.bytes_failed = self.bytes_failed.saturating_add(buffered_bytes);
            }
        }

        self.buf.clear();
        self.buffered_points = 0;
        self.buffered_bytes = 0;
    }

    pub fn shutdown(&mut self) {
        self.flush();
        // Nothing else to shutdown for the exporter.
    }

    pub fn stats(&self) -> PublisherStats {
        PublisherStats {
            bytes_sent: self.bytes_sent,
            bytes_failed: self.bytes_failed,
            data_points_sent: self.data_points_sent,
            data_points_failed: self.data_points_failed,
            requests_sent: self.requests_sent,
            requests_failed: self.requests_failed,
            unknown_response_tags: self.unknown_response_tags,
        }
    }
}

fn approx_bytes_metric(name: &str, labels: &Vec<Label>) -> u64 {
    let mut n = name.len() as u64;
    for kv in labels {
        n = n.saturating_add(kv.key.len() as u64 + kv.value.len() as u64 + 2);
    }
    n
}

fn labels_to_otlp_kv(labels: &Vec<Label>) -> Vec<otlp_common::KeyValue> {
    labels
        .iter()
        .map(|l| otlp_common::KeyValue {
            key: l.key.clone(),
            value: Some(otlp_common::AnyValue {
                value: Some(otlp_common::any_value::Value::StringValue(l.value.clone())),
            }),
        })
        .collect()
}

fn normalize_grpc_endpoint(input: &str) -> String {
    // Do not append a path. gRPC expects a host:port with scheme.
    if input.starts_with("http://") || input.starts_with("https://") {
        input.to_string()
    } else if input.contains("://") {
        input.to_string()
    } else {
        format!("http://{}", input)
    }
}

fn saturating_u64_to_i64(v: u64) -> i64 {
    if v > i64::MAX as u64 {
        i64::MAX
    } else {
        v as i64
    }
}
