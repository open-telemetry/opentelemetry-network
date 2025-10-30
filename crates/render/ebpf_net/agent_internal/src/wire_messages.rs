/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__dns_packet {
    pub _rpc_id: u16,
    pub _len: u16,
    pub total_len: u16,
    pub is_rx: u8,
    pub sk: u64,
}

impl jb_agent_internal__dns_packet {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(331u16, true)
    }
}

impl Default for jb_agent_internal__dns_packet {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const DNS_PACKET_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod dns_packet_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__dns_packet>();
        let align = align_of::<jb_agent_internal__dns_packet>();
        let padded_raw_size = (DNS_PACKET_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__dns_packet, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__dns_packet, _len), 2);
        assert_eq!(offset_of!(jb_agent_internal__dns_packet, total_len), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__dns_packet, is_rx), 6usize);
        assert_eq!(offset_of!(jb_agent_internal__dns_packet, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__reset_tcp_counters {
    pub _rpc_id: u16,
    pub packets_delivered: u32,
    pub sk: u64,
    pub bytes_acked: u64,
    pub bytes_received: u64,
    pub packets_retrans: u32,
    pub pid: u32,
}

impl jb_agent_internal__reset_tcp_counters {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(332u16, 40, true)
    }
}

impl Default for jb_agent_internal__reset_tcp_counters {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const RESET_TCP_COUNTERS_WIRE_SIZE: usize = 40;

#[cfg(test)]
mod reset_tcp_counters_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__reset_tcp_counters>();
        let align = align_of::<jb_agent_internal__reset_tcp_counters>();
        let padded_raw_size = (RESET_TCP_COUNTERS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__reset_tcp_counters, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__reset_tcp_counters, packets_delivered),
            4usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__reset_tcp_counters, sk),
            8usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__reset_tcp_counters, bytes_acked),
            16usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__reset_tcp_counters, bytes_received),
            24usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__reset_tcp_counters, packets_retrans),
            32usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__reset_tcp_counters, pid),
            36usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__new_sock_created {
    pub _rpc_id: u16,
    pub pid: u32,
    pub sk: u64,
}

impl jb_agent_internal__new_sock_created {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(333u16, 16, true)
    }
}

impl Default for jb_agent_internal__new_sock_created {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NEW_SOCK_CREATED_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod new_sock_created_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__new_sock_created>();
        let align = align_of::<jb_agent_internal__new_sock_created>();
        let padded_raw_size = (NEW_SOCK_CREATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__new_sock_created, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__new_sock_created, pid), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__new_sock_created, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__udp_new_socket {
    pub _rpc_id: u16,
    pub lport: u16,
    pub pid: u32,
    pub sk: u64,
    pub laddr: [u8; 16],
}

impl jb_agent_internal__udp_new_socket {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(334u16, 32, true)
    }
}

impl Default for jb_agent_internal__udp_new_socket {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_NEW_SOCKET_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod udp_new_socket_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__udp_new_socket>();
        let align = align_of::<jb_agent_internal__udp_new_socket>();
        let padded_raw_size = (UDP_NEW_SOCKET_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__udp_new_socket, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__udp_new_socket, lport), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_new_socket, pid), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_new_socket, sk), 8usize);
        assert_eq!(
            offset_of!(jb_agent_internal__udp_new_socket, laddr),
            16usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__udp_destroy_socket {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl jb_agent_internal__udp_destroy_socket {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(335u16, 16, true)
    }
}

impl Default for jb_agent_internal__udp_destroy_socket {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_DESTROY_SOCKET_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod udp_destroy_socket_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__udp_destroy_socket>();
        let align = align_of::<jb_agent_internal__udp_destroy_socket>();
        let padded_raw_size = (UDP_DESTROY_SOCKET_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__udp_destroy_socket, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__udp_destroy_socket, sk),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__udp_stats {
    pub _rpc_id: u16,
    pub rport: u16,
    pub packets: u32,
    pub sk: u64,
    pub bytes: u32,
    pub drops: u32,
    pub lport: u16,
    pub raddr: [u8; 16],
    pub changed_af: u8,
    pub is_rx: u8,
    pub laddr: [u8; 16],
}

impl jb_agent_internal__udp_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(336u16, 60, true)
    }
}

impl Default for jb_agent_internal__udp_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_STATS_WIRE_SIZE: usize = 60;

#[cfg(test)]
mod udp_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__udp_stats>();
        let align = align_of::<jb_agent_internal__udp_stats>();
        let padded_raw_size = (UDP_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, rport), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, packets), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, sk), 8usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, bytes), 16usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, drops), 20usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, lport), 24usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, raddr), 26usize);
        assert_eq!(
            offset_of!(jb_agent_internal__udp_stats, changed_af),
            42usize
        );
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, is_rx), 43usize);
        assert_eq!(offset_of!(jb_agent_internal__udp_stats, laddr), 44usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__pid_info {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
    pub cgroup: u64,
    pub parent_pid: i32,
}

impl jb_agent_internal__pid_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(337u16, 36, true)
    }
}

impl Default for jb_agent_internal__pid_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_INFO_WIRE_SIZE: usize = 36;

#[cfg(test)]
mod pid_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__pid_info>();
        let align = align_of::<jb_agent_internal__pid_info>();
        let padded_raw_size = (PID_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__pid_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__pid_info, comm), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__pid_info, pid), 20usize);
        assert_eq!(offset_of!(jb_agent_internal__pid_info, cgroup), 24usize);
        assert_eq!(offset_of!(jb_agent_internal__pid_info, parent_pid), 32usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__pid_close {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
}

impl jb_agent_internal__pid_close {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(338u16, 24, true)
    }
}

impl Default for jb_agent_internal__pid_close {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_CLOSE_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod pid_close_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__pid_close>();
        let align = align_of::<jb_agent_internal__pid_close>();
        let padded_raw_size = (PID_CLOSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__pid_close, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__pid_close, comm), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__pid_close, pid), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__pid_set_comm {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
}

impl jb_agent_internal__pid_set_comm {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(371u16, 24, true)
    }
}

impl Default for jb_agent_internal__pid_set_comm {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_SET_COMM_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod pid_set_comm_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__pid_set_comm>();
        let align = align_of::<jb_agent_internal__pid_set_comm>();
        let padded_raw_size = (PID_SET_COMM_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__pid_set_comm, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__pid_set_comm, comm), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__pid_set_comm, pid), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__set_state_ipv4 {
    pub _rpc_id: u16,
    pub dport: u16,
    pub dest: u32,
    pub sk: u64,
    pub src: u32,
    pub tx_rx: u32,
    pub sport: u16,
}

impl jb_agent_internal__set_state_ipv4 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(339u16, 26, true)
    }
}

impl Default for jb_agent_internal__set_state_ipv4 {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_STATE_IPV4_WIRE_SIZE: usize = 26;

#[cfg(test)]
mod set_state_ipv4_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__set_state_ipv4>();
        let align = align_of::<jb_agent_internal__set_state_ipv4>();
        let padded_raw_size = (SET_STATE_IPV4_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv4, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv4, dport), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv4, dest), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv4, sk), 8usize);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv4, src), 16usize);
        assert_eq!(
            offset_of!(jb_agent_internal__set_state_ipv4, tx_rx),
            20usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__set_state_ipv4, sport),
            24usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__set_state_ipv6 {
    pub _rpc_id: u16,
    pub dport: u16,
    pub tx_rx: u32,
    pub sk: u64,
    pub sport: u16,
    pub dest: [u8; 16],
    pub src: [u8; 16],
}

impl jb_agent_internal__set_state_ipv6 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(340u16, 50, true)
    }
}

impl Default for jb_agent_internal__set_state_ipv6 {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_STATE_IPV6_WIRE_SIZE: usize = 50;

#[cfg(test)]
mod set_state_ipv6_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__set_state_ipv6>();
        let align = align_of::<jb_agent_internal__set_state_ipv6>();
        let padded_raw_size = (SET_STATE_IPV6_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv6, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv6, dport), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv6, tx_rx), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv6, sk), 8usize);
        assert_eq!(
            offset_of!(jb_agent_internal__set_state_ipv6, sport),
            16usize
        );
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv6, dest), 18usize);
        assert_eq!(offset_of!(jb_agent_internal__set_state_ipv6, src), 34usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__rtt_estimator {
    pub _rpc_id: u16,
    pub ca_state: u8,
    pub srtt: u32,
    pub bytes_acked: u64,
    pub sk: u64,
    pub bytes_received: u64,
    pub snd_cwnd: u32,
    pub packets_in_flight: u32,
    pub packets_delivered: u32,
    pub packets_retrans: u32,
    pub rcv_holes: u32,
    pub rcv_delivered: u32,
    pub rcv_rtt: u32,
}

impl jb_agent_internal__rtt_estimator {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(361u16, 60, true)
    }
}

impl Default for jb_agent_internal__rtt_estimator {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const RTT_ESTIMATOR_WIRE_SIZE: usize = 60;

#[cfg(test)]
mod rtt_estimator_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__rtt_estimator>();
        let align = align_of::<jb_agent_internal__rtt_estimator>();
        let padded_raw_size = (RTT_ESTIMATOR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__rtt_estimator, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, ca_state),
            2usize
        );
        assert_eq!(offset_of!(jb_agent_internal__rtt_estimator, srtt), 4usize);
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, bytes_acked),
            8usize
        );
        assert_eq!(offset_of!(jb_agent_internal__rtt_estimator, sk), 16usize);
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, bytes_received),
            24usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, snd_cwnd),
            32usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, packets_in_flight),
            36usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, packets_delivered),
            40usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, packets_retrans),
            44usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, rcv_holes),
            48usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, rcv_delivered),
            52usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__rtt_estimator, rcv_rtt),
            56usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__close_sock_info {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl jb_agent_internal__close_sock_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(362u16, 16, true)
    }
}

impl Default for jb_agent_internal__close_sock_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CLOSE_SOCK_INFO_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod close_sock_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__close_sock_info>();
        let align = align_of::<jb_agent_internal__close_sock_info>();
        let padded_raw_size = (CLOSE_SOCK_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__close_sock_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__close_sock_info, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__kill_css {
    pub _rpc_id: u16,
    pub name: [u8; 256],
    pub cgroup: u64,
    pub cgroup_parent: u64,
}

impl jb_agent_internal__kill_css {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(363u16, 280, true)
    }
}

impl Default for jb_agent_internal__kill_css {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const KILL_CSS_WIRE_SIZE: usize = 280;

#[cfg(test)]
mod kill_css_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__kill_css>();
        let align = align_of::<jb_agent_internal__kill_css>();
        let padded_raw_size = (KILL_CSS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__kill_css, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__kill_css, name), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__kill_css, cgroup), 264usize);
        assert_eq!(
            offset_of!(jb_agent_internal__kill_css, cgroup_parent),
            272usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__css_populate_dir {
    pub _rpc_id: u16,
    pub name: [u8; 256],
    pub cgroup: u64,
    pub cgroup_parent: u64,
}

impl jb_agent_internal__css_populate_dir {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(364u16, 280, true)
    }
}

impl Default for jb_agent_internal__css_populate_dir {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CSS_POPULATE_DIR_WIRE_SIZE: usize = 280;

#[cfg(test)]
mod css_populate_dir_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__css_populate_dir>();
        let align = align_of::<jb_agent_internal__css_populate_dir>();
        let padded_raw_size = (CSS_POPULATE_DIR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__css_populate_dir, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_agent_internal__css_populate_dir, name),
            2usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__css_populate_dir, cgroup),
            264usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__css_populate_dir, cgroup_parent),
            272usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__existing_cgroup_probe {
    pub _rpc_id: u16,
    pub name: [u8; 256],
    pub cgroup: u64,
    pub cgroup_parent: u64,
}

impl jb_agent_internal__existing_cgroup_probe {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(365u16, 280, true)
    }
}

impl Default for jb_agent_internal__existing_cgroup_probe {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const EXISTING_CGROUP_PROBE_WIRE_SIZE: usize = 280;

#[cfg(test)]
mod existing_cgroup_probe_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__existing_cgroup_probe>();
        let align = align_of::<jb_agent_internal__existing_cgroup_probe>();
        let padded_raw_size = (EXISTING_CGROUP_PROBE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__existing_cgroup_probe, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_cgroup_probe, name),
            2usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_cgroup_probe, cgroup),
            264usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_cgroup_probe, cgroup_parent),
            272usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__cgroup_attach_task {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
    pub cgroup: u64,
}

impl jb_agent_internal__cgroup_attach_task {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(366u16, 32, true)
    }
}

impl Default for jb_agent_internal__cgroup_attach_task {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CGROUP_ATTACH_TASK_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod cgroup_attach_task_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__cgroup_attach_task>();
        let align = align_of::<jb_agent_internal__cgroup_attach_task>();
        let padded_raw_size = (CGROUP_ATTACH_TASK_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__cgroup_attach_task, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__cgroup_attach_task, comm),
            2usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__cgroup_attach_task, pid),
            20usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__cgroup_attach_task, cgroup),
            24usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__nf_conntrack_alter_reply {
    pub _rpc_id: u16,
    pub src_port: u16,
    pub src_ip: u32,
    pub ct: u64,
    pub dst_ip: u32,
    pub nat_src_ip: u32,
    pub nat_dst_ip: u32,
    pub dst_port: u16,
    pub nat_src_port: u16,
    pub nat_dst_port: u16,
    pub proto: u8,
    pub nat_proto: u8,
}

impl jb_agent_internal__nf_conntrack_alter_reply {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(367u16, 36, true)
    }
}

impl Default for jb_agent_internal__nf_conntrack_alter_reply {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NF_CONNTRACK_ALTER_REPLY_WIRE_SIZE: usize = 36;

#[cfg(test)]
mod nf_conntrack_alter_reply_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__nf_conntrack_alter_reply>();
        let align = align_of::<jb_agent_internal__nf_conntrack_alter_reply>();
        let padded_raw_size = (NF_CONNTRACK_ALTER_REPLY_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, src_port),
            2usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, src_ip),
            4usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, ct),
            8usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, dst_ip),
            16usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, nat_src_ip),
            20usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, nat_dst_ip),
            24usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, dst_port),
            28usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, nat_src_port),
            30usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, nat_dst_port),
            32usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, proto),
            34usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_conntrack_alter_reply, nat_proto),
            35usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__nf_nat_cleanup_conntrack {
    pub _rpc_id: u16,
    pub src_port: u16,
    pub src_ip: u32,
    pub ct: u64,
    pub dst_ip: u32,
    pub dst_port: u16,
    pub proto: u8,
}

impl jb_agent_internal__nf_nat_cleanup_conntrack {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(368u16, 23, true)
    }
}

impl Default for jb_agent_internal__nf_nat_cleanup_conntrack {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NF_NAT_CLEANUP_CONNTRACK_WIRE_SIZE: usize = 23;

#[cfg(test)]
mod nf_nat_cleanup_conntrack_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__nf_nat_cleanup_conntrack>();
        let align = align_of::<jb_agent_internal__nf_nat_cleanup_conntrack>();
        let padded_raw_size = (NF_NAT_CLEANUP_CONNTRACK_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__nf_nat_cleanup_conntrack, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_nat_cleanup_conntrack, src_port),
            2usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_nat_cleanup_conntrack, src_ip),
            4usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_nat_cleanup_conntrack, ct),
            8usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_nat_cleanup_conntrack, dst_ip),
            16usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_nat_cleanup_conntrack, dst_port),
            20usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__nf_nat_cleanup_conntrack, proto),
            22usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__existing_conntrack_tuple {
    pub _rpc_id: u16,
    pub src_port: u16,
    pub src_ip: u32,
    pub ct: u64,
    pub dst_ip: u32,
    pub dst_port: u16,
    pub proto: u8,
    pub dir: u8,
}

impl jb_agent_internal__existing_conntrack_tuple {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(369u16, 24, true)
    }
}

impl Default for jb_agent_internal__existing_conntrack_tuple {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const EXISTING_CONNTRACK_TUPLE_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod existing_conntrack_tuple_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__existing_conntrack_tuple>();
        let align = align_of::<jb_agent_internal__existing_conntrack_tuple>();
        let padded_raw_size = (EXISTING_CONNTRACK_TUPLE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, src_port),
            2usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, src_ip),
            4usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, ct),
            8usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, dst_ip),
            16usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, dst_port),
            20usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, proto),
            22usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__existing_conntrack_tuple, dir),
            23usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__tcp_syn_timeout {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl jb_agent_internal__tcp_syn_timeout {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(370u16, 16, true)
    }
}

impl Default for jb_agent_internal__tcp_syn_timeout {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TCP_SYN_TIMEOUT_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod tcp_syn_timeout_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__tcp_syn_timeout>();
        let align = align_of::<jb_agent_internal__tcp_syn_timeout>();
        let padded_raw_size = (TCP_SYN_TIMEOUT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__tcp_syn_timeout, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__tcp_syn_timeout, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__http_response {
    pub _rpc_id: u16,
    pub code: u16,
    pub pid: u32,
    pub sk: u64,
    pub latency_ns: u64,
    pub client_server: u8,
}

impl jb_agent_internal__http_response {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(372u16, 25, true)
    }
}

impl Default for jb_agent_internal__http_response {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const HTTP_RESPONSE_WIRE_SIZE: usize = 25;

#[cfg(test)]
mod http_response_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__http_response>();
        let align = align_of::<jb_agent_internal__http_response>();
        let padded_raw_size = (HTTP_RESPONSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__http_response, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__http_response, code), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__http_response, pid), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__http_response, sk), 8usize);
        assert_eq!(
            offset_of!(jb_agent_internal__http_response, latency_ns),
            16usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__http_response, client_server),
            24usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__bpf_log {
    pub _rpc_id: u16,
    pub filelineid: u64,
    pub code: u64,
    pub arg0: u64,
    pub arg1: u64,
    pub arg2: u64,
}

impl jb_agent_internal__bpf_log {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(373u16, 48, true)
    }
}

impl Default for jb_agent_internal__bpf_log {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const BPF_LOG_WIRE_SIZE: usize = 48;

#[cfg(test)]
mod bpf_log_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__bpf_log>();
        let align = align_of::<jb_agent_internal__bpf_log>();
        let padded_raw_size = (BPF_LOG_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__bpf_log, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__bpf_log, filelineid), 8usize);
        assert_eq!(offset_of!(jb_agent_internal__bpf_log, code), 16usize);
        assert_eq!(offset_of!(jb_agent_internal__bpf_log, arg0), 24usize);
        assert_eq!(offset_of!(jb_agent_internal__bpf_log, arg1), 32usize);
        assert_eq!(offset_of!(jb_agent_internal__bpf_log, arg2), 40usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__stack_trace {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub kernel_stack_id: i32,
    pub user_stack_id: i32,
    pub tgid: u32,
}

impl jb_agent_internal__stack_trace {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(374u16, 32, true)
    }
}

impl Default for jb_agent_internal__stack_trace {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const STACK_TRACE_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod stack_trace_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__stack_trace>();
        let align = align_of::<jb_agent_internal__stack_trace>();
        let padded_raw_size = (STACK_TRACE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__stack_trace, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__stack_trace, comm), 2usize);
        assert_eq!(
            offset_of!(jb_agent_internal__stack_trace, kernel_stack_id),
            20usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__stack_trace, user_stack_id),
            24usize
        );
        assert_eq!(offset_of!(jb_agent_internal__stack_trace, tgid), 28usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__tcp_data {
    pub _rpc_id: u16,
    pub stream_type: u8,
    pub client_server: u8,
    pub pid: u32,
    pub sk: u64,
    pub offset: u64,
    pub length: u32,
}

impl jb_agent_internal__tcp_data {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(375u16, 28, true)
    }
}

impl Default for jb_agent_internal__tcp_data {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TCP_DATA_WIRE_SIZE: usize = 28;

#[cfg(test)]
mod tcp_data_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__tcp_data>();
        let align = align_of::<jb_agent_internal__tcp_data>();
        let padded_raw_size = (TCP_DATA_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__tcp_data, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__tcp_data, stream_type), 2usize);
        assert_eq!(
            offset_of!(jb_agent_internal__tcp_data, client_server),
            3usize
        );
        assert_eq!(offset_of!(jb_agent_internal__tcp_data, pid), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__tcp_data, sk), 8usize);
        assert_eq!(offset_of!(jb_agent_internal__tcp_data, offset), 16usize);
        assert_eq!(offset_of!(jb_agent_internal__tcp_data, length), 24usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__pid_exit {
    pub _rpc_id: u16,
    pub pid: u32,
    pub tgid: u64,
    pub exit_code: i32,
}

impl jb_agent_internal__pid_exit {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(377u16, 20, true)
    }
}

impl Default for jb_agent_internal__pid_exit {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_EXIT_WIRE_SIZE: usize = 20;

#[cfg(test)]
mod pid_exit_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__pid_exit>();
        let align = align_of::<jb_agent_internal__pid_exit>();
        let padded_raw_size = (PID_EXIT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__pid_exit, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__pid_exit, pid), 4usize);
        assert_eq!(offset_of!(jb_agent_internal__pid_exit, tgid), 8usize);
        assert_eq!(offset_of!(jb_agent_internal__pid_exit, exit_code), 16usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__report_debug_event {
    pub _rpc_id: u16,
    pub event: u16,
    pub arg1: u64,
    pub arg2: u64,
    pub arg3: u64,
    pub arg4: u64,
}

impl jb_agent_internal__report_debug_event {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(378u16, 40, true)
    }
}

impl Default for jb_agent_internal__report_debug_event {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const REPORT_DEBUG_EVENT_WIRE_SIZE: usize = 40;

#[cfg(test)]
mod report_debug_event_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__report_debug_event>();
        let align = align_of::<jb_agent_internal__report_debug_event>();
        let padded_raw_size = (REPORT_DEBUG_EVENT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_agent_internal__report_debug_event, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_agent_internal__report_debug_event, event),
            2usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__report_debug_event, arg1),
            8usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__report_debug_event, arg2),
            16usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__report_debug_event, arg3),
            24usize
        );
        assert_eq!(
            offset_of!(jb_agent_internal__report_debug_event, arg4),
            32usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__tcp_reset {
    pub _rpc_id: u16,
    pub is_rx: u8,
    pub sk: u64,
}

impl jb_agent_internal__tcp_reset {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(379u16, 16, true)
    }
}

impl Default for jb_agent_internal__tcp_reset {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TCP_RESET_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod tcp_reset_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_agent_internal__tcp_reset>();
        let align = align_of::<jb_agent_internal__tcp_reset>();
        let padded_raw_size = (TCP_RESET_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__tcp_reset, _rpc_id), 0);
        assert_eq!(offset_of!(jb_agent_internal__tcp_reset, is_rx), 2usize);
        assert_eq!(offset_of!(jb_agent_internal__tcp_reset, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_agent_internal__pulse {
    pub _rpc_id: u16,
}

impl jb_agent_internal__pulse {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(65535u16, 2, true)
    }
}

impl Default for jb_agent_internal__pulse {
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
        let size = size_of::<jb_agent_internal__pulse>();
        let align = align_of::<jb_agent_internal__pulse>();
        let padded_raw_size = (PULSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_agent_internal__pulse, _rpc_id), 0);
    }
}

#[inline]
pub fn all_message_metadata() -> ::std::vec::Vec<render_parser::MessageMetadata> {
    ::std::vec![
        jb_agent_internal__dns_packet::metadata(),
        jb_agent_internal__reset_tcp_counters::metadata(),
        jb_agent_internal__new_sock_created::metadata(),
        jb_agent_internal__udp_new_socket::metadata(),
        jb_agent_internal__udp_destroy_socket::metadata(),
        jb_agent_internal__udp_stats::metadata(),
        jb_agent_internal__pid_info::metadata(),
        jb_agent_internal__pid_close::metadata(),
        jb_agent_internal__pid_set_comm::metadata(),
        jb_agent_internal__set_state_ipv4::metadata(),
        jb_agent_internal__set_state_ipv6::metadata(),
        jb_agent_internal__rtt_estimator::metadata(),
        jb_agent_internal__close_sock_info::metadata(),
        jb_agent_internal__kill_css::metadata(),
        jb_agent_internal__css_populate_dir::metadata(),
        jb_agent_internal__existing_cgroup_probe::metadata(),
        jb_agent_internal__cgroup_attach_task::metadata(),
        jb_agent_internal__nf_conntrack_alter_reply::metadata(),
        jb_agent_internal__nf_nat_cleanup_conntrack::metadata(),
        jb_agent_internal__existing_conntrack_tuple::metadata(),
        jb_agent_internal__tcp_syn_timeout::metadata(),
        jb_agent_internal__http_response::metadata(),
        jb_agent_internal__bpf_log::metadata(),
        jb_agent_internal__stack_trace::metadata(),
        jb_agent_internal__tcp_data::metadata(),
        jb_agent_internal__pid_exit::metadata(),
        jb_agent_internal__report_debug_event::metadata(),
        jb_agent_internal__tcp_reset::metadata(),
        jb_agent_internal__pulse::metadata(),
    ]
}
