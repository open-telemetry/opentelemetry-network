//! CLI entrypoint for the Kubernetes metadata collector.
//!
//! Parses configuration from flags and environment variables, then executes
//! the collector pipeline.

use clap::Parser;
use k8s_collector::config::Config;

#[derive(Parser, Debug)]
#[command(
    name = "k8s-collector",
    about = "OpenTelemetry eBPF Kubernetes Metadata Collector (Rust)"
)]
struct Cli {
    /// Print configuration values and exit
    #[arg(long = "print-config")]
    print_config: bool,

    /// Deletion tombstone TTL in seconds
    #[arg(long = "delete-ttl-secs", default_value_t = 60)]
    delete_ttl_secs: u64,
    /// Deletion tombstone capacity
    #[arg(long = "delete-capacity", default_value_t = 10_000)]
    delete_capacity: usize,

    /// Reducer intake host
    #[arg(long = "intake-host")]
    intake_host: Option<String>,
    /// Reducer intake port
    #[arg(long = "intake-port")]
    intake_port: Option<u16>,
}

/// Build the final [`Config`] from CLI flags and environment variables.
fn build_config(cli: &Cli) -> Result<Config, String> {
    // Allow overriding the TTL via env if present
    let ttl_secs: u64 = std::env::var("K8S_COLLECTOR_DELETE_TTL_SECS")
        .ok()
        .and_then(|v| v.parse().ok())
        .unwrap_or(cli.delete_ttl_secs);
    let ttl = std::time::Duration::from_secs(ttl_secs);
    let cap: usize = std::env::var("K8S_COLLECTOR_DELETE_CAPACITY")
        .ok()
        .and_then(|v| v.parse().ok())
        .unwrap_or(cli.delete_capacity);

    let mut cfg = Config::default();
    // Intake host/port via env or args
    if let Some(h) = std::env::var("EBPF_NET_INTAKE_HOST")
        .ok()
        .or(cli.intake_host.clone())
    {
        cfg.intake_host = h;
    }
    if let Some(p) = std::env::var("EBPF_NET_INTAKE_PORT")
        .ok()
        .and_then(|v| v.parse().ok())
        .or(cli.intake_port)
    {
        cfg.intake_port = p;
    }
    cfg.delete_ttl = ttl;
    cfg.delete_capacity = cap;
    Ok(cfg)
}

/// Print the effective configuration in a human-readable form.
fn print_config(cfg: &Config) {
    println!("intake_host: {}", cfg.intake_host);
    println!("intake_port: {}", cfg.intake_port);
    println!("delete_ttl_secs: {}", cfg.delete_ttl.as_secs());
    println!("delete_capacity: {}", cfg.delete_capacity);
}

fn main() {
    // Initialize logging for the collector binary.
    let _ = env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
        .try_init();

    let cli = Cli::parse();
    let cfg = match build_config(&cli) {
        Ok(c) => c,
        Err(e) => {
            eprintln!("{}", e);
            std::process::exit(2);
        }
    };

    if cli.print_config {
        print_config(&cfg);
        return;
    }

    // In this minimal integration, run validates config and returns.
    if let Err(e) = k8s_collector::run_with_config(cfg) {
        eprintln!("collector error: {}", e);
        std::process::exit(1);
    }
}
