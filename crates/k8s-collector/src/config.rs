//! Configuration for the Kubernetes metadata collector.
//!
//! Settings cover endpoint selection and deletion tombstone policy.

use std::time::Duration;

#[derive(Clone, Debug)]
pub struct Config {
    /// Reducer host to connect to (ingest TCP server)
    pub intake_host: String,
    /// Reducer port to connect to
    pub intake_port: u16,
    /// Max time to retain delete tombstones to satisfy straggling updates.
    pub delete_ttl: Duration,
    /// Max number of delete tombstones to retain (older ones evicted).
    pub delete_capacity: usize,
}

impl Default for Config {
    /// Reasonable defaults:
    /// - connect to `127.0.0.1:8000`
    /// - retain tombstones for 60s up to 10k entries
    fn default() -> Self {
        Self {
            intake_host: "127.0.0.1".into(),
            intake_port: 8000,
            delete_ttl: Duration::from_secs(60),
            delete_capacity: 10_000,
        }
    }
}
