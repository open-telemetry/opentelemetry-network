//! Kubernetes metadata collector (Rust).
//!
//! This crate implements a layered pipeline to watch Kubernetes resources,
//! correlate Pods with their effective owners, and stream normalized messages
//! to the reducer. The layers are:
//! - convert_to_meta: map Kubernetes API objects to compact metadata structs
//! - tombstone_adapter: retain recent Delete events to bridge race windows
//! - matcher: match Pods to owners, handle escalation (RS→Deployment, Job→CronJob)
//! - output/encode: translate matched events into encoded render buffers
//! - writer: async TCP client with optional LZ4 streaming compression
//! - collector: orchestrates watchers, adapters, matcher, and writer
//!
//! The public entry point is [`run_with_config`].

pub mod collector;
pub mod config;
pub mod convert_to_meta;
pub mod encode;
pub mod matcher;
pub mod output;
pub mod tombstone_adapter;
pub mod types;
pub mod writer;

use crate::config::Config;

#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("collector stopped")]
    Stopped,
    #[error("failed to initialize Kubernetes client: {0}")]
    KubeInit(String),
    #[error("failed to initialize Tokio runtime: {0}")]
    RuntimeInit(String),
}

/// Convenience runner used by the binary: runs the layered pipeline with kube watchers.
///
/// Creates a Tokio runtime and executes [`collector::run`].
///
/// Errors indicate runtime initialization failure or fatal errors in the
/// orchestrated tasks.
pub fn run_with_config(cfg: Config) -> Result<(), Error> {
    let rt = tokio::runtime::Runtime::new().map_err(|e| Error::RuntimeInit(e.to_string()))?;
    rt.block_on(async move { collector::run(cfg).await })
}
