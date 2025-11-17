/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__logger_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__logger_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(600u16, 16, true)
    }
}

impl Default for jb_logging__logger_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const LOGGER_START_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod logger_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__logger_start>();
        let align = align_of::<jb_logging__logger_start>();
        let padded_raw_size = (LOGGER_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__logger_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__logger_start, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__logger_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__logger_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(601u16, 16, true)
    }
}

impl Default for jb_logging__logger_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const LOGGER_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod logger_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__logger_end>();
        let align = align_of::<jb_logging__logger_end>();
        let padded_raw_size = (LOGGER_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__logger_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__logger_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agent_lost_events {
    pub _rpc_id: u16,
    pub _len: u16,
    pub count: u32,
    pub _ref: u64,
}

impl jb_logging__agent_lost_events {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(602u16, true)
    }
}

impl Default for jb_logging__agent_lost_events {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_LOST_EVENTS_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agent_lost_events_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agent_lost_events>();
        let align = align_of::<jb_logging__agent_lost_events>();
        let padded_raw_size = (AGENT_LOST_EVENTS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__agent_lost_events, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__agent_lost_events, _len), 2);
        assert_eq!(offset_of!(jb_logging__agent_lost_events, count), 4usize);
        assert_eq!(offset_of!(jb_logging__agent_lost_events, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__pod_not_found {
    pub _rpc_id: u16,
    pub _len: u16,
    pub on_delete: u8,
    pub _ref: u64,
}

impl jb_logging__pod_not_found {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(603u16, true)
    }
}

impl Default for jb_logging__pod_not_found {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_NOT_FOUND_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod pod_not_found_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__pod_not_found>();
        let align = align_of::<jb_logging__pod_not_found>();
        let padded_raw_size = (POD_NOT_FOUND_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__pod_not_found, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__pod_not_found, _len), 2);
        assert_eq!(offset_of!(jb_logging__pod_not_found, on_delete), 4usize);
        assert_eq!(offset_of!(jb_logging__pod_not_found, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__cgroup_not_found {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub _ref: u64,
}

impl jb_logging__cgroup_not_found {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(604u16, 24, true)
    }
}

impl Default for jb_logging__cgroup_not_found {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CGROUP_NOT_FOUND_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod cgroup_not_found_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__cgroup_not_found>();
        let align = align_of::<jb_logging__cgroup_not_found>();
        let padded_raw_size = (CGROUP_NOT_FOUND_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__cgroup_not_found, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__cgroup_not_found, cgroup), 8usize);
        assert_eq!(offset_of!(jb_logging__cgroup_not_found, _ref), 16usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__rewriting_private_to_public_ip_mapping {
    pub _rpc_id: u16,
    pub _len: u16,
    pub private_addr: u16,
    pub existing_public_addr: u16,
    pub _ref: u64,
}

impl jb_logging__rewriting_private_to_public_ip_mapping {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(605u16, true)
    }
}

impl Default for jb_logging__rewriting_private_to_public_ip_mapping {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const REWRITING_PRIVATE_TO_PUBLIC_IP_MAPPING_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod rewriting_private_to_public_ip_mapping_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__rewriting_private_to_public_ip_mapping>();
        let align = align_of::<jb_logging__rewriting_private_to_public_ip_mapping>();
        let padded_raw_size =
            (REWRITING_PRIVATE_TO_PUBLIC_IP_MAPPING_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__rewriting_private_to_public_ip_mapping, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__rewriting_private_to_public_ip_mapping, _len),
            2
        );
        assert_eq!(
            offset_of!(
                jb_logging__rewriting_private_to_public_ip_mapping,
                private_addr
            ),
            4usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__rewriting_private_to_public_ip_mapping,
                existing_public_addr
            ),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__rewriting_private_to_public_ip_mapping, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__private_ip_in_private_to_public_ip_mapping {
    pub _rpc_id: u16,
    pub _len: u16,
    pub private_addr: u16,
    pub _ref: u64,
}

impl jb_logging__private_ip_in_private_to_public_ip_mapping {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(606u16, true)
    }
}

impl Default for jb_logging__private_ip_in_private_to_public_ip_mapping {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PRIVATE_IP_IN_PRIVATE_TO_PUBLIC_IP_MAPPING_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod private_ip_in_private_to_public_ip_mapping_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__private_ip_in_private_to_public_ip_mapping>();
        let align = align_of::<jb_logging__private_ip_in_private_to_public_ip_mapping>();
        let padded_raw_size =
            (PRIVATE_IP_IN_PRIVATE_TO_PUBLIC_IP_MAPPING_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(
                jb_logging__private_ip_in_private_to_public_ip_mapping,
                _rpc_id
            ),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__private_ip_in_private_to_public_ip_mapping, _len),
            2
        );
        assert_eq!(
            offset_of!(
                jb_logging__private_ip_in_private_to_public_ip_mapping,
                private_addr
            ),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__private_ip_in_private_to_public_ip_mapping, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__failed_to_insert_dns_record {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__failed_to_insert_dns_record {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(607u16, 16, true)
    }
}

impl Default for jb_logging__failed_to_insert_dns_record {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const FAILED_TO_INSERT_DNS_RECORD_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod failed_to_insert_dns_record_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__failed_to_insert_dns_record>();
        let align = align_of::<jb_logging__failed_to_insert_dns_record>();
        let padded_raw_size = (FAILED_TO_INSERT_DNS_RECORD_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__failed_to_insert_dns_record, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__failed_to_insert_dns_record, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__tcp_socket_failed_getting_process_reference {
    pub _rpc_id: u16,
    pub pid: u32,
    pub _ref: u64,
}

impl jb_logging__tcp_socket_failed_getting_process_reference {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(608u16, 16, true)
    }
}

impl Default for jb_logging__tcp_socket_failed_getting_process_reference {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TCP_SOCKET_FAILED_GETTING_PROCESS_REFERENCE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod tcp_socket_failed_getting_process_reference_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__tcp_socket_failed_getting_process_reference>();
        let align = align_of::<jb_logging__tcp_socket_failed_getting_process_reference>();
        let padded_raw_size =
            (TCP_SOCKET_FAILED_GETTING_PROCESS_REFERENCE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(
                jb_logging__tcp_socket_failed_getting_process_reference,
                _rpc_id
            ),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__tcp_socket_failed_getting_process_reference, pid),
            4usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__tcp_socket_failed_getting_process_reference,
                _ref
            ),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__udp_socket_failed_getting_process_reference {
    pub _rpc_id: u16,
    pub pid: u32,
    pub _ref: u64,
}

impl jb_logging__udp_socket_failed_getting_process_reference {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(609u16, 16, true)
    }
}

impl Default for jb_logging__udp_socket_failed_getting_process_reference {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_SOCKET_FAILED_GETTING_PROCESS_REFERENCE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod udp_socket_failed_getting_process_reference_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__udp_socket_failed_getting_process_reference>();
        let align = align_of::<jb_logging__udp_socket_failed_getting_process_reference>();
        let padded_raw_size =
            (UDP_SOCKET_FAILED_GETTING_PROCESS_REFERENCE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(
                jb_logging__udp_socket_failed_getting_process_reference,
                _rpc_id
            ),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__udp_socket_failed_getting_process_reference, pid),
            4usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__udp_socket_failed_getting_process_reference,
                _ref
            ),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__socket_address_already_assigned {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__socket_address_already_assigned {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(610u16, 16, true)
    }
}

impl Default for jb_logging__socket_address_already_assigned {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SOCKET_ADDRESS_ALREADY_ASSIGNED_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod socket_address_already_assigned_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__socket_address_already_assigned>();
        let align = align_of::<jb_logging__socket_address_already_assigned>();
        let padded_raw_size =
            (SOCKET_ADDRESS_ALREADY_ASSIGNED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__socket_address_already_assigned, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__socket_address_already_assigned, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__ingest_decompression_error {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_hostname: u16,
    pub client_type: u8,
    pub _ref: u64,
}

impl jb_logging__ingest_decompression_error {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(611u16, true)
    }
}

impl Default for jb_logging__ingest_decompression_error {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const INGEST_DECOMPRESSION_ERROR_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod ingest_decompression_error_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__ingest_decompression_error>();
        let align = align_of::<jb_logging__ingest_decompression_error>();
        let padded_raw_size = (INGEST_DECOMPRESSION_ERROR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__ingest_decompression_error, _rpc_id),
            0
        );
        assert_eq!(offset_of!(jb_logging__ingest_decompression_error, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__ingest_decompression_error, client_hostname),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__ingest_decompression_error, client_type),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__ingest_decompression_error, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__ingest_processing_error {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_hostname: u16,
    pub client_type: u8,
    pub _ref: u64,
}

impl jb_logging__ingest_processing_error {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(612u16, true)
    }
}

impl Default for jb_logging__ingest_processing_error {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const INGEST_PROCESSING_ERROR_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod ingest_processing_error_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__ingest_processing_error>();
        let align = align_of::<jb_logging__ingest_processing_error>();
        let padded_raw_size = (INGEST_PROCESSING_ERROR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__ingest_processing_error, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__ingest_processing_error, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__ingest_processing_error, client_hostname),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__ingest_processing_error, client_type),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__ingest_processing_error, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__ingest_connection_error {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_hostname: u16,
    pub client_type: u8,
    pub _ref: u64,
}

impl jb_logging__ingest_connection_error {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(613u16, true)
    }
}

impl Default for jb_logging__ingest_connection_error {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const INGEST_CONNECTION_ERROR_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod ingest_connection_error_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__ingest_connection_error>();
        let align = align_of::<jb_logging__ingest_connection_error>();
        let padded_raw_size = (INGEST_CONNECTION_ERROR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__ingest_connection_error, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__ingest_connection_error, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__ingest_connection_error, client_hostname),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__ingest_connection_error, client_type),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__ingest_connection_error, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agent_auth_success {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_type: u8,
    pub _ref: u64,
}

impl jb_logging__agent_auth_success {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(614u16, true)
    }
}

impl Default for jb_logging__agent_auth_success {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_AUTH_SUCCESS_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agent_auth_success_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agent_auth_success>();
        let align = align_of::<jb_logging__agent_auth_success>();
        let padded_raw_size = (AGENT_AUTH_SUCCESS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__agent_auth_success, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__agent_auth_success, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__agent_auth_success, client_type),
            4usize
        );
        assert_eq!(offset_of!(jb_logging__agent_auth_success, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agent_auth_failure {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_hostname: u16,
    pub client_type: u8,
    pub _ref: u64,
}

impl jb_logging__agent_auth_failure {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(615u16, true)
    }
}

impl Default for jb_logging__agent_auth_failure {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_AUTH_FAILURE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agent_auth_failure_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agent_auth_failure>();
        let align = align_of::<jb_logging__agent_auth_failure>();
        let padded_raw_size = (AGENT_AUTH_FAILURE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__agent_auth_failure, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__agent_auth_failure, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__agent_auth_failure, client_hostname),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_auth_failure, client_type),
            6usize
        );
        assert_eq!(offset_of!(jb_logging__agent_auth_failure, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agent_attempting_auth_using_api_key {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_type: u8,
    pub _ref: u64,
}

impl jb_logging__agent_attempting_auth_using_api_key {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(616u16, true)
    }
}

impl Default for jb_logging__agent_attempting_auth_using_api_key {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_ATTEMPTING_AUTH_USING_API_KEY_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agent_attempting_auth_using_api_key_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agent_attempting_auth_using_api_key>();
        let align = align_of::<jb_logging__agent_attempting_auth_using_api_key>();
        let padded_raw_size =
            (AGENT_ATTEMPTING_AUTH_USING_API_KEY_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__agent_attempting_auth_using_api_key, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__agent_attempting_auth_using_api_key, _len),
            2
        );
        assert_eq!(
            offset_of!(jb_logging__agent_attempting_auth_using_api_key, client_type),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_attempting_auth_using_api_key, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__k8s_container_pod_not_found {
    pub _rpc_id: u16,
    pub pod_uid_suffix: [u8; 64],
    pub pod_uid_hash: u64,
    pub _ref: u64,
}

impl jb_logging__k8s_container_pod_not_found {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(617u16, 88, true)
    }
}

impl Default for jb_logging__k8s_container_pod_not_found {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_CONTAINER_POD_NOT_FOUND_WIRE_SIZE: usize = 88;

#[cfg(test)]
mod k8s_container_pod_not_found_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__k8s_container_pod_not_found>();
        let align = align_of::<jb_logging__k8s_container_pod_not_found>();
        let padded_raw_size = (K8S_CONTAINER_POD_NOT_FOUND_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__k8s_container_pod_not_found, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__k8s_container_pod_not_found, pod_uid_suffix),
            2usize
        );
        assert_eq!(
            offset_of!(jb_logging__k8s_container_pod_not_found, pod_uid_hash),
            72usize
        );
        assert_eq!(
            offset_of!(jb_logging__k8s_container_pod_not_found, _ref),
            80usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agent_connect_success {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_type: u8,
    pub _ref: u64,
}

impl jb_logging__agent_connect_success {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(618u16, true)
    }
}

impl Default for jb_logging__agent_connect_success {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_CONNECT_SUCCESS_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agent_connect_success_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agent_connect_success>();
        let align = align_of::<jb_logging__agent_connect_success>();
        let padded_raw_size = (AGENT_CONNECT_SUCCESS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__agent_connect_success, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__agent_connect_success, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__agent_connect_success, client_type),
            4usize
        );
        assert_eq!(offset_of!(jb_logging__agent_connect_success, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__core_stats_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__core_stats_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(619u16, 16, true)
    }
}

impl Default for jb_logging__core_stats_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CORE_STATS_START_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod core_stats_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__core_stats_start>();
        let align = align_of::<jb_logging__core_stats_start>();
        let padded_raw_size = (CORE_STATS_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__core_stats_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__core_stats_start, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__core_stats_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__core_stats_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(620u16, 16, true)
    }
}

impl Default for jb_logging__core_stats_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CORE_STATS_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod core_stats_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__core_stats_end>();
        let align = align_of::<jb_logging__core_stats_end>();
        let padded_raw_size = (CORE_STATS_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__core_stats_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__core_stats_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__span_utilization_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub span_name: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub _ref: u64,
    pub allocated: u16,
    pub max_allocated: u16,
    pub pool_size_: u16,
}

impl jb_logging__span_utilization_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(621u16, true)
    }
}

impl Default for jb_logging__span_utilization_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SPAN_UTILIZATION_STATS_WIRE_SIZE: usize = 30;

#[cfg(test)]
mod span_utilization_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__span_utilization_stats>();
        let align = align_of::<jb_logging__span_utilization_stats>();
        let padded_raw_size = (SPAN_UTILIZATION_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__span_utilization_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__span_utilization_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__span_utilization_stats, span_name),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__span_utilization_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__span_utilization_stats, time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__span_utilization_stats, _ref),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__span_utilization_stats, allocated),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__span_utilization_stats, max_allocated),
            26usize
        );
        assert_eq!(
            offset_of!(jb_logging__span_utilization_stats, pool_size_),
            28usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__connection_message_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub severity_: u32,
    pub time_ns: u64,
    pub count: u64,
    pub _ref: u64,
    pub module: u16,
    pub shard: u16,
    pub conn: u16,
}

impl jb_logging__connection_message_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(622u16, true)
    }
}

impl Default for jb_logging__connection_message_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONNECTION_MESSAGE_STATS_WIRE_SIZE: usize = 38;

#[cfg(test)]
mod connection_message_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__connection_message_stats>();
        let align = align_of::<jb_logging__connection_message_stats>();
        let padded_raw_size = (CONNECTION_MESSAGE_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__connection_message_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__connection_message_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__connection_message_stats, severity_),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_stats, time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_stats, count),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_stats, _ref),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_stats, module),
            32usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_stats, shard),
            34usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_stats, conn),
            36usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__connection_message_error_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub count: u64,
    pub time_ns: u64,
    pub _ref: u64,
    pub conn: u16,
    pub msg_: u16,
}

impl jb_logging__connection_message_error_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(623u16, true)
    }
}

impl Default for jb_logging__connection_message_error_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONNECTION_MESSAGE_ERROR_STATS_WIRE_SIZE: usize = 36;

#[cfg(test)]
mod connection_message_error_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__connection_message_error_stats>();
        let align = align_of::<jb_logging__connection_message_error_stats>();
        let padded_raw_size =
            (CONNECTION_MESSAGE_ERROR_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, _len),
            2
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, module),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, count),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, time_ns),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, _ref),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, conn),
            32usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_message_error_stats, msg_),
            34usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__status_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub _ref: u64,
    pub program: u16,
    pub status: u8,
}

impl jb_logging__status_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(624u16, true)
    }
}

impl Default for jb_logging__status_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const STATUS_STATS_WIRE_SIZE: usize = 27;

#[cfg(test)]
mod status_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__status_stats>();
        let align = align_of::<jb_logging__status_stats>();
        let padded_raw_size = (STATUS_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__status_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__status_stats, _len), 2);
        assert_eq!(offset_of!(jb_logging__status_stats, module), 4usize);
        assert_eq!(offset_of!(jb_logging__status_stats, shard), 6usize);
        assert_eq!(offset_of!(jb_logging__status_stats, time_ns), 8usize);
        assert_eq!(offset_of!(jb_logging__status_stats, _ref), 16usize);
        assert_eq!(offset_of!(jb_logging__status_stats, program), 24usize);
        assert_eq!(offset_of!(jb_logging__status_stats, status), 26usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__rpc_receive_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub receiver_app: u16,
    pub shard: u16,
    pub max_latency_ns: u64,
    pub time_ns: u64,
    pub _ref: u64,
}

impl jb_logging__rpc_receive_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(625u16, true)
    }
}

impl Default for jb_logging__rpc_receive_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const RPC_RECEIVE_STATS_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod rpc_receive_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__rpc_receive_stats>();
        let align = align_of::<jb_logging__rpc_receive_stats>();
        let padded_raw_size = (RPC_RECEIVE_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__rpc_receive_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__rpc_receive_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__rpc_receive_stats, receiver_app),
            4usize
        );
        assert_eq!(offset_of!(jb_logging__rpc_receive_stats, shard), 6usize);
        assert_eq!(
            offset_of!(jb_logging__rpc_receive_stats, max_latency_ns),
            8usize
        );
        assert_eq!(offset_of!(jb_logging__rpc_receive_stats, time_ns), 16usize);
        assert_eq!(offset_of!(jb_logging__rpc_receive_stats, _ref), 24usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__rpc_write_stalls_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub sender_app: u16,
    pub shard: u16,
    pub count: u64,
    pub time_ns: u64,
    pub _ref: u64,
}

impl jb_logging__rpc_write_stalls_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(626u16, true)
    }
}

impl Default for jb_logging__rpc_write_stalls_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const RPC_WRITE_STALLS_STATS_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod rpc_write_stalls_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__rpc_write_stalls_stats>();
        let align = align_of::<jb_logging__rpc_write_stalls_stats>();
        let padded_raw_size = (RPC_WRITE_STALLS_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__rpc_write_stalls_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__rpc_write_stalls_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__rpc_write_stalls_stats, sender_app),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_stalls_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_stalls_stats, count),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_stalls_stats, time_ns),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_stalls_stats, _ref),
            24usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__rpc_write_utilization_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub max_buf_used: u32,
    pub max_buf_util: u64,
    pub max_elem_util: u64,
    pub time_ns: u64,
    pub _ref: u64,
    pub sender_app: u16,
    pub shard: u16,
}

impl jb_logging__rpc_write_utilization_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(627u16, true)
    }
}

impl Default for jb_logging__rpc_write_utilization_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const RPC_WRITE_UTILIZATION_STATS_WIRE_SIZE: usize = 44;

#[cfg(test)]
mod rpc_write_utilization_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__rpc_write_utilization_stats>();
        let align = align_of::<jb_logging__rpc_write_utilization_stats>();
        let padded_raw_size = (RPC_WRITE_UTILIZATION_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, _rpc_id),
            0
        );
        assert_eq!(offset_of!(jb_logging__rpc_write_utilization_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, max_buf_used),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, max_buf_util),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, max_elem_util),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, time_ns),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, _ref),
            32usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, sender_app),
            40usize
        );
        assert_eq!(
            offset_of!(jb_logging__rpc_write_utilization_stats, shard),
            42usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__code_timing_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub name: u16,
    pub line: u16,
    pub index_string: u64,
    pub count: u64,
    pub avg_ns: u64,
    pub min_ns: u64,
    pub max_ns: u64,
    pub sum_ns: u64,
    pub time_ns: u64,
    pub _ref: u64,
}

impl jb_logging__code_timing_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(628u16, true)
    }
}

impl Default for jb_logging__code_timing_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CODE_TIMING_STATS_WIRE_SIZE: usize = 72;

#[cfg(test)]
mod code_timing_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__code_timing_stats>();
        let align = align_of::<jb_logging__code_timing_stats>();
        let padded_raw_size = (CODE_TIMING_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__code_timing_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, _len), 2);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, name), 4usize);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, line), 6usize);
        assert_eq!(
            offset_of!(jb_logging__code_timing_stats, index_string),
            8usize
        );
        assert_eq!(offset_of!(jb_logging__code_timing_stats, count), 16usize);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, avg_ns), 24usize);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, min_ns), 32usize);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, max_ns), 40usize);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, sum_ns), 48usize);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, time_ns), 56usize);
        assert_eq!(offset_of!(jb_logging__code_timing_stats, _ref), 64usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agg_core_stats_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__agg_core_stats_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(629u16, 16, true)
    }
}

impl Default for jb_logging__agg_core_stats_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGG_CORE_STATS_START_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agg_core_stats_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agg_core_stats_start>();
        let align = align_of::<jb_logging__agg_core_stats_start>();
        let padded_raw_size = (AGG_CORE_STATS_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__agg_core_stats_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__agg_core_stats_start, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agg_core_stats_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__agg_core_stats_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(630u16, 16, true)
    }
}

impl Default for jb_logging__agg_core_stats_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGG_CORE_STATS_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod agg_core_stats_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agg_core_stats_end>();
        let align = align_of::<jb_logging__agg_core_stats_end>();
        let padded_raw_size = (AGG_CORE_STATS_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__agg_core_stats_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__agg_core_stats_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agg_root_truncation_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub count: u64,
    pub time_ns: u64,
    pub _ref: u64,
}

impl jb_logging__agg_root_truncation_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(631u16, true)
    }
}

impl Default for jb_logging__agg_root_truncation_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGG_ROOT_TRUNCATION_STATS_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod agg_root_truncation_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agg_root_truncation_stats>();
        let align = align_of::<jb_logging__agg_root_truncation_stats>();
        let padded_raw_size = (AGG_ROOT_TRUNCATION_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__agg_root_truncation_stats, _rpc_id),
            0
        );
        assert_eq!(offset_of!(jb_logging__agg_root_truncation_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__agg_root_truncation_stats, module),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_root_truncation_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_root_truncation_stats, count),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_root_truncation_stats, time_ns),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_root_truncation_stats, _ref),
            24usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agg_prometheus_bytes_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub shard: u16,
    pub prometheus_bytes_written: u64,
    pub prometheus_bytes_discarded: u64,
    pub time_ns: u64,
    pub _ref: u64,
}

impl jb_logging__agg_prometheus_bytes_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(632u16, true)
    }
}

impl Default for jb_logging__agg_prometheus_bytes_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGG_PROMETHEUS_BYTES_STATS_WIRE_SIZE: usize = 40;

#[cfg(test)]
mod agg_prometheus_bytes_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agg_prometheus_bytes_stats>();
        let align = align_of::<jb_logging__agg_prometheus_bytes_stats>();
        let padded_raw_size = (AGG_PROMETHEUS_BYTES_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__agg_prometheus_bytes_stats, _rpc_id),
            0
        );
        assert_eq!(offset_of!(jb_logging__agg_prometheus_bytes_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__agg_prometheus_bytes_stats, shard),
            4usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__agg_prometheus_bytes_stats,
                prometheus_bytes_written
            ),
            8usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__agg_prometheus_bytes_stats,
                prometheus_bytes_discarded
            ),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_prometheus_bytes_stats, time_ns),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_prometheus_bytes_stats, _ref),
            32usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agg_otlp_grpc_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub bytes_failed: u64,
    pub bytes_sent: u64,
    pub data_points_failed: u64,
    pub data_points_sent: u64,
    pub requests_failed: u64,
    pub requests_sent: u64,
    pub unknown_response_tags: u64,
    pub time_ns: u64,
    pub _ref: u64,
}

impl jb_logging__agg_otlp_grpc_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(644u16, true)
    }
}

impl Default for jb_logging__agg_otlp_grpc_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGG_OTLP_GRPC_STATS_WIRE_SIZE: usize = 80;

#[cfg(test)]
mod agg_otlp_grpc_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agg_otlp_grpc_stats>();
        let align = align_of::<jb_logging__agg_otlp_grpc_stats>();
        let padded_raw_size = (AGG_OTLP_GRPC_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__agg_otlp_grpc_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__agg_otlp_grpc_stats, _len), 2);
        assert_eq!(offset_of!(jb_logging__agg_otlp_grpc_stats, module), 4usize);
        assert_eq!(offset_of!(jb_logging__agg_otlp_grpc_stats, shard), 6usize);
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, bytes_failed),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, bytes_sent),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, data_points_failed),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, data_points_sent),
            32usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, requests_failed),
            40usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, requests_sent),
            48usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, unknown_response_tags),
            56usize
        );
        assert_eq!(
            offset_of!(jb_logging__agg_otlp_grpc_stats, time_ns),
            64usize
        );
        assert_eq!(offset_of!(jb_logging__agg_otlp_grpc_stats, _ref), 72usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__ingest_core_stats_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__ingest_core_stats_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(633u16, 16, true)
    }
}

impl Default for jb_logging__ingest_core_stats_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const INGEST_CORE_STATS_START_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod ingest_core_stats_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__ingest_core_stats_start>();
        let align = align_of::<jb_logging__ingest_core_stats_start>();
        let padded_raw_size = (INGEST_CORE_STATS_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__ingest_core_stats_start, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_logging__ingest_core_stats_start, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__ingest_core_stats_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_logging__ingest_core_stats_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(634u16, 16, true)
    }
}

impl Default for jb_logging__ingest_core_stats_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const INGEST_CORE_STATS_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod ingest_core_stats_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__ingest_core_stats_end>();
        let align = align_of::<jb_logging__ingest_core_stats_end>();
        let padded_raw_size = (INGEST_CORE_STATS_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__ingest_core_stats_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__ingest_core_stats_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__client_handle_pool_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub client_handle_pool: u64,
    pub client_handle_pool_fraction: u64,
    pub _ref: u64,
    pub span_name: u16,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub agent_hostname: u16,
    pub os: u16,
}

impl jb_logging__client_handle_pool_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(635u16, true)
    }
}

impl Default for jb_logging__client_handle_pool_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CLIENT_HANDLE_POOL_STATS_WIRE_SIZE: usize = 62;

#[cfg(test)]
mod client_handle_pool_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__client_handle_pool_stats>();
        let align = align_of::<jb_logging__client_handle_pool_stats>();
        let padded_raw_size = (CLIENT_HANDLE_POOL_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__client_handle_pool_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__client_handle_pool_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, module),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, client_handle_pool),
            16usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__client_handle_pool_stats,
                client_handle_pool_fraction
            ),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, _ref),
            32usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, span_name),
            40usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, version),
            42usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, cloud),
            44usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, env),
            46usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, role),
            48usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, az),
            50usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, node_id),
            52usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, kernel_version),
            54usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, client_type),
            56usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, agent_hostname),
            58usize
        );
        assert_eq!(
            offset_of!(jb_logging__client_handle_pool_stats, os),
            60usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agent_connection_message_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub count: u64,
    pub _ref: u64,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub agent_hostname: u16,
    pub os: u16,
    pub os_version: u16,
    pub severity_: u16,
}

impl jb_logging__agent_connection_message_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(636u16, true)
    }
}

impl Default for jb_logging__agent_connection_message_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_CONNECTION_MESSAGE_STATS_WIRE_SIZE: usize = 56;

#[cfg(test)]
mod agent_connection_message_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agent_connection_message_stats>();
        let align = align_of::<jb_logging__agent_connection_message_stats>();
        let padded_raw_size =
            (AGENT_CONNECTION_MESSAGE_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, _len),
            2
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, module),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, count),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, _ref),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, version),
            32usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, cloud),
            34usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, env),
            36usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, role),
            38usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, az),
            40usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, node_id),
            42usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, kernel_version),
            44usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, client_type),
            46usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, agent_hostname),
            48usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, os),
            50usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, os_version),
            52usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_stats, severity_),
            54usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__agent_connection_message_error_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub count: u64,
    pub _ref: u64,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub agent_hostname: u16,
    pub os: u16,
    pub os_version: u16,
    pub message: u16,
}

impl jb_logging__agent_connection_message_error_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(637u16, true)
    }
}

impl Default for jb_logging__agent_connection_message_error_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_CONNECTION_MESSAGE_ERROR_STATS_WIRE_SIZE: usize = 56;

#[cfg(test)]
mod agent_connection_message_error_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__agent_connection_message_error_stats>();
        let align = align_of::<jb_logging__agent_connection_message_error_stats>();
        let padded_raw_size =
            (AGENT_CONNECTION_MESSAGE_ERROR_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, _len),
            2
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, module),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, count),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, _ref),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, version),
            32usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, cloud),
            34usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, env),
            36usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, role),
            38usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, az),
            40usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, node_id),
            42usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__agent_connection_message_error_stats,
                kernel_version
            ),
            44usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__agent_connection_message_error_stats,
                client_type
            ),
            46usize
        );
        assert_eq!(
            offset_of!(
                jb_logging__agent_connection_message_error_stats,
                agent_hostname
            ),
            48usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, os),
            50usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, os_version),
            52usize
        );
        assert_eq!(
            offset_of!(jb_logging__agent_connection_message_error_stats, message),
            54usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__connection_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub time_since_last_message_ns: u64,
    pub clock_offset_ns: u64,
    pub _ref: u64,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub agent_hostname: u16,
    pub os: u16,
}

impl jb_logging__connection_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(638u16, true)
    }
}

impl Default for jb_logging__connection_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONNECTION_STATS_WIRE_SIZE: usize = 60;

#[cfg(test)]
mod connection_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__connection_stats>();
        let align = align_of::<jb_logging__connection_stats>();
        let padded_raw_size = (CONNECTION_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__connection_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__connection_stats, _len), 2);
        assert_eq!(offset_of!(jb_logging__connection_stats, module), 4usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, shard), 6usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, time_ns), 8usize);
        assert_eq!(
            offset_of!(jb_logging__connection_stats, time_since_last_message_ns),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_stats, clock_offset_ns),
            24usize
        );
        assert_eq!(offset_of!(jb_logging__connection_stats, _ref), 32usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, version), 40usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, cloud), 42usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, env), 44usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, role), 46usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, az), 48usize);
        assert_eq!(offset_of!(jb_logging__connection_stats, node_id), 50usize);
        assert_eq!(
            offset_of!(jb_logging__connection_stats, kernel_version),
            52usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_stats, client_type),
            54usize
        );
        assert_eq!(
            offset_of!(jb_logging__connection_stats, agent_hostname),
            56usize
        );
        assert_eq!(offset_of!(jb_logging__connection_stats, os), 58usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__collector_log_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub count: u32,
    pub time_ns: u64,
    pub _ref: u64,
    pub module: u16,
    pub shard: u16,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub agent_hostname: u16,
    pub os: u16,
    pub os_version: u16,
}

impl jb_logging__collector_log_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(639u16, true)
    }
}

impl Default for jb_logging__collector_log_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const COLLECTOR_LOG_STATS_WIRE_SIZE: usize = 50;

#[cfg(test)]
mod collector_log_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__collector_log_stats>();
        let align = align_of::<jb_logging__collector_log_stats>();
        let padded_raw_size = (COLLECTOR_LOG_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__collector_log_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, _len), 2);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, count), 4usize);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, time_ns), 8usize);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, _ref), 16usize);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, module), 24usize);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, shard), 26usize);
        assert_eq!(
            offset_of!(jb_logging__collector_log_stats, version),
            28usize
        );
        assert_eq!(offset_of!(jb_logging__collector_log_stats, cloud), 30usize);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, env), 32usize);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, role), 34usize);
        assert_eq!(offset_of!(jb_logging__collector_log_stats, az), 36usize);
        assert_eq!(
            offset_of!(jb_logging__collector_log_stats, node_id),
            38usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_log_stats, kernel_version),
            40usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_log_stats, client_type),
            42usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_log_stats, agent_hostname),
            44usize
        );
        assert_eq!(offset_of!(jb_logging__collector_log_stats, os), 46usize);
        assert_eq!(
            offset_of!(jb_logging__collector_log_stats, os_version),
            48usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__entry_point_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub _ref: u64,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub agent_hostname: u16,
    pub os: u16,
    pub os_version: u16,
    pub kernel_headers_source: u16,
    pub entrypoint_info: u8,
}

impl jb_logging__entry_point_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(640u16, true)
    }
}

impl Default for jb_logging__entry_point_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const ENTRY_POINT_STATS_WIRE_SIZE: usize = 49;

#[cfg(test)]
mod entry_point_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__entry_point_stats>();
        let align = align_of::<jb_logging__entry_point_stats>();
        let padded_raw_size = (ENTRY_POINT_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__entry_point_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, _len), 2);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, module), 4usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, shard), 6usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, time_ns), 8usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, _ref), 16usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, version), 24usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, cloud), 26usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, env), 28usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, role), 30usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, az), 32usize);
        assert_eq!(offset_of!(jb_logging__entry_point_stats, node_id), 34usize);
        assert_eq!(
            offset_of!(jb_logging__entry_point_stats, kernel_version),
            36usize
        );
        assert_eq!(
            offset_of!(jb_logging__entry_point_stats, client_type),
            38usize
        );
        assert_eq!(
            offset_of!(jb_logging__entry_point_stats, agent_hostname),
            40usize
        );
        assert_eq!(offset_of!(jb_logging__entry_point_stats, os), 42usize);
        assert_eq!(
            offset_of!(jb_logging__entry_point_stats, os_version),
            44usize
        );
        assert_eq!(
            offset_of!(jb_logging__entry_point_stats, kernel_headers_source),
            46usize
        );
        assert_eq!(
            offset_of!(jb_logging__entry_point_stats, entrypoint_info),
            48usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__collector_health_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub _ref: u64,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub hostname: u16,
    pub os: u16,
    pub os_version: u16,
    pub status: u16,
}

impl jb_logging__collector_health_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(641u16, true)
    }
}

impl Default for jb_logging__collector_health_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const COLLECTOR_HEALTH_STATS_WIRE_SIZE: usize = 48;

#[cfg(test)]
mod collector_health_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__collector_health_stats>();
        let align = align_of::<jb_logging__collector_health_stats>();
        let padded_raw_size = (COLLECTOR_HEALTH_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__collector_health_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__collector_health_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, module),
            4usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, shard),
            6usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, time_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, _ref),
            16usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, version),
            24usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, cloud),
            26usize
        );
        assert_eq!(offset_of!(jb_logging__collector_health_stats, env), 28usize);
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, role),
            30usize
        );
        assert_eq!(offset_of!(jb_logging__collector_health_stats, az), 32usize);
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, node_id),
            34usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, kernel_version),
            36usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, client_type),
            38usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, hostname),
            40usize
        );
        assert_eq!(offset_of!(jb_logging__collector_health_stats, os), 42usize);
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, os_version),
            44usize
        );
        assert_eq!(
            offset_of!(jb_logging__collector_health_stats, status),
            46usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__bpf_log_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub module: u16,
    pub shard: u16,
    pub time_ns: u64,
    pub _ref: u64,
    pub version: u16,
    pub cloud: u16,
    pub env: u16,
    pub role: u16,
    pub az: u16,
    pub node_id: u16,
    pub kernel_version: u16,
    pub client_type: u16,
    pub hostname: u16,
    pub os: u16,
    pub os_version: u16,
    pub filename: u16,
    pub line: u16,
    pub code: u16,
    pub arg0: u16,
    pub arg1: u16,
}

impl jb_logging__bpf_log_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(642u16, true)
    }
}

impl Default for jb_logging__bpf_log_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const BPF_LOG_STATS_WIRE_SIZE: usize = 56;

#[cfg(test)]
mod bpf_log_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__bpf_log_stats>();
        let align = align_of::<jb_logging__bpf_log_stats>();
        let padded_raw_size = (BPF_LOG_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, _len), 2);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, module), 4usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, shard), 6usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, time_ns), 8usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, _ref), 16usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, version), 24usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, cloud), 26usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, env), 28usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, role), 30usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, az), 32usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, node_id), 34usize);
        assert_eq!(
            offset_of!(jb_logging__bpf_log_stats, kernel_version),
            36usize
        );
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, client_type), 38usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, hostname), 40usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, os), 42usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, os_version), 44usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, filename), 46usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, line), 48usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, code), 50usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, arg0), 52usize);
        assert_eq!(offset_of!(jb_logging__bpf_log_stats, arg1), 54usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__server_stats {
    pub _rpc_id: u16,
    pub _len: u16,
    pub connection_counter: u64,
    pub disconnect_counter: u64,
    pub time_ns: u64,
    pub _ref: u64,
}

impl jb_logging__server_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(643u16, true)
    }
}

impl Default for jb_logging__server_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SERVER_STATS_WIRE_SIZE: usize = 40;

#[cfg(test)]
mod server_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_logging__server_stats>();
        let align = align_of::<jb_logging__server_stats>();
        let padded_raw_size = (SERVER_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__server_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_logging__server_stats, _len), 2);
        assert_eq!(
            offset_of!(jb_logging__server_stats, connection_counter),
            8usize
        );
        assert_eq!(
            offset_of!(jb_logging__server_stats, disconnect_counter),
            16usize
        );
        assert_eq!(offset_of!(jb_logging__server_stats, time_ns), 24usize);
        assert_eq!(offset_of!(jb_logging__server_stats, _ref), 32usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_logging__pulse {
    pub _rpc_id: u16,
}

impl jb_logging__pulse {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(65535u16, 2, true)
    }
}

impl Default for jb_logging__pulse {
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
        let size = size_of::<jb_logging__pulse>();
        let align = align_of::<jb_logging__pulse>();
        let padded_raw_size = (PULSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_logging__pulse, _rpc_id), 0);
    }
}

#[inline]
pub fn all_message_metadata() -> ::std::vec::Vec<render_parser::MessageMetadata> {
    ::std::vec![
        jb_logging__logger_start::metadata(),
        jb_logging__logger_end::metadata(),
        jb_logging__agent_lost_events::metadata(),
        jb_logging__pod_not_found::metadata(),
        jb_logging__cgroup_not_found::metadata(),
        jb_logging__rewriting_private_to_public_ip_mapping::metadata(),
        jb_logging__private_ip_in_private_to_public_ip_mapping::metadata(),
        jb_logging__failed_to_insert_dns_record::metadata(),
        jb_logging__tcp_socket_failed_getting_process_reference::metadata(),
        jb_logging__udp_socket_failed_getting_process_reference::metadata(),
        jb_logging__socket_address_already_assigned::metadata(),
        jb_logging__ingest_decompression_error::metadata(),
        jb_logging__ingest_processing_error::metadata(),
        jb_logging__ingest_connection_error::metadata(),
        jb_logging__agent_auth_success::metadata(),
        jb_logging__agent_auth_failure::metadata(),
        jb_logging__agent_attempting_auth_using_api_key::metadata(),
        jb_logging__k8s_container_pod_not_found::metadata(),
        jb_logging__agent_connect_success::metadata(),
        jb_logging__core_stats_start::metadata(),
        jb_logging__core_stats_end::metadata(),
        jb_logging__span_utilization_stats::metadata(),
        jb_logging__connection_message_stats::metadata(),
        jb_logging__connection_message_error_stats::metadata(),
        jb_logging__status_stats::metadata(),
        jb_logging__rpc_receive_stats::metadata(),
        jb_logging__rpc_write_stalls_stats::metadata(),
        jb_logging__rpc_write_utilization_stats::metadata(),
        jb_logging__code_timing_stats::metadata(),
        jb_logging__agg_core_stats_start::metadata(),
        jb_logging__agg_core_stats_end::metadata(),
        jb_logging__agg_root_truncation_stats::metadata(),
        jb_logging__agg_prometheus_bytes_stats::metadata(),
        jb_logging__agg_otlp_grpc_stats::metadata(),
        jb_logging__ingest_core_stats_start::metadata(),
        jb_logging__ingest_core_stats_end::metadata(),
        jb_logging__client_handle_pool_stats::metadata(),
        jb_logging__agent_connection_message_stats::metadata(),
        jb_logging__agent_connection_message_error_stats::metadata(),
        jb_logging__connection_stats::metadata(),
        jb_logging__collector_log_stats::metadata(),
        jb_logging__entry_point_stats::metadata(),
        jb_logging__collector_health_stats::metadata(),
        jb_logging__bpf_log_stats::metadata(),
        jb_logging__server_stats::metadata(),
        jb_logging__pulse::metadata(),
    ]
}
