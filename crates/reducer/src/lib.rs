#![allow(clippy::too_many_arguments)]

use clap::Parser;

use reducer_sys::ffi::{ReducerConfig as FfiReducerConfig, TsdbFormat};

mod aggregation_core;
pub mod aggregation_framework;
mod aggregation_message_handler;
mod aggregator;
pub mod ffi;
mod internal_events;
mod metrics;
mod otlp_encoding;
mod queue_handler;

#[derive(Parser, Debug)]
#[command(name = "reducer", about = "OpenTelemetry eBPF Reducer")]
struct Cli {
    // Logging configuration
    /// Do not write logs to a file
    #[arg(long = "no-log-file")]
    no_log_file: bool,
    /// Log to console (stdout)
    #[arg(long = "log-console", alias = "console-log")]
    log_console: bool,
    /// Set minimum log level to trace
    #[arg(long = "trace")]
    trace: bool,
    /// Set minimum log level to debug
    #[arg(long = "debug")]
    debug: bool,
    /// Set minimum log level to info
    #[arg(long = "info")]
    info: bool,
    /// Set minimum log level to warning
    #[arg(long = "warning")]
    warning: bool,
    /// Set minimum log level to error
    #[arg(long = "error")]
    error: bool,
    /// Set minimum log level to critical
    #[arg(long = "critical")]
    critical: bool,

    /// Print configuration values and exit
    #[arg(long = "print-config")]
    print_config: bool,

    /// TCP port to listen on for incoming connections from collectors
    #[arg(short = 'p', long = "port")]
    port: Option<u32>,

    /// Format of TSDB data for scraped metrics (prometheus|json)
    #[arg(long = "metrics-tsdb-format", default_value = "prometheus")]
    metrics_tsdb_format: String,

    // Features
    #[arg(long = "enable-aws-enrichment")]
    enable_aws_enrichment: bool,
    #[arg(long = "disable-node-ip-field")]
    disable_node_ip_field: bool,
    #[arg(long = "enable-id-id")]
    enable_id_id: bool,
    #[arg(long = "enable-az-id")]
    enable_az_id: bool,
    #[arg(long = "enable-flow-logs")]
    enable_flow_logs: bool,
    #[arg(long = "enable-autonomous-system-ip")]
    enable_autonomous_system_ip: bool,
    #[arg(long = "enable-percentile-latencies")]
    enable_percentile_latencies: bool,

    // Scaling
    #[arg(long = "num-ingest-shards")]
    num_ingest_shards: Option<u32>,
    #[arg(long = "num-matching-shards")]
    num_matching_shards: Option<u32>,
    #[arg(long = "num-aggregation-shards")]
    num_aggregation_shards: Option<u32>,
    #[arg(long = "partitions-per-shard")]
    partitions_per_shard: Option<u32>,

    // Prometheus output
    #[arg(long = "disable-prometheus-metrics")]
    disable_prometheus_metrics: bool,
    #[arg(long = "shard-prometheus-metrics")]
    shard_prometheus_metrics: bool,
    #[arg(long = "prom")]
    prom_bind: Option<String>,
    #[arg(long = "scrape-size-limit-bytes")]
    scrape_size_limit_bytes: Option<u64>,

    // OTLP gRPC output
    #[arg(long = "enable-otlp-grpc-metrics")]
    enable_otlp_grpc_metrics: bool,
    #[arg(long = "otlp-grpc-metrics-host", default_value = "localhost")]
    otlp_grpc_metrics_address: String,
    #[arg(long = "otlp-grpc-metrics-port")]
    otlp_grpc_metrics_port: Option<u32>,
    #[arg(long = "otlp-grpc-batch-size")]
    otlp_grpc_batch_size: Option<i32>,
    #[arg(long = "enable-otlp-grpc-metric-descriptions")]
    enable_otlp_grpc_metric_descriptions: bool,

    // Metrics output enable/disable
    #[arg(long = "disable-metrics")]
    disable_metrics: Option<String>,
    #[arg(long = "enable-metrics")]
    enable_metrics: Option<String>,

    // Internal stats
    #[arg(long = "internal-prom")]
    internal_prom_bind: Option<String>,
    #[arg(long = "stats-scrape-size-limit-bytes")]
    stats_scrape_size_limit_bytes: Option<u64>,

    // Logging and debugging
    #[arg(long = "index-dump-interval")]
    index_dump_interval: Option<u64>,

    // Whitelist controls
    /// Enable all logging whitelists (equivalent to '--log-whitelist-*=*')
    #[arg(long = "log-whitelist-all")]
    log_whitelist_all: bool,
    /// Comma-separated list for client-type whitelist
    #[arg(long = "log-whitelist-client-type")]
    log_whitelist_client_type: Option<String>,
    /// Comma-separated list for node-resolution-type whitelist
    #[arg(long = "log-whitelist-node-resolution-type")]
    log_whitelist_node_resolution_type: Option<String>,
    /// Comma-separated list for channel whitelist
    #[arg(long = "log-whitelist-channel")]
    log_whitelist_channel: Option<String>,
    /// Comma-separated list for ingest whitelist
    #[arg(long = "log-whitelist-ingest")]
    log_whitelist_ingest: Option<String>,
    /// Comma-separated list for matching whitelist
    #[arg(long = "log-whitelist-matching")]
    log_whitelist_matching: Option<String>,
}

fn default_config() -> FfiReducerConfig {
    FfiReducerConfig {
        telemetry_port: 8000,

        num_ingest_shards: 1,
        num_matching_shards: 1,
        num_aggregation_shards: 1,
        partitions_per_shard: 1,

        enable_id_id: false,
        enable_az_id: false,
        enable_flow_logs: false,

        enable_otlp_grpc_metrics: false,
        otlp_grpc_metrics_address: "localhost".into(),
        otlp_grpc_metrics_port: 4317,
        otlp_grpc_batch_size: 1000,
        enable_otlp_grpc_metric_descriptions: false,

        disable_prometheus_metrics: false,
        shard_prometheus_metrics: false,
        prom_bind: "127.0.0.1:7010".into(),
        scrape_size_limit_bytes: 0,
        internal_prom_bind: "0.0.0.0:7001".into(),
        stats_scrape_size_limit_bytes: 0,
        scrape_metrics_tsdb_format: TsdbFormat::prometheus,

        disable_node_ip_field: false,
        enable_autonomous_system_ip: false,

        geoip_path: String::new(),

        enable_aws_enrichment: false,
        enable_percentile_latencies: false,

        disable_metrics: String::new(),
        enable_metrics: String::new(),

        index_dump_interval: 0,

        log_whitelist_all: false,
        log_whitelist_client_type: String::new(),
        log_whitelist_node_resolution_type: String::new(),
        log_whitelist_channel: String::new(),
        log_whitelist_ingest: String::new(),
        log_whitelist_matching: String::new(),
    }
}

fn parse_tsdb_format(s: &str) -> Result<TsdbFormat, String> {
    match s.to_ascii_lowercase().as_str() {
        "prometheus" => Ok(TsdbFormat::prometheus),
        "json" => Ok(TsdbFormat::json),
        other => Err(format!(
            "Invalid TSDB format for scraped metrics: {}. Supported formats: prometheus, json",
            other
        )),
    }
}

fn build_final_config(cli: &Cli) -> Result<FfiReducerConfig, String> {
    let mut cfg = default_config();

    if let Some(v) = cli.port {
        cfg.telemetry_port = v;
    }

    if let Some(v) = cli.num_ingest_shards {
        cfg.num_ingest_shards = v;
    }
    if let Some(v) = cli.num_matching_shards {
        cfg.num_matching_shards = v;
    }
    if let Some(v) = cli.num_aggregation_shards {
        cfg.num_aggregation_shards = v;
    }
    if let Some(v) = cli.partitions_per_shard {
        cfg.partitions_per_shard = v;
    }

    cfg.enable_id_id |= cli.enable_id_id;
    cfg.enable_az_id |= cli.enable_az_id;
    cfg.enable_flow_logs |= cli.enable_flow_logs;

    cfg.enable_otlp_grpc_metrics |= cli.enable_otlp_grpc_metrics;
    cfg.otlp_grpc_metrics_address = cli.otlp_grpc_metrics_address.clone();
    if let Some(v) = cli.otlp_grpc_metrics_port {
        cfg.otlp_grpc_metrics_port = v;
    }
    if let Some(v) = cli.otlp_grpc_batch_size {
        cfg.otlp_grpc_batch_size = v;
    }
    cfg.enable_otlp_grpc_metric_descriptions |= cli.enable_otlp_grpc_metric_descriptions;

    cfg.disable_prometheus_metrics |= cli.disable_prometheus_metrics;
    cfg.shard_prometheus_metrics |= cli.shard_prometheus_metrics;
    if let Some(v) = &cli.prom_bind {
        cfg.prom_bind = v.clone();
    }
    if let Some(v) = cli.scrape_size_limit_bytes {
        cfg.scrape_size_limit_bytes = v;
    }
    if let Some(v) = &cli.internal_prom_bind {
        cfg.internal_prom_bind = v.clone();
    }

    // stats limit mirrors scrape limit when not provided
    cfg.stats_scrape_size_limit_bytes = match cli.stats_scrape_size_limit_bytes {
        Some(v) => v,
        None => cfg.scrape_size_limit_bytes,
    };

    cfg.scrape_metrics_tsdb_format = parse_tsdb_format(&cli.metrics_tsdb_format)?;

    cfg.disable_node_ip_field |= cli.disable_node_ip_field;
    cfg.enable_autonomous_system_ip |= cli.enable_autonomous_system_ip;

    // geoip from env, empty => unset
    if let Ok(val) = std::env::var("GEOIP_PATH") {
        if !val.is_empty() {
            cfg.geoip_path = val;
        }
    }

    cfg.enable_aws_enrichment |= cli.enable_aws_enrichment;
    cfg.enable_percentile_latencies |= cli.enable_percentile_latencies;

    if let Some(v) = &cli.disable_metrics {
        cfg.disable_metrics = v.clone();
    }
    if let Some(v) = &cli.enable_metrics {
        cfg.enable_metrics = v.clone();
    }

    if let Some(v) = cli.index_dump_interval {
        cfg.index_dump_interval = v;
    }

    // Logging whitelist pass-through (strings and all-flag)
    cfg.log_whitelist_all |= cli.log_whitelist_all;
    if let Some(v) = &cli.log_whitelist_client_type {
        cfg.log_whitelist_client_type = v.clone();
    }
    if let Some(v) = &cli.log_whitelist_node_resolution_type {
        cfg.log_whitelist_node_resolution_type = v.clone();
    }
    if let Some(v) = &cli.log_whitelist_channel {
        cfg.log_whitelist_channel = v.clone();
    }
    if let Some(v) = &cli.log_whitelist_ingest {
        cfg.log_whitelist_ingest = v.clone();
    }
    if let Some(v) = &cli.log_whitelist_matching {
        cfg.log_whitelist_matching = v.clone();
    }

    Ok(cfg)
}

fn to_string_tsdb(fmt: TsdbFormat) -> &'static str {
    match fmt {
        TsdbFormat::prometheus => "prometheus",
        TsdbFormat::json => "json",
        TsdbFormat::otlp_grpc => "otlp_grpc",
        // cxx::bridge enums are repr types; be exhaustive defensively
        _ => "prometheus",
    }
}

fn print_config(cfg: &FfiReducerConfig) {
    // Mirror reducer/reducer_config.inl printing semantics
    println!("telemetry_port: {}", cfg.telemetry_port);
    println!("num_ingest_shards: {}", cfg.num_ingest_shards);
    println!("num_matching_shards: {}", cfg.num_matching_shards);
    println!("num_aggregation_shards: {}", cfg.num_aggregation_shards);
    println!("partitions_per_shard: {}", cfg.partitions_per_shard);
    println!("enable_id_id: {}", cfg.enable_id_id);
    println!("enable_az_id: {}", cfg.enable_az_id);
    println!("enable_flow_logs: {}", cfg.enable_flow_logs);
    println!("enable_otlp_grpc_metrics: {}", cfg.enable_otlp_grpc_metrics);
    println!(
        "otlp_grpc_metrics_address: {}",
        cfg.otlp_grpc_metrics_address
    );
    println!("otlp_grpc_metrics_port: {}", cfg.otlp_grpc_metrics_port);
    println!("otlp_grpc_batch_size: {}", cfg.otlp_grpc_batch_size);
    println!(
        "enable_otlp_grpc_metric_descriptions: {}",
        cfg.enable_otlp_grpc_metric_descriptions
    );
    println!(
        "disable_prometheus_metrics: {}",
        cfg.disable_prometheus_metrics
    );
    println!("shard_prometheus_metrics: {}", cfg.shard_prometheus_metrics);
    println!("prom_bind: {}", cfg.prom_bind);
    println!("internal_prom_bind: {}", cfg.internal_prom_bind);

    if cfg.scrape_size_limit_bytes == 0 {
        println!("scrape_size_limit_bytes: unlimited");
    } else {
        println!("scrape_size_limit_bytes: {}", cfg.scrape_size_limit_bytes);
    }

    if cfg.stats_scrape_size_limit_bytes == 0 {
        println!("stats_scrape_size_limit_bytes: unlimited");
    } else {
        println!(
            "stats_scrape_size_limit_bytes: {}",
            cfg.stats_scrape_size_limit_bytes
        );
    }

    println!(
        "scrape_metrics_tsdb_format: {}",
        to_string_tsdb(cfg.scrape_metrics_tsdb_format)
    );
    println!("disable_node_ip_field: {}", cfg.disable_node_ip_field);
    println!(
        "enable_autonomous_system_ip: {}",
        cfg.enable_autonomous_system_ip
    );
    println!(
        "geoip_path: {}",
        if cfg.geoip_path.is_empty() {
            "none"
        } else {
            &cfg.geoip_path
        }
    );
    println!("enable_aws_enrichment: {}", cfg.enable_aws_enrichment);
    println!(
        "enable_percentile_latencies: {}",
        cfg.enable_percentile_latencies
    );
    println!("disable_metrics: {}", cfg.disable_metrics);
    println!("enable_metrics: {}", cfg.enable_metrics);
    println!("index_dump_interval: {}", cfg.index_dump_interval);
}

pub fn run_with_env_args() -> i32 {
    let cli = Cli::parse();

    // Initialize logging similar to legacy C++ ArgsParser handler
    unsafe {
        reducer_sys::ffi::otn_init_logging(cli.log_console, cli.no_log_file);
        // Apply explicit level only if any of the level flags are set
        let mut set_level = None;
        if cli.trace {
            set_level = Some(0);
        } else if cli.debug {
            set_level = Some(1);
        } else if cli.info {
            set_level = Some(2);
        } else if cli.warning {
            set_level = Some(3);
        } else if cli.error {
            set_level = Some(4);
        } else if cli.critical {
            set_level = Some(5);
        }
        if let Some(code) = set_level {
            reducer_sys::ffi::otn_set_log_level(code);
        }
    }

    let cfg = match build_final_config(&cli) {
        Ok(c) => c,
        Err(e) => {
            eprintln!("{}", e);
            return 1;
        }
    };

    if cli.print_config {
        print_config(&cfg);
        return 0;
    }

    unsafe { reducer_sys::ffi::otn_reducer_main_with_config(&cfg) }
}
