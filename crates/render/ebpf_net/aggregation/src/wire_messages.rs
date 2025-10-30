/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__agg_root_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_aggregation__agg_root_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(461u16, 16, true)
    }
}

impl Default for jb_aggregation__agg_root_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGG_ROOT_START_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agg_root_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_aggregation__agg_root_start>();
        let align = align_of::<jb_aggregation__agg_root_start>();
        let padded_raw_size = (AGG_ROOT_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__agg_root_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_aggregation__agg_root_start, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__agg_root_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_aggregation__agg_root_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(462u16, 16, true)
    }
}

impl Default for jb_aggregation__agg_root_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGG_ROOT_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agg_root_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_aggregation__agg_root_end>();
        let align = align_of::<jb_aggregation__agg_root_end>();
        let padded_raw_size = (AGG_ROOT_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__agg_root_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_aggregation__agg_root_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__update_node {
    pub _rpc_id: u16,
    pub _len: u16,
    pub id: u16,
    pub az: u16,
    pub _ref: u64,
    pub role: u16,
    pub version: u16,
    pub env: u16,
    pub ns: u16,
    pub address: u16,
    pub process: u16,
    pub container: u16,
    pub pod_name: u16,
    pub side: u8,
    pub node_type: u8,
}

impl jb_aggregation__update_node {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(463u16, true)
    }
}

impl Default for jb_aggregation__update_node {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UPDATE_NODE_WIRE_SIZE: usize = 34;

#[cfg(test)]
mod update_node_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_aggregation__update_node>();
        let align = align_of::<jb_aggregation__update_node>();
        let padded_raw_size = (UPDATE_NODE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__update_node, _rpc_id), 0);
        assert_eq!(offset_of!(jb_aggregation__update_node, _len), 2);
        assert_eq!(offset_of!(jb_aggregation__update_node, id), 4usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, az), 6usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, _ref), 8usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, role), 16usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, version), 18usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, env), 20usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, ns), 22usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, address), 24usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, process), 26usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, container), 28usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, pod_name), 30usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, side), 32usize);
        assert_eq!(offset_of!(jb_aggregation__update_node, node_type), 33usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__update_tcp_metrics {
    pub _rpc_id: u16,
    pub direction: u8,
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

impl jb_aggregation__update_tcp_metrics {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(465u16, 60, true)
    }
}

impl Default for jb_aggregation__update_tcp_metrics {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UPDATE_TCP_METRICS_WIRE_SIZE: usize = 60;

#[cfg(test)]
mod update_tcp_metrics_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_aggregation__update_tcp_metrics>();
        let align = align_of::<jb_aggregation__update_tcp_metrics>();
        let padded_raw_size = (UPDATE_TCP_METRICS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__update_tcp_metrics, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, direction),
            2usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, active_sockets),
            4usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, sum_bytes),
            8usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, sum_srtt),
            16usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, sum_delivered),
            24usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, _ref),
            32usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, sum_retrans),
            40usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, active_rtts),
            44usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, syn_timeouts),
            48usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, new_sockets),
            52usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_tcp_metrics, tcp_resets),
            56usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__update_udp_metrics {
    pub _rpc_id: u16,
    pub direction: u8,
    pub active_sockets: u32,
    pub bytes: u64,
    pub _ref: u64,
    pub addr_changes: u32,
    pub packets: u32,
    pub drops: u32,
}

impl jb_aggregation__update_udp_metrics {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(466u16, 36, true)
    }
}

impl Default for jb_aggregation__update_udp_metrics {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UPDATE_UDP_METRICS_WIRE_SIZE: usize = 36;

#[cfg(test)]
mod update_udp_metrics_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_aggregation__update_udp_metrics>();
        let align = align_of::<jb_aggregation__update_udp_metrics>();
        let padded_raw_size = (UPDATE_UDP_METRICS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__update_udp_metrics, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_aggregation__update_udp_metrics, direction),
            2usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_udp_metrics, active_sockets),
            4usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_udp_metrics, bytes),
            8usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_udp_metrics, _ref),
            16usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_udp_metrics, addr_changes),
            24usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_udp_metrics, packets),
            28usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_udp_metrics, drops),
            32usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__update_http_metrics {
    pub _rpc_id: u16,
    pub direction: u8,
    pub active_sockets: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
    pub _ref: u64,
    pub sum_code_200: u32,
    pub sum_code_400: u32,
    pub sum_code_500: u32,
    pub sum_code_other: u32,
}

impl jb_aggregation__update_http_metrics {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(467u16, 48, true)
    }
}

impl Default for jb_aggregation__update_http_metrics {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UPDATE_HTTP_METRICS_WIRE_SIZE: usize = 48;

#[cfg(test)]
mod update_http_metrics_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_aggregation__update_http_metrics>();
        let align = align_of::<jb_aggregation__update_http_metrics>();
        let padded_raw_size = (UPDATE_HTTP_METRICS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__update_http_metrics, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, direction),
            2usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, active_sockets),
            4usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, sum_total_time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, sum_processing_time_ns),
            16usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, _ref),
            24usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, sum_code_200),
            32usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, sum_code_400),
            36usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, sum_code_500),
            40usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_http_metrics, sum_code_other),
            44usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__update_dns_metrics {
    pub _rpc_id: u16,
    pub direction: u8,
    pub active_sockets: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
    pub _ref: u64,
    pub requests_a: u32,
    pub requests_aaaa: u32,
    pub responses: u32,
    pub timeouts: u32,
}

impl jb_aggregation__update_dns_metrics {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(468u16, 48, true)
    }
}

impl Default for jb_aggregation__update_dns_metrics {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UPDATE_DNS_METRICS_WIRE_SIZE: usize = 48;

#[cfg(test)]
mod update_dns_metrics_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_aggregation__update_dns_metrics>();
        let align = align_of::<jb_aggregation__update_dns_metrics>();
        let padded_raw_size = (UPDATE_DNS_METRICS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__update_dns_metrics, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, direction),
            2usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, active_sockets),
            4usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, sum_total_time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, sum_processing_time_ns),
            16usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, _ref),
            24usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, requests_a),
            32usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, requests_aaaa),
            36usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, responses),
            40usize
        );
        assert_eq!(
            offset_of!(jb_aggregation__update_dns_metrics, timeouts),
            44usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_aggregation__pulse {
    pub _rpc_id: u16,
}

impl jb_aggregation__pulse {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(65535u16, 2, true)
    }
}

impl Default for jb_aggregation__pulse {
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
        let size = size_of::<jb_aggregation__pulse>();
        let align = align_of::<jb_aggregation__pulse>();
        let padded_raw_size = (PULSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_aggregation__pulse, _rpc_id), 0);
    }
}

#[inline]
pub fn all_message_metadata() -> ::std::vec::Vec<render_parser::MessageMetadata> {
    ::std::vec![
        jb_aggregation__agg_root_start::metadata(),
        jb_aggregation__agg_root_end::metadata(),
        jb_aggregation__update_node::metadata(),
        jb_aggregation__update_tcp_metrics::metadata(),
        jb_aggregation__update_udp_metrics::metadata(),
        jb_aggregation__update_http_metrics::metadata(),
        jb_aggregation__update_dns_metrics::metadata(),
        jb_aggregation__pulse::metadata(),
    ]
}
