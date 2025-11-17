//! Encoder for render events into ingestion wire buffers.
//!
//! Uses the generated encoder functions from `encoder_ebpf_net_ingest` to
//! produce exact-sized buffers expected by the reducer. The `encode` function
//! maps a [`RenderEvent`](crate::output::RenderEvent) into a ready-to-send
//! buffer.

use core::ffi::c_char;

use crate::output::RenderEvent;
use crate::types::OwnerKind;

// Ensure encoder crate is linked so its #[no_mangle] symbols are available
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_ingest;

#[repr(C)]
struct JbBlob {
    buf: *const c_char,
    len: u16,
}

extern "C" {
    fn ebpf_net_ingest_encode_pod_new_with_name(
        dest: *mut u8,
        dest_len: u32,
        tstamp: u64,
        uid: JbBlob,
        ip: u32,
        owner_name: JbBlob,
        pod_name: JbBlob,
        owner_kind: u8,
        owner_uid: JbBlob,
        is_host_network: u8,
        ns: JbBlob,
        version: JbBlob,
    );
    fn ebpf_net_ingest_encode_pod_container(
        dest: *mut u8,
        dest_len: u32,
        tstamp: u64,
        uid: JbBlob,
        container_id: JbBlob,
        container_name: JbBlob,
        container_image: JbBlob,
    );
    fn ebpf_net_ingest_encode_pod_delete(dest: *mut u8, dest_len: u32, tstamp: u64, uid: JbBlob);
    fn ebpf_net_ingest_encode_pod_resync(
        dest: *mut u8,
        dest_len: u32,
        tstamp: u64,
        resync_count: u64,
    );
}
extern "C" {
    fn ebpf_net_ingest_encode_version_info(
        dest: *mut u8,
        dest_len: u32,
        tstamp: u64,
        major: u32,
        minor: u32,
        patch: u32,
    );
}

/// Construct a `JbBlob` view from a Rust `&str`.
fn as_blob(s: &str) -> JbBlob {
    JbBlob {
        buf: s.as_ptr() as *const c_char,
        len: s.len() as u16,
    }
}

/// Map [`OwnerKind`] into the reducer's numeric enum codes.
fn owner_kind_code(k: OwnerKind) -> u8 {
    match k {
        OwnerKind::ReplicaSet => 0,
        OwnerKind::Deployment => 1,
        OwnerKind::Job => 2,
        OwnerKind::CronJob => 3,
        OwnerKind::NoOwner => 254,
        OwnerKind::Other => 255,
    }
}

/// Parse an IPv4 string as a host-order `u32`.
fn ipv4_to_u32(ip: &str) -> u32 {
    ip.parse::<std::net::Ipv4Addr>().map(u32::from).unwrap_or(0)
}

/// Encode a [`RenderEvent`] into a ready-to-send wire buffer.
pub fn encode(event: &RenderEvent, tstamp: u64) -> Vec<u8> {
    match event {
        RenderEvent::PodResync { epoch } => {
            let len = 8 + 16; // timestamp + struct
            let mut buf = vec![0u8; len];
            unsafe {
                ebpf_net_ingest_encode_pod_resync(
                    buf.as_mut_ptr(),
                    buf.len() as u32,
                    tstamp,
                    *epoch,
                );
            }
            buf
        }
        RenderEvent::PodDelete { uid } => {
            let consumed = 4 + uid.len();
            let len = 8 + consumed;
            let mut buf = vec![0u8; len];
            unsafe {
                ebpf_net_ingest_encode_pod_delete(
                    buf.as_mut_ptr(),
                    buf.len() as u32,
                    tstamp,
                    as_blob(uid),
                );
            }
            buf
        }
        RenderEvent::PodContainer { uid, container } => {
            let consumed =
                10 + uid.len() + container.id.len() + container.name.len() + container.image.len();
            let len = 8 + consumed;
            let mut buf = vec![0u8; len];
            unsafe {
                ebpf_net_ingest_encode_pod_container(
                    buf.as_mut_ptr(),
                    buf.len() as u32,
                    tstamp,
                    as_blob(uid),
                    as_blob(&container.id),
                    as_blob(&container.name),
                    as_blob(&container.image),
                );
            }
            buf
        }
        RenderEvent::PodNew {
            uid,
            ip,
            pod_name,
            ns,
            version,
            owner_kind,
            owner_uid,
            owner_name,
            is_host_network,
        } => {
            let consumed = 20
                + uid.len()
                + owner_name.len()
                + pod_name.len()
                + owner_uid.len()
                + ns.len()
                + version.len();
            let len = 8 + consumed;
            let mut buf = vec![0u8; len];
            unsafe {
                ebpf_net_ingest_encode_pod_new_with_name(
                    buf.as_mut_ptr(),
                    buf.len() as u32,
                    tstamp,
                    as_blob(uid),
                    ipv4_to_u32(ip),
                    as_blob(owner_name),
                    as_blob(pod_name),
                    owner_kind_code(*owner_kind),
                    as_blob(owner_uid),
                    if *is_host_network { 1 } else { 0 },
                    as_blob(ns),
                    as_blob(version),
                );
            }
            buf
        }
    }
}

/// Encode a `version_info` handshake message.
pub fn encode_version_info(major: u32, minor: u32, patch: u32, tstamp: u64) -> Vec<u8> {
    let len = 8 + 16;
    let mut buf = vec![0u8; len];
    unsafe {
        ebpf_net_ingest_encode_version_info(
            buf.as_mut_ptr(),
            buf.len() as u32,
            tstamp,
            major,
            minor,
            patch,
        );
    }
    buf
}
