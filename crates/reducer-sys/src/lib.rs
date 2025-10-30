#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_aggregation;
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_ingest;
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_logging;
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_matching;
#[allow(unused_extern_crates)]
extern crate otlp_export;

#[cxx::bridge(namespace = "reducer_cfg")]
pub mod ffi {
    #[repr(u16)]
    #[derive(Debug, Clone, Copy)]
    pub enum TsdbFormat {
        prometheus = 1,
        json = 2,
        otlp_grpc = 3,
    }

    /// Final, CXX-friendly reducer configuration.
    /// Note: sentinel values encode optional fields:
    /// - size limits: 0 = unlimited
    /// - geoip_path: empty string = unset
    #[derive(Debug)]
    pub struct ReducerConfig {
        pub telemetry_port: u32,

        pub num_ingest_shards: u32,
        pub num_matching_shards: u32,
        pub num_aggregation_shards: u32,
        pub partitions_per_shard: u32,

        pub enable_id_id: bool,
        pub enable_az_id: bool,
        pub enable_flow_logs: bool,

        pub enable_otlp_grpc_metrics: bool,
        pub otlp_grpc_metrics_address: String,
        pub otlp_grpc_metrics_port: u32,
        pub otlp_grpc_batch_size: i32,
        pub enable_otlp_grpc_metric_descriptions: bool,

        pub disable_prometheus_metrics: bool,
        pub shard_prometheus_metrics: bool,
        pub prom_bind: String,
        pub scrape_size_limit_bytes: u64, // 0 => unlimited
        pub internal_prom_bind: String,
        pub stats_scrape_size_limit_bytes: u64, // 0 => unlimited
        pub scrape_metrics_tsdb_format: TsdbFormat,

        pub disable_node_ip_field: bool,
        pub enable_autonomous_system_ip: bool,

        pub geoip_path: String, // empty => unset

        pub enable_aws_enrichment: bool,
        pub enable_percentile_latencies: bool,

        pub disable_metrics: String,
        pub enable_metrics: String,

        pub index_dump_interval: u64,

        // Logging whitelist controls
        pub log_whitelist_all: bool,
        pub log_whitelist_client_type: String,
        pub log_whitelist_node_resolution_type: String,
        pub log_whitelist_channel: String,
        pub log_whitelist_ingest: String,
        pub log_whitelist_matching: String,
    }

    extern "C++" {

        include!("reducer/entrypoint.h");

        // Implemented in C++ (entrypoint.cc)
        unsafe fn otn_reducer_main_with_config(cfg: &ReducerConfig) -> i32;

        // Logging controls (implemented in C++)
        unsafe fn otn_init_logging(log_console: bool, no_log_file: bool);
        /// Level code mapping: 0=trace,1=debug,2=info,3=warning,4=error,5=critical
        unsafe fn otn_set_log_level(level_code: i32);
    }
}
