/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__flow_start {
    pub _rpc_id: u16,
    pub port1: u16,
    pub port2: u16,
    pub _ref: u64,
    pub addr1: u128,
    pub addr2: u128,
}

impl jb_matching__flow_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(421u16, 48, true)
    }
}

impl Default for jb_matching__flow_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const FLOW_START_WIRE_SIZE: usize = 48;

#[cfg(test)]
mod flow_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__flow_start>();
        let align = align_of::<jb_matching__flow_start>();
        let padded_raw_size = (FLOW_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__flow_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__flow_start, port1), 2usize);
        assert_eq!(offset_of!(jb_matching__flow_start, port2), 4usize);
        assert_eq!(offset_of!(jb_matching__flow_start, _ref), 8usize);
        assert_eq!(offset_of!(jb_matching__flow_start, addr1), 16usize);
        assert_eq!(offset_of!(jb_matching__flow_start, addr2), 32usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__flow_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_matching__flow_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(422u16, 16, true)
    }
}

impl Default for jb_matching__flow_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const FLOW_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod flow_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__flow_end>();
        let align = align_of::<jb_matching__flow_end>();
        let padded_raw_size = (FLOW_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__flow_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__flow_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__agent_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub id: u16,
    pub az: u16,
    pub _ref: u64,
    pub env: u16,
    pub role: u16,
    pub side: u8,
}

impl jb_matching__agent_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(423u16, true)
    }
}

impl Default for jb_matching__agent_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_INFO_WIRE_SIZE: usize = 21;

#[cfg(test)]
mod agent_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__agent_info>();
        let align = align_of::<jb_matching__agent_info>();
        let padded_raw_size = (AGENT_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__agent_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__agent_info, _len), 2);
        assert_eq!(offset_of!(jb_matching__agent_info, id), 4usize);
        assert_eq!(offset_of!(jb_matching__agent_info, az), 6usize);
        assert_eq!(offset_of!(jb_matching__agent_info, _ref), 8usize);
        assert_eq!(offset_of!(jb_matching__agent_info, env), 16usize);
        assert_eq!(offset_of!(jb_matching__agent_info, role), 18usize);
        assert_eq!(offset_of!(jb_matching__agent_info, side), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__task_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub comm: u16,
    pub side: u8,
    pub _ref: u64,
}

impl jb_matching__task_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(424u16, true)
    }
}

impl Default for jb_matching__task_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TASK_INFO_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod task_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__task_info>();
        let align = align_of::<jb_matching__task_info>();
        let padded_raw_size = (TASK_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__task_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__task_info, _len), 2);
        assert_eq!(offset_of!(jb_matching__task_info, comm), 4usize);
        assert_eq!(offset_of!(jb_matching__task_info, side), 6usize);
        assert_eq!(offset_of!(jb_matching__task_info, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__socket_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub local_port: u16,
    pub remote_port: u16,
    pub _ref: u64,
    pub side: u8,
    pub local_addr: [u8; 16],
    pub remote_addr: [u8; 16],
    pub is_connector: u8,
}

impl jb_matching__socket_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(425u16, true)
    }
}

impl Default for jb_matching__socket_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SOCKET_INFO_WIRE_SIZE: usize = 50;

#[cfg(test)]
mod socket_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__socket_info>();
        let align = align_of::<jb_matching__socket_info>();
        let padded_raw_size = (SOCKET_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__socket_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__socket_info, _len), 2);
        assert_eq!(offset_of!(jb_matching__socket_info, local_port), 4usize);
        assert_eq!(offset_of!(jb_matching__socket_info, remote_port), 6usize);
        assert_eq!(offset_of!(jb_matching__socket_info, _ref), 8usize);
        assert_eq!(offset_of!(jb_matching__socket_info, side), 16usize);
        assert_eq!(offset_of!(jb_matching__socket_info, local_addr), 17usize);
        assert_eq!(offset_of!(jb_matching__socket_info, remote_addr), 33usize);
        assert_eq!(offset_of!(jb_matching__socket_info, is_connector), 49usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__k8s_info {
    pub _rpc_id: u16,
    pub side: u8,
    pub pod_uid_suffix: [u8; 64],
    pub pod_uid_hash: u64,
    pub _ref: u64,
}

impl jb_matching__k8s_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(426u16, 88, true)
    }
}

impl Default for jb_matching__k8s_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_INFO_WIRE_SIZE: usize = 88;

#[cfg(test)]
mod k8s_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__k8s_info>();
        let align = align_of::<jb_matching__k8s_info>();
        let padded_raw_size = (K8S_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__k8s_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__k8s_info, side), 2usize);
        assert_eq!(offset_of!(jb_matching__k8s_info, pod_uid_suffix), 3usize);
        assert_eq!(offset_of!(jb_matching__k8s_info, pod_uid_hash), 72usize);
        assert_eq!(offset_of!(jb_matching__k8s_info, _ref), 80usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__tcp_update {
    pub _rpc_id: u16,
    pub side: u8,
    pub is_rx: u8,
    pub active_sockets: u32,
    pub sum_bytes: u64,
    pub sum_srtt: u64,
    pub sum_delivered: u64,
    pub _ref: u64,
    pub sum_retrans: u32,
    pub active_rtts: u32,
    pub syn_timeouts: u32,
    pub new_sockets: u32,
    pub tcp_resets: u32,
}

impl jb_matching__tcp_update {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(427u16, 60, true)
    }
}

impl Default for jb_matching__tcp_update {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TCP_UPDATE_WIRE_SIZE: usize = 60;

#[cfg(test)]
mod tcp_update_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__tcp_update>();
        let align = align_of::<jb_matching__tcp_update>();
        let padded_raw_size = (TCP_UPDATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__tcp_update, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__tcp_update, side), 2usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, is_rx), 3usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, active_sockets), 4usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, sum_bytes), 8usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, sum_srtt), 16usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, sum_delivered), 24usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, _ref), 32usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, sum_retrans), 40usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, active_rtts), 44usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, syn_timeouts), 48usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, new_sockets), 52usize);
        assert_eq!(offset_of!(jb_matching__tcp_update, tcp_resets), 56usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__udp_update {
    pub _rpc_id: u16,
    pub side: u8,
    pub is_rx: u8,
    pub active_sockets: u32,
    pub bytes: u64,
    pub _ref: u64,
    pub addr_changes: u32,
    pub packets: u32,
    pub drops: u32,
}

impl jb_matching__udp_update {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(428u16, 36, true)
    }
}

impl Default for jb_matching__udp_update {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_UPDATE_WIRE_SIZE: usize = 36;

#[cfg(test)]
mod udp_update_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__udp_update>();
        let align = align_of::<jb_matching__udp_update>();
        let padded_raw_size = (UDP_UPDATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__udp_update, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__udp_update, side), 2usize);
        assert_eq!(offset_of!(jb_matching__udp_update, is_rx), 3usize);
        assert_eq!(offset_of!(jb_matching__udp_update, active_sockets), 4usize);
        assert_eq!(offset_of!(jb_matching__udp_update, bytes), 8usize);
        assert_eq!(offset_of!(jb_matching__udp_update, _ref), 16usize);
        assert_eq!(offset_of!(jb_matching__udp_update, addr_changes), 24usize);
        assert_eq!(offset_of!(jb_matching__udp_update, packets), 28usize);
        assert_eq!(offset_of!(jb_matching__udp_update, drops), 32usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__http_update {
    pub _rpc_id: u16,
    pub side: u8,
    pub client_server: u8,
    pub active_sockets: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
    pub _ref: u64,
    pub sum_code_200: u32,
    pub sum_code_400: u32,
    pub sum_code_500: u32,
    pub sum_code_other: u32,
}

impl jb_matching__http_update {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(429u16, 48, true)
    }
}

impl Default for jb_matching__http_update {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const HTTP_UPDATE_WIRE_SIZE: usize = 48;

#[cfg(test)]
mod http_update_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__http_update>();
        let align = align_of::<jb_matching__http_update>();
        let padded_raw_size = (HTTP_UPDATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__http_update, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__http_update, side), 2usize);
        assert_eq!(offset_of!(jb_matching__http_update, client_server), 3usize);
        assert_eq!(offset_of!(jb_matching__http_update, active_sockets), 4usize);
        assert_eq!(
            offset_of!(jb_matching__http_update, sum_total_time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_matching__http_update, sum_processing_time_ns),
            16usize
        );
        assert_eq!(offset_of!(jb_matching__http_update, _ref), 24usize);
        assert_eq!(offset_of!(jb_matching__http_update, sum_code_200), 32usize);
        assert_eq!(offset_of!(jb_matching__http_update, sum_code_400), 36usize);
        assert_eq!(offset_of!(jb_matching__http_update, sum_code_500), 40usize);
        assert_eq!(
            offset_of!(jb_matching__http_update, sum_code_other),
            44usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__dns_update {
    pub _rpc_id: u16,
    pub side: u8,
    pub client_server: u8,
    pub active_sockets: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
    pub _ref: u64,
    pub requests_a: u32,
    pub requests_aaaa: u32,
    pub responses: u32,
    pub timeouts: u32,
}

impl jb_matching__dns_update {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(430u16, 48, true)
    }
}

impl Default for jb_matching__dns_update {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const DNS_UPDATE_WIRE_SIZE: usize = 48;

#[cfg(test)]
mod dns_update_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__dns_update>();
        let align = align_of::<jb_matching__dns_update>();
        let padded_raw_size = (DNS_UPDATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__dns_update, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__dns_update, side), 2usize);
        assert_eq!(offset_of!(jb_matching__dns_update, client_server), 3usize);
        assert_eq!(offset_of!(jb_matching__dns_update, active_sockets), 4usize);
        assert_eq!(
            offset_of!(jb_matching__dns_update, sum_total_time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_matching__dns_update, sum_processing_time_ns),
            16usize
        );
        assert_eq!(offset_of!(jb_matching__dns_update, _ref), 24usize);
        assert_eq!(offset_of!(jb_matching__dns_update, requests_a), 32usize);
        assert_eq!(offset_of!(jb_matching__dns_update, requests_aaaa), 36usize);
        assert_eq!(offset_of!(jb_matching__dns_update, responses), 40usize);
        assert_eq!(offset_of!(jb_matching__dns_update, timeouts), 44usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__container_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub name: u16,
    pub pod: u16,
    pub _ref: u64,
    pub role: u16,
    pub version: u16,
    pub side: u8,
    pub node_type: u8,
}

impl jb_matching__container_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(434u16, true)
    }
}

impl Default for jb_matching__container_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONTAINER_INFO_WIRE_SIZE: usize = 22;

#[cfg(test)]
mod container_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__container_info>();
        let align = align_of::<jb_matching__container_info>();
        let padded_raw_size = (CONTAINER_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__container_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__container_info, _len), 2);
        assert_eq!(offset_of!(jb_matching__container_info, name), 4usize);
        assert_eq!(offset_of!(jb_matching__container_info, pod), 6usize);
        assert_eq!(offset_of!(jb_matching__container_info, _ref), 8usize);
        assert_eq!(offset_of!(jb_matching__container_info, role), 16usize);
        assert_eq!(offset_of!(jb_matching__container_info, version), 18usize);
        assert_eq!(offset_of!(jb_matching__container_info, side), 20usize);
        assert_eq!(offset_of!(jb_matching__container_info, node_type), 21usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__service_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub side: u8,
    pub _ref: u64,
}

impl jb_matching__service_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(435u16, true)
    }
}

impl Default for jb_matching__service_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SERVICE_INFO_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod service_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__service_info>();
        let align = align_of::<jb_matching__service_info>();
        let padded_raw_size = (SERVICE_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__service_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__service_info, _len), 2);
        assert_eq!(offset_of!(jb_matching__service_info, side), 4usize);
        assert_eq!(offset_of!(jb_matching__service_info, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__aws_enrichment_start {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub ip: u128,
}

impl jb_matching__aws_enrichment_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(431u16, 32, true)
    }
}

impl Default for jb_matching__aws_enrichment_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AWS_ENRICHMENT_START_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod aws_enrichment_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__aws_enrichment_start>();
        let align = align_of::<jb_matching__aws_enrichment_start>();
        let padded_raw_size = (AWS_ENRICHMENT_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__aws_enrichment_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__aws_enrichment_start, _ref), 8usize);
        assert_eq!(offset_of!(jb_matching__aws_enrichment_start, ip), 16usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__aws_enrichment_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_matching__aws_enrichment_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(432u16, 16, true)
    }
}

impl Default for jb_matching__aws_enrichment_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AWS_ENRICHMENT_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod aws_enrichment_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__aws_enrichment_end>();
        let align = align_of::<jb_matching__aws_enrichment_end>();
        let padded_raw_size = (AWS_ENRICHMENT_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__aws_enrichment_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__aws_enrichment_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__aws_enrichment {
    pub _rpc_id: u16,
    pub _len: u16,
    pub role: u16,
    pub az: u16,
    pub _ref: u64,
}

impl jb_matching__aws_enrichment {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(433u16, true)
    }
}

impl Default for jb_matching__aws_enrichment {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AWS_ENRICHMENT_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod aws_enrichment_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__aws_enrichment>();
        let align = align_of::<jb_matching__aws_enrichment>();
        let padded_raw_size = (AWS_ENRICHMENT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__aws_enrichment, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__aws_enrichment, _len), 2);
        assert_eq!(offset_of!(jb_matching__aws_enrichment, role), 4usize);
        assert_eq!(offset_of!(jb_matching__aws_enrichment, az), 6usize);
        assert_eq!(offset_of!(jb_matching__aws_enrichment, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__k8s_pod_start {
    pub _rpc_id: u16,
    pub uid_suffix: [u8; 64],
    pub uid_hash: u64,
    pub _ref: u64,
}

impl jb_matching__k8s_pod_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(436u16, 88, true)
    }
}

impl Default for jb_matching__k8s_pod_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_POD_START_WIRE_SIZE: usize = 88;

#[cfg(test)]
mod k8s_pod_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__k8s_pod_start>();
        let align = align_of::<jb_matching__k8s_pod_start>();
        let padded_raw_size = (K8S_POD_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__k8s_pod_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__k8s_pod_start, uid_suffix), 2usize);
        assert_eq!(offset_of!(jb_matching__k8s_pod_start, uid_hash), 72usize);
        assert_eq!(offset_of!(jb_matching__k8s_pod_start, _ref), 80usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__k8s_pod_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_matching__k8s_pod_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(437u16, 16, true)
    }
}

impl Default for jb_matching__k8s_pod_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_POD_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod k8s_pod_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__k8s_pod_end>();
        let align = align_of::<jb_matching__k8s_pod_end>();
        let padded_raw_size = (K8S_POD_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__k8s_pod_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__k8s_pod_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__set_pod_detail {
    pub _rpc_id: u16,
    pub _len: u16,
    pub owner_name: u16,
    pub pod_name: u16,
    pub _ref: u64,
    pub ns: u16,
    pub version: u16,
}

impl jb_matching__set_pod_detail {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(438u16, true)
    }
}

impl Default for jb_matching__set_pod_detail {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_POD_DETAIL_WIRE_SIZE: usize = 20;

#[cfg(test)]
mod set_pod_detail_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__set_pod_detail>();
        let align = align_of::<jb_matching__set_pod_detail>();
        let padded_raw_size = (SET_POD_DETAIL_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__set_pod_detail, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__set_pod_detail, _len), 2);
        assert_eq!(offset_of!(jb_matching__set_pod_detail, owner_name), 4usize);
        assert_eq!(offset_of!(jb_matching__set_pod_detail, pod_name), 6usize);
        assert_eq!(offset_of!(jb_matching__set_pod_detail, _ref), 8usize);
        assert_eq!(offset_of!(jb_matching__set_pod_detail, ns), 16usize);
        assert_eq!(offset_of!(jb_matching__set_pod_detail, version), 18usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__k8s_container_start {
    pub _rpc_id: u16,
    pub uid_suffix: [u8; 64],
    pub uid_hash: u64,
    pub _ref: u64,
}

impl jb_matching__k8s_container_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(439u16, 88, true)
    }
}

impl Default for jb_matching__k8s_container_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_CONTAINER_START_WIRE_SIZE: usize = 88;

#[cfg(test)]
mod k8s_container_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__k8s_container_start>();
        let align = align_of::<jb_matching__k8s_container_start>();
        let padded_raw_size = (K8S_CONTAINER_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__k8s_container_start, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_matching__k8s_container_start, uid_suffix),
            2usize
        );
        assert_eq!(
            offset_of!(jb_matching__k8s_container_start, uid_hash),
            72usize
        );
        assert_eq!(offset_of!(jb_matching__k8s_container_start, _ref), 80usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__k8s_container_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_matching__k8s_container_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(440u16, 16, true)
    }
}

impl Default for jb_matching__k8s_container_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_CONTAINER_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod k8s_container_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__k8s_container_end>();
        let align = align_of::<jb_matching__k8s_container_end>();
        let padded_raw_size = (K8S_CONTAINER_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__k8s_container_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__k8s_container_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__set_container_pod {
    pub _rpc_id: u16,
    pub _len: u16,
    pub name: u16,
    pub pod_uid_suffix: [u8; 64],
    pub pod_uid_hash: u64,
    pub _ref: u64,
}

impl jb_matching__set_container_pod {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(471u16, true)
    }
}

impl Default for jb_matching__set_container_pod {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_CONTAINER_POD_WIRE_SIZE: usize = 88;

#[cfg(test)]
mod set_container_pod_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__set_container_pod>();
        let align = align_of::<jb_matching__set_container_pod>();
        let padded_raw_size = (SET_CONTAINER_POD_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__set_container_pod, _rpc_id), 0);
        assert_eq!(offset_of!(jb_matching__set_container_pod, _len), 2);
        assert_eq!(offset_of!(jb_matching__set_container_pod, name), 4usize);
        assert_eq!(
            offset_of!(jb_matching__set_container_pod, pod_uid_suffix),
            6usize
        );
        assert_eq!(
            offset_of!(jb_matching__set_container_pod, pod_uid_hash),
            72usize
        );
        assert_eq!(offset_of!(jb_matching__set_container_pod, _ref), 80usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_matching__pulse {
    pub _rpc_id: u16,
}

impl jb_matching__pulse {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(65535u16, 2, true)
    }
}

impl Default for jb_matching__pulse {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PULSE_WIRE_SIZE: usize = 2;

#[cfg(test)]
mod pulse_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_matching__pulse>();
        let align = align_of::<jb_matching__pulse>();
        let padded_raw_size = (PULSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_matching__pulse, _rpc_id), 0);
    }
}

#[inline]
pub fn all_message_metadata() -> ::std::vec::Vec<render_parser::MessageMetadata> {
    ::std::vec![
        jb_matching__flow_start::metadata(),
        jb_matching__flow_end::metadata(),
        jb_matching__agent_info::metadata(),
        jb_matching__task_info::metadata(),
        jb_matching__socket_info::metadata(),
        jb_matching__k8s_info::metadata(),
        jb_matching__tcp_update::metadata(),
        jb_matching__udp_update::metadata(),
        jb_matching__http_update::metadata(),
        jb_matching__dns_update::metadata(),
        jb_matching__container_info::metadata(),
        jb_matching__service_info::metadata(),
        jb_matching__aws_enrichment_start::metadata(),
        jb_matching__aws_enrichment_end::metadata(),
        jb_matching__aws_enrichment::metadata(),
        jb_matching__k8s_pod_start::metadata(),
        jb_matching__k8s_pod_end::metadata(),
        jb_matching__set_pod_detail::metadata(),
        jb_matching__k8s_container_start::metadata(),
        jb_matching__k8s_container_end::metadata(),
        jb_matching__set_container_pod::metadata(),
        jb_matching__pulse::metadata(),
    ]
}
