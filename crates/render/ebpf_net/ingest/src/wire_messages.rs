/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_info {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
}

impl jb_ingest__pid_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(301u16, 24, true)
    }
}

impl Default for jb_ingest__pid_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_INFO_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod pid_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pid_info>();
        let align = align_of::<jb_ingest__pid_info>();
        let padded_raw_size = (PID_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pid_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pid_info, comm), 2usize);
        assert_eq!(offset_of!(jb_ingest__pid_info, pid), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_close_info {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
}

impl jb_ingest__pid_close_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(306u16, 24, true)
    }
}

impl Default for jb_ingest__pid_close_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_CLOSE_INFO_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod pid_close_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pid_close_info>();
        let align = align_of::<jb_ingest__pid_close_info>();
        let padded_raw_size = (PID_CLOSE_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pid_close_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pid_close_info, comm), 2usize);
        assert_eq!(offset_of!(jb_ingest__pid_close_info, pid), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_info_create_deprecated {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
    pub cgroup: u64,
}

impl jb_ingest__pid_info_create_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(393u16, 32, true)
    }
}

impl Default for jb_ingest__pid_info_create_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_INFO_CREATE_DEPRECATED_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod pid_info_create_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pid_info_create_deprecated>();
        let align = align_of::<jb_ingest__pid_info_create_deprecated>();
        let padded_raw_size = (PID_INFO_CREATE_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__pid_info_create_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__pid_info_create_deprecated, comm),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__pid_info_create_deprecated, pid),
            20usize
        );
        assert_eq!(
            offset_of!(jb_ingest__pid_info_create_deprecated, cgroup),
            24usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_info_create {
    pub _rpc_id: u16,
    pub _len: u16,
    pub pid: u32,
    pub cgroup: u64,
    pub parent_pid: i32,
    pub comm: [u8; 16],
}

impl jb_ingest__pid_info_create {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(546u16, true)
    }
}

impl Default for jb_ingest__pid_info_create {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_INFO_CREATE_WIRE_SIZE: usize = 36;

#[cfg(test)]
mod pid_info_create_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pid_info_create>();
        let align = align_of::<jb_ingest__pid_info_create>();
        let padded_raw_size = (PID_INFO_CREATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pid_info_create, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pid_info_create, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pid_info_create, pid), 4usize);
        assert_eq!(offset_of!(jb_ingest__pid_info_create, cgroup), 8usize);
        assert_eq!(offset_of!(jb_ingest__pid_info_create, parent_pid), 16usize);
        assert_eq!(offset_of!(jb_ingest__pid_info_create, comm), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_cgroup_move {
    pub _rpc_id: u16,
    pub pid: u32,
    pub cgroup: u64,
}

impl jb_ingest__pid_cgroup_move {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(397u16, 16, true)
    }
}

impl Default for jb_ingest__pid_cgroup_move {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_CGROUP_MOVE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod pid_cgroup_move_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pid_cgroup_move>();
        let align = align_of::<jb_ingest__pid_cgroup_move>();
        let padded_raw_size = (PID_CGROUP_MOVE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pid_cgroup_move, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pid_cgroup_move, pid), 4usize);
        assert_eq!(offset_of!(jb_ingest__pid_cgroup_move, cgroup), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_set_comm {
    pub _rpc_id: u16,
    pub comm: [u8; 16],
    pub pid: u32,
}

impl jb_ingest__pid_set_comm {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(399u16, 24, true)
    }
}

impl Default for jb_ingest__pid_set_comm {
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
        let size = size_of::<jb_ingest__pid_set_comm>();
        let align = align_of::<jb_ingest__pid_set_comm>();
        let padded_raw_size = (PID_SET_COMM_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pid_set_comm, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pid_set_comm, comm), 2usize);
        assert_eq!(offset_of!(jb_ingest__pid_set_comm, pid), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_set_cmdline {
    pub _rpc_id: u16,
    pub _len: u16,
    pub pid: u32,
}

impl jb_ingest__pid_set_cmdline {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(547u16, true)
    }
}

impl Default for jb_ingest__pid_set_cmdline {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_SET_CMDLINE_WIRE_SIZE: usize = 8;

#[cfg(test)]
mod pid_set_cmdline_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pid_set_cmdline>();
        let align = align_of::<jb_ingest__pid_set_cmdline>();
        let padded_raw_size = (PID_SET_CMDLINE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pid_set_cmdline, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pid_set_cmdline, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pid_set_cmdline, pid), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__tracked_process_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_ingest__tracked_process_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(500u16, 16, true)
    }
}

impl Default for jb_ingest__tracked_process_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TRACKED_PROCESS_START_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod tracked_process_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__tracked_process_start>();
        let align = align_of::<jb_ingest__tracked_process_start>();
        let padded_raw_size = (TRACKED_PROCESS_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__tracked_process_start, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__tracked_process_start, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__tracked_process_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_ingest__tracked_process_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(501u16, 16, true)
    }
}

impl Default for jb_ingest__tracked_process_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const TRACKED_PROCESS_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod tracked_process_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__tracked_process_end>();
        let align = align_of::<jb_ingest__tracked_process_end>();
        let padded_raw_size = (TRACKED_PROCESS_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__tracked_process_end, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__tracked_process_end, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_tgid {
    pub _rpc_id: u16,
    pub tgid: u32,
    pub _ref: u64,
}

impl jb_ingest__set_tgid {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(502u16, 16, true)
    }
}

impl Default for jb_ingest__set_tgid {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_TGID_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod set_tgid_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_tgid>();
        let align = align_of::<jb_ingest__set_tgid>();
        let padded_raw_size = (SET_TGID_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_tgid, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__set_tgid, tgid), 4usize);
        assert_eq!(offset_of!(jb_ingest__set_tgid, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_cgroup {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub _ref: u64,
}

impl jb_ingest__set_cgroup {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(503u16, 24, true)
    }
}

impl Default for jb_ingest__set_cgroup {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_CGROUP_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod set_cgroup_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_cgroup>();
        let align = align_of::<jb_ingest__set_cgroup>();
        let padded_raw_size = (SET_CGROUP_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_cgroup, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__set_cgroup, cgroup), 8usize);
        assert_eq!(offset_of!(jb_ingest__set_cgroup, _ref), 16usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_command {
    pub _rpc_id: u16,
    pub _len: u16,
    pub _ref: u64,
}

impl jb_ingest__set_command {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(504u16, true)
    }
}

impl Default for jb_ingest__set_command {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_COMMAND_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod set_command_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_command>();
        let align = align_of::<jb_ingest__set_command>();
        let padded_raw_size = (SET_COMMAND_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_command, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__set_command, _len), 2);
        assert_eq!(offset_of!(jb_ingest__set_command, _ref), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pid_exit {
    pub _rpc_id: u16,
    pub pid: u32,
    pub tgid: u64,
    pub _ref: u64,
    pub exit_code: i32,
}

impl jb_ingest__pid_exit {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(517u16, 28, true)
    }
}

impl Default for jb_ingest__pid_exit {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PID_EXIT_WIRE_SIZE: usize = 28;

#[cfg(test)]
mod pid_exit_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pid_exit>();
        let align = align_of::<jb_ingest__pid_exit>();
        let padded_raw_size = (PID_EXIT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pid_exit, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pid_exit, pid), 4usize);
        assert_eq!(offset_of!(jb_ingest__pid_exit, tgid), 8usize);
        assert_eq!(offset_of!(jb_ingest__pid_exit, _ref), 16usize);
        assert_eq!(offset_of!(jb_ingest__pid_exit, exit_code), 24usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__cgroup_create_deprecated {
    pub _rpc_id: u16,
    pub name: [u8; 64],
    pub cgroup: u64,
    pub cgroup_parent: u64,
}

impl jb_ingest__cgroup_create_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(394u16, 88, true)
    }
}

impl Default for jb_ingest__cgroup_create_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CGROUP_CREATE_DEPRECATED_WIRE_SIZE: usize = 88;

#[cfg(test)]
mod cgroup_create_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__cgroup_create_deprecated>();
        let align = align_of::<jb_ingest__cgroup_create_deprecated>();
        let padded_raw_size = (CGROUP_CREATE_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__cgroup_create_deprecated, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__cgroup_create_deprecated, name),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__cgroup_create_deprecated, cgroup),
            72usize
        );
        assert_eq!(
            offset_of!(jb_ingest__cgroup_create_deprecated, cgroup_parent),
            80usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__cgroup_create {
    pub _rpc_id: u16,
    pub name: [u8; 256],
    pub cgroup: u64,
    pub cgroup_parent: u64,
}

impl jb_ingest__cgroup_create {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(544u16, 280, true)
    }
}

impl Default for jb_ingest__cgroup_create {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CGROUP_CREATE_WIRE_SIZE: usize = 280;

#[cfg(test)]
mod cgroup_create_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__cgroup_create>();
        let align = align_of::<jb_ingest__cgroup_create>();
        let padded_raw_size = (CGROUP_CREATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__cgroup_create, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__cgroup_create, name), 2usize);
        assert_eq!(offset_of!(jb_ingest__cgroup_create, cgroup), 264usize);
        assert_eq!(
            offset_of!(jb_ingest__cgroup_create, cgroup_parent),
            272usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__cgroup_close {
    pub _rpc_id: u16,
    pub cgroup: u64,
}

impl jb_ingest__cgroup_close {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(395u16, 16, true)
    }
}

impl Default for jb_ingest__cgroup_close {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CGROUP_CLOSE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod cgroup_close_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__cgroup_close>();
        let align = align_of::<jb_ingest__cgroup_close>();
        let padded_raw_size = (CGROUP_CLOSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__cgroup_close, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__cgroup_close, cgroup), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__container_metadata {
    pub _rpc_id: u16,
    pub _len: u16,
    pub id: u16,
    pub name: u16,
    pub cgroup: u64,
    pub image: u16,
    pub ip_addr: u16,
    pub cluster: u16,
    pub container_name: u16,
    pub task_family: u16,
    pub task_version: u16,
}

impl jb_ingest__container_metadata {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(396u16, true)
    }
}

impl Default for jb_ingest__container_metadata {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONTAINER_METADATA_WIRE_SIZE: usize = 28;

#[cfg(test)]
mod container_metadata_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__container_metadata>();
        let align = align_of::<jb_ingest__container_metadata>();
        let padded_raw_size = (CONTAINER_METADATA_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__container_metadata, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__container_metadata, _len), 2);
        assert_eq!(offset_of!(jb_ingest__container_metadata, id), 4usize);
        assert_eq!(offset_of!(jb_ingest__container_metadata, name), 6usize);
        assert_eq!(offset_of!(jb_ingest__container_metadata, cgroup), 8usize);
        assert_eq!(offset_of!(jb_ingest__container_metadata, image), 16usize);
        assert_eq!(offset_of!(jb_ingest__container_metadata, ip_addr), 18usize);
        assert_eq!(offset_of!(jb_ingest__container_metadata, cluster), 20usize);
        assert_eq!(
            offset_of!(jb_ingest__container_metadata, container_name),
            22usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_metadata, task_family),
            24usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_metadata, task_version),
            26usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_name {
    pub _rpc_id: u16,
    pub _len: u16,
    pub _deprecated_pod_uid: u16,
    pub cgroup: u64,
}

impl jb_ingest__pod_name {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(410u16, true)
    }
}

impl Default for jb_ingest__pod_name {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_NAME_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod pod_name_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_name>();
        let align = align_of::<jb_ingest__pod_name>();
        let padded_raw_size = (POD_NAME_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_name, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_name, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pod_name, _deprecated_pod_uid), 4usize);
        assert_eq!(offset_of!(jb_ingest__pod_name, cgroup), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__nomad_metadata {
    pub _rpc_id: u16,
    pub _len: u16,
    pub ns: u16,
    pub group_name: u16,
    pub cgroup: u64,
    pub task_name: u16,
}

impl jb_ingest__nomad_metadata {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(508u16, true)
    }
}

impl Default for jb_ingest__nomad_metadata {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NOMAD_METADATA_WIRE_SIZE: usize = 18;

#[cfg(test)]
mod nomad_metadata_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__nomad_metadata>();
        let align = align_of::<jb_ingest__nomad_metadata>();
        let padded_raw_size = (NOMAD_METADATA_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__nomad_metadata, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__nomad_metadata, _len), 2);
        assert_eq!(offset_of!(jb_ingest__nomad_metadata, ns), 4usize);
        assert_eq!(offset_of!(jb_ingest__nomad_metadata, group_name), 6usize);
        assert_eq!(offset_of!(jb_ingest__nomad_metadata, cgroup), 8usize);
        assert_eq!(offset_of!(jb_ingest__nomad_metadata, task_name), 16usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__k8s_metadata {
    pub _rpc_id: u16,
    pub _len: u16,
    pub container_name: u16,
    pub pod_name: u16,
    pub cgroup: u64,
    pub pod_ns: u16,
    pub pod_uid: u16,
}

impl jb_ingest__k8s_metadata {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(512u16, true)
    }
}

impl Default for jb_ingest__k8s_metadata {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_METADATA_WIRE_SIZE: usize = 20;

#[cfg(test)]
mod k8s_metadata_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__k8s_metadata>();
        let align = align_of::<jb_ingest__k8s_metadata>();
        let padded_raw_size = (K8S_METADATA_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__k8s_metadata, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata, _len), 2);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata, container_name), 4usize);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata, pod_name), 6usize);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata, cgroup), 8usize);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata, pod_ns), 16usize);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata, pod_uid), 18usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__k8s_metadata_port {
    pub _rpc_id: u16,
    pub _len: u16,
    pub port: u16,
    pub protocol: u8,
    pub cgroup: u64,
}

impl jb_ingest__k8s_metadata_port {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(513u16, true)
    }
}

impl Default for jb_ingest__k8s_metadata_port {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const K8S_METADATA_PORT_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod k8s_metadata_port_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__k8s_metadata_port>();
        let align = align_of::<jb_ingest__k8s_metadata_port>();
        let padded_raw_size = (K8S_METADATA_PORT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__k8s_metadata_port, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata_port, _len), 2);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata_port, port), 4usize);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata_port, protocol), 6usize);
        assert_eq!(offset_of!(jb_ingest__k8s_metadata_port, cgroup), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__container_resource_limits_deprecated {
    pub _rpc_id: u16,
    pub cpu_shares: u16,
    pub cpu_period: u16,
    pub cpu_quota: u16,
    pub cgroup: u64,
    pub memory_limit: u64,
    pub memory_soft_limit: u64,
    pub total_memory_limit: i64,
    pub memory_swappiness: u8,
}

impl jb_ingest__container_resource_limits_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(514u16, 41, true)
    }
}

impl Default for jb_ingest__container_resource_limits_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONTAINER_RESOURCE_LIMITS_DEPRECATED_WIRE_SIZE: usize = 41;

#[cfg(test)]
mod container_resource_limits_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__container_resource_limits_deprecated>();
        let align = align_of::<jb_ingest__container_resource_limits_deprecated>();
        let padded_raw_size =
            (CONTAINER_RESOURCE_LIMITS_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits_deprecated, cpu_shares),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits_deprecated, cpu_period),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits_deprecated, cpu_quota),
            6usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits_deprecated, cgroup),
            8usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__container_resource_limits_deprecated,
                memory_limit
            ),
            16usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__container_resource_limits_deprecated,
                memory_soft_limit
            ),
            24usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__container_resource_limits_deprecated,
                total_memory_limit
            ),
            32usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__container_resource_limits_deprecated,
                memory_swappiness
            ),
            40usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__container_resource_limits {
    pub _rpc_id: u16,
    pub cpu_shares: u16,
    pub cpu_period: u32,
    pub cgroup: u64,
    pub memory_limit: u64,
    pub memory_soft_limit: u64,
    pub total_memory_limit: i64,
    pub cpu_quota: u32,
    pub memory_swappiness: u8,
}

impl jb_ingest__container_resource_limits {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(518u16, 45, true)
    }
}

impl Default for jb_ingest__container_resource_limits {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONTAINER_RESOURCE_LIMITS_WIRE_SIZE: usize = 45;

#[cfg(test)]
mod container_resource_limits_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__container_resource_limits>();
        let align = align_of::<jb_ingest__container_resource_limits>();
        let padded_raw_size = (CONTAINER_RESOURCE_LIMITS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__container_resource_limits, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, cpu_shares),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, cpu_period),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, cgroup),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, memory_limit),
            16usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, memory_soft_limit),
            24usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, total_memory_limit),
            32usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, cpu_quota),
            40usize
        );
        assert_eq!(
            offset_of!(jb_ingest__container_resource_limits, memory_swappiness),
            44usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__container_annotation {
    pub _rpc_id: u16,
    pub _len: u16,
    pub key: u16,
    pub cgroup: u64,
}

impl jb_ingest__container_annotation {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(538u16, true)
    }
}

impl Default for jb_ingest__container_annotation {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONTAINER_ANNOTATION_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod container_annotation_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__container_annotation>();
        let align = align_of::<jb_ingest__container_annotation>();
        let padded_raw_size = (CONTAINER_ANNOTATION_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__container_annotation, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__container_annotation, _len), 2);
        assert_eq!(offset_of!(jb_ingest__container_annotation, key), 4usize);
        assert_eq!(offset_of!(jb_ingest__container_annotation, cgroup), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__new_sock_info {
    pub _rpc_id: u16,
    pub pid: u32,
    pub sk: u64,
}

impl jb_ingest__new_sock_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(302u16, 16, true)
    }
}

impl Default for jb_ingest__new_sock_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NEW_SOCK_INFO_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod new_sock_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__new_sock_info>();
        let align = align_of::<jb_ingest__new_sock_info>();
        let padded_raw_size = (NEW_SOCK_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__new_sock_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__new_sock_info, pid), 4usize);
        assert_eq!(offset_of!(jb_ingest__new_sock_info, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_state_ipv4 {
    pub _rpc_id: u16,
    pub dport: u16,
    pub dest: u32,
    pub sk: u64,
    pub src: u32,
    pub tx_rx: u32,
    pub sport: u16,
}

impl jb_ingest__set_state_ipv4 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(303u16, 26, true)
    }
}

impl Default for jb_ingest__set_state_ipv4 {
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
        let size = size_of::<jb_ingest__set_state_ipv4>();
        let align = align_of::<jb_ingest__set_state_ipv4>();
        let padded_raw_size = (SET_STATE_IPV4_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_state_ipv4, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv4, dport), 2usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv4, dest), 4usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv4, sk), 8usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv4, src), 16usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv4, tx_rx), 20usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv4, sport), 24usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_state_ipv6 {
    pub _rpc_id: u16,
    pub dport: u16,
    pub tx_rx: u32,
    pub sk: u64,
    pub sport: u16,
    pub dest: [u8; 16],
    pub src: [u8; 16],
}

impl jb_ingest__set_state_ipv6 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(304u16, 50, true)
    }
}

impl Default for jb_ingest__set_state_ipv6 {
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
        let size = size_of::<jb_ingest__set_state_ipv6>();
        let align = align_of::<jb_ingest__set_state_ipv6>();
        let padded_raw_size = (SET_STATE_IPV6_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_state_ipv6, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv6, dport), 2usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv6, tx_rx), 4usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv6, sk), 8usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv6, sport), 16usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv6, dest), 18usize);
        assert_eq!(offset_of!(jb_ingest__set_state_ipv6, src), 34usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__socket_stats {
    pub _rpc_id: u16,
    pub is_rx: u8,
    pub diff_delivered: u32,
    pub sk: u64,
    pub diff_bytes: u64,
    pub diff_retrans: u32,
    pub max_srtt: u32,
}

impl jb_ingest__socket_stats {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(326u16, 32, true)
    }
}

impl Default for jb_ingest__socket_stats {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SOCKET_STATS_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod socket_stats_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__socket_stats>();
        let align = align_of::<jb_ingest__socket_stats>();
        let padded_raw_size = (SOCKET_STATS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__socket_stats, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__socket_stats, is_rx), 2usize);
        assert_eq!(offset_of!(jb_ingest__socket_stats, diff_delivered), 4usize);
        assert_eq!(offset_of!(jb_ingest__socket_stats, sk), 8usize);
        assert_eq!(offset_of!(jb_ingest__socket_stats, diff_bytes), 16usize);
        assert_eq!(offset_of!(jb_ingest__socket_stats, diff_retrans), 24usize);
        assert_eq!(offset_of!(jb_ingest__socket_stats, max_srtt), 28usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__nat_remapping {
    pub _rpc_id: u16,
    pub sport: u16,
    pub src: u32,
    pub sk: u64,
    pub dst: u32,
    pub dport: u16,
}

impl jb_ingest__nat_remapping {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(360u16, 22, true)
    }
}

impl Default for jb_ingest__nat_remapping {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NAT_REMAPPING_WIRE_SIZE: usize = 22;

#[cfg(test)]
mod nat_remapping_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__nat_remapping>();
        let align = align_of::<jb_ingest__nat_remapping>();
        let padded_raw_size = (NAT_REMAPPING_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__nat_remapping, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__nat_remapping, sport), 2usize);
        assert_eq!(offset_of!(jb_ingest__nat_remapping, src), 4usize);
        assert_eq!(offset_of!(jb_ingest__nat_remapping, sk), 8usize);
        assert_eq!(offset_of!(jb_ingest__nat_remapping, dst), 16usize);
        assert_eq!(offset_of!(jb_ingest__nat_remapping, dport), 20usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__close_sock_info {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl jb_ingest__close_sock_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(308u16, 16, true)
    }
}

impl Default for jb_ingest__close_sock_info {
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
        let size = size_of::<jb_ingest__close_sock_info>();
        let align = align_of::<jb_ingest__close_sock_info>();
        let padded_raw_size = (CLOSE_SOCK_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__close_sock_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__close_sock_info, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__syn_timeout {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl jb_ingest__syn_timeout {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(398u16, 16, true)
    }
}

impl Default for jb_ingest__syn_timeout {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SYN_TIMEOUT_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod syn_timeout_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__syn_timeout>();
        let align = align_of::<jb_ingest__syn_timeout>();
        let padded_raw_size = (SYN_TIMEOUT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__syn_timeout, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__syn_timeout, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__http_response {
    pub _rpc_id: u16,
    pub code: u16,
    pub pid: u32,
    pub sk: u64,
    pub latency_ns: u64,
    pub client_server: u8,
}

impl jb_ingest__http_response {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(401u16, 25, true)
    }
}

impl Default for jb_ingest__http_response {
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
        let size = size_of::<jb_ingest__http_response>();
        let align = align_of::<jb_ingest__http_response>();
        let padded_raw_size = (HTTP_RESPONSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__http_response, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__http_response, code), 2usize);
        assert_eq!(offset_of!(jb_ingest__http_response, pid), 4usize);
        assert_eq!(offset_of!(jb_ingest__http_response, sk), 8usize);
        assert_eq!(offset_of!(jb_ingest__http_response, latency_ns), 16usize);
        assert_eq!(offset_of!(jb_ingest__http_response, client_server), 24usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__tcp_reset {
    pub _rpc_id: u16,
    pub is_rx: u8,
    pub sk: u64,
}

impl jb_ingest__tcp_reset {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(519u16, 16, true)
    }
}

impl Default for jb_ingest__tcp_reset {
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
        let size = size_of::<jb_ingest__tcp_reset>();
        let align = align_of::<jb_ingest__tcp_reset>();
        let padded_raw_size = (TCP_RESET_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__tcp_reset, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__tcp_reset, is_rx), 2usize);
        assert_eq!(offset_of!(jb_ingest__tcp_reset, sk), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__process_steady_state {
    pub _rpc_id: u16,
    pub time: u64,
}

impl jb_ingest__process_steady_state {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(307u16, 16, true)
    }
}

impl Default for jb_ingest__process_steady_state {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PROCESS_STEADY_STATE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod process_steady_state_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__process_steady_state>();
        let align = align_of::<jb_ingest__process_steady_state>();
        let padded_raw_size = (PROCESS_STEADY_STATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__process_steady_state, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__process_steady_state, time), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__socket_steady_state {
    pub _rpc_id: u16,
    pub time: u64,
}

impl jb_ingest__socket_steady_state {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(309u16, 16, true)
    }
}

impl Default for jb_ingest__socket_steady_state {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SOCKET_STEADY_STATE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod socket_steady_state_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__socket_steady_state>();
        let align = align_of::<jb_ingest__socket_steady_state>();
        let padded_raw_size = (SOCKET_STEADY_STATE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__socket_steady_state, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__socket_steady_state, time), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__version_info {
    pub _rpc_id: u16,
    pub major: u32,
    pub minor: u32,
    pub patch: u32,
}

impl jb_ingest__version_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(310u16, 16, false)
    }
}

impl Default for jb_ingest__version_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const VERSION_INFO_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod version_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__version_info>();
        let align = align_of::<jb_ingest__version_info>();
        let padded_raw_size = (VERSION_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__version_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__version_info, major), 4usize);
        assert_eq!(offset_of!(jb_ingest__version_info, minor), 8usize);
        assert_eq!(offset_of!(jb_ingest__version_info, patch), 12usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_node_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub az: u16,
    pub role: u16,
    pub instance_id: u16,
}

impl jb_ingest__set_node_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(415u16, true)
    }
}

impl Default for jb_ingest__set_node_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_NODE_INFO_WIRE_SIZE: usize = 10;

#[cfg(test)]
mod set_node_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_node_info>();
        let align = align_of::<jb_ingest__set_node_info>();
        let padded_raw_size = (SET_NODE_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_node_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__set_node_info, _len), 2);
        assert_eq!(offset_of!(jb_ingest__set_node_info, az), 4usize);
        assert_eq!(offset_of!(jb_ingest__set_node_info, role), 6usize);
        assert_eq!(offset_of!(jb_ingest__set_node_info, instance_id), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_config_label {
    pub _rpc_id: u16,
    pub _len: u16,
    pub key: u16,
}

impl jb_ingest__set_config_label {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(416u16, true)
    }
}

impl Default for jb_ingest__set_config_label {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_CONFIG_LABEL_WIRE_SIZE: usize = 6;

#[cfg(test)]
mod set_config_label_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_config_label>();
        let align = align_of::<jb_ingest__set_config_label>();
        let padded_raw_size = (SET_CONFIG_LABEL_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_config_label, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__set_config_label, _len), 2);
        assert_eq!(offset_of!(jb_ingest__set_config_label, key), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_availability_zone_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub az: [u8; 16],
}

impl jb_ingest__set_availability_zone_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(321u16, 19, true)
    }
}

impl Default for jb_ingest__set_availability_zone_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_AVAILABILITY_ZONE_DEPRECATED_WIRE_SIZE: usize = 19;

#[cfg(test)]
mod set_availability_zone_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_availability_zone_deprecated>();
        let align = align_of::<jb_ingest__set_availability_zone_deprecated>();
        let padded_raw_size =
            (SET_AVAILABILITY_ZONE_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__set_availability_zone_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__set_availability_zone_deprecated, retcode),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__set_availability_zone_deprecated, az),
            3usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_iam_role_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub role: [u8; 64],
}

impl jb_ingest__set_iam_role_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(322u16, 67, true)
    }
}

impl Default for jb_ingest__set_iam_role_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_IAM_ROLE_DEPRECATED_WIRE_SIZE: usize = 67;

#[cfg(test)]
mod set_iam_role_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_iam_role_deprecated>();
        let align = align_of::<jb_ingest__set_iam_role_deprecated>();
        let padded_raw_size = (SET_IAM_ROLE_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__set_iam_role_deprecated, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__set_iam_role_deprecated, retcode),
            2usize
        );
        assert_eq!(offset_of!(jb_ingest__set_iam_role_deprecated, role), 3usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_instance_id_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub id: [u8; 17],
}

impl jb_ingest__set_instance_id_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(323u16, 20, true)
    }
}

impl Default for jb_ingest__set_instance_id_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_INSTANCE_ID_DEPRECATED_WIRE_SIZE: usize = 20;

#[cfg(test)]
mod set_instance_id_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_instance_id_deprecated>();
        let align = align_of::<jb_ingest__set_instance_id_deprecated>();
        let padded_raw_size = (SET_INSTANCE_ID_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__set_instance_id_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__set_instance_id_deprecated, retcode),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__set_instance_id_deprecated, id),
            3usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_instance_type_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub val: [u8; 17],
}

impl jb_ingest__set_instance_type_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(324u16, 20, true)
    }
}

impl Default for jb_ingest__set_instance_type_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_INSTANCE_TYPE_DEPRECATED_WIRE_SIZE: usize = 20;

#[cfg(test)]
mod set_instance_type_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_instance_type_deprecated>();
        let align = align_of::<jb_ingest__set_instance_type_deprecated>();
        let padded_raw_size = (SET_INSTANCE_TYPE_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__set_instance_type_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__set_instance_type_deprecated, retcode),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__set_instance_type_deprecated, val),
            3usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__dns_response_fake {
    pub _rpc_id: u16,
    pub _len: u16,
    pub total_dn_len: u16,
    pub ips: u16,
}

impl jb_ingest__dns_response_fake {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(325u16, true)
    }
}

impl Default for jb_ingest__dns_response_fake {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const DNS_RESPONSE_FAKE_WIRE_SIZE: usize = 8;

#[cfg(test)]
mod dns_response_fake_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__dns_response_fake>();
        let align = align_of::<jb_ingest__dns_response_fake>();
        let padded_raw_size = (DNS_RESPONSE_FAKE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__dns_response_fake, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__dns_response_fake, _len), 2);
        assert_eq!(
            offset_of!(jb_ingest__dns_response_fake, total_dn_len),
            4usize
        );
        assert_eq!(offset_of!(jb_ingest__dns_response_fake, ips), 6usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__dns_response_dep_a_deprecated {
    pub _rpc_id: u16,
    pub _len: u16,
    pub total_dn_len: u16,
    pub domain_name: u16,
    pub ipv4_addrs: u16,
}

impl jb_ingest__dns_response_dep_a_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(391u16, true)
    }
}

impl Default for jb_ingest__dns_response_dep_a_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const DNS_RESPONSE_DEP_A_DEPRECATED_WIRE_SIZE: usize = 10;

#[cfg(test)]
mod dns_response_dep_a_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__dns_response_dep_a_deprecated>();
        let align = align_of::<jb_ingest__dns_response_dep_a_deprecated>();
        let padded_raw_size = (DNS_RESPONSE_DEP_A_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_a_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_a_deprecated, _len),
            2
        );
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_a_deprecated, total_dn_len),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_a_deprecated, domain_name),
            6usize
        );
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_a_deprecated, ipv4_addrs),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__set_config_label_deprecated {
    pub _rpc_id: u16,
    pub key: [u8; 20],
    pub val: [u8; 40],
}

impl jb_ingest__set_config_label_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(327u16, 62, true)
    }
}

impl Default for jb_ingest__set_config_label_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SET_CONFIG_LABEL_DEPRECATED_WIRE_SIZE: usize = 62;

#[cfg(test)]
mod set_config_label_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__set_config_label_deprecated>();
        let align = align_of::<jb_ingest__set_config_label_deprecated>();
        let padded_raw_size = (SET_CONFIG_LABEL_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__set_config_label_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__set_config_label_deprecated, key),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__set_config_label_deprecated, val),
            22usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__api_key {
    pub _rpc_id: u16,
    pub tenant: [u8; 20],
    pub api_key: [u8; 64],
}

impl jb_ingest__api_key {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(352u16, 86, false)
    }
}

impl Default for jb_ingest__api_key {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const API_KEY_WIRE_SIZE: usize = 86;

#[cfg(test)]
mod api_key_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__api_key>();
        let align = align_of::<jb_ingest__api_key>();
        let padded_raw_size = (API_KEY_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__api_key, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__api_key, tenant), 2usize);
        assert_eq!(offset_of!(jb_ingest__api_key, api_key), 22usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__private_ipv4_addr {
    pub _rpc_id: u16,
    pub vpc_id: [u8; 22],
    pub addr: u32,
}

impl jb_ingest__private_ipv4_addr {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(353u16, 28, true)
    }
}

impl Default for jb_ingest__private_ipv4_addr {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PRIVATE_IPV4_ADDR_WIRE_SIZE: usize = 28;

#[cfg(test)]
mod private_ipv4_addr_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__private_ipv4_addr>();
        let align = align_of::<jb_ingest__private_ipv4_addr>();
        let padded_raw_size = (PRIVATE_IPV4_ADDR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__private_ipv4_addr, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__private_ipv4_addr, vpc_id), 2usize);
        assert_eq!(offset_of!(jb_ingest__private_ipv4_addr, addr), 24usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__ipv6_addr {
    pub _rpc_id: u16,
    pub addr: [u8; 16],
    pub vpc_id: [u8; 22],
}

impl jb_ingest__ipv6_addr {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(354u16, 40, true)
    }
}

impl Default for jb_ingest__ipv6_addr {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const IPV6_ADDR_WIRE_SIZE: usize = 40;

#[cfg(test)]
mod ipv6_addr_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__ipv6_addr>();
        let align = align_of::<jb_ingest__ipv6_addr>();
        let padded_raw_size = (IPV6_ADDR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__ipv6_addr, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__ipv6_addr, addr), 2usize);
        assert_eq!(offset_of!(jb_ingest__ipv6_addr, vpc_id), 18usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__public_to_private_ipv4 {
    pub _rpc_id: u16,
    pub vpc_id: [u8; 22],
    pub public_addr: u32,
    pub private_addr: u32,
}

impl jb_ingest__public_to_private_ipv4 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(355u16, 32, true)
    }
}

impl Default for jb_ingest__public_to_private_ipv4 {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const PUBLIC_TO_PRIVATE_IPV4_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod public_to_private_ipv4_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__public_to_private_ipv4>();
        let align = align_of::<jb_ingest__public_to_private_ipv4>();
        let padded_raw_size = (PUBLIC_TO_PRIVATE_IPV4_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__public_to_private_ipv4, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__public_to_private_ipv4, vpc_id),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__public_to_private_ipv4, public_addr),
            24usize
        );
        assert_eq!(
            offset_of!(jb_ingest__public_to_private_ipv4, private_addr),
            28usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__metadata_complete {
    pub _rpc_id: u16,
    pub time: u64,
}

impl jb_ingest__metadata_complete {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(356u16, 16, true)
    }
}

impl Default for jb_ingest__metadata_complete {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const METADATA_COMPLETE_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod metadata_complete_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__metadata_complete>();
        let align = align_of::<jb_ingest__metadata_complete>();
        let padded_raw_size = (METADATA_COMPLETE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__metadata_complete, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__metadata_complete, time), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__bpf_lost_samples {
    pub _rpc_id: u16,
    pub count: u64,
}

impl jb_ingest__bpf_lost_samples {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(357u16, 16, true)
    }
}

impl Default for jb_ingest__bpf_lost_samples {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const BPF_LOST_SAMPLES_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod bpf_lost_samples_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__bpf_lost_samples>();
        let align = align_of::<jb_ingest__bpf_lost_samples>();
        let padded_raw_size = (BPF_LOST_SAMPLES_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__bpf_lost_samples, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__bpf_lost_samples, count), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_new_legacy {
    pub _rpc_id: u16,
    pub _len: u16,
    pub ip: u32,
    pub uid: u16,
    pub owner_name: u16,
    pub owner_uid: u16,
    pub owner_kind: u8,
    pub is_host_network: u8,
}

impl jb_ingest__pod_new_legacy {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(358u16, true)
    }
}

impl Default for jb_ingest__pod_new_legacy {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_NEW_LEGACY_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod pod_new_legacy_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_new_legacy>();
        let align = align_of::<jb_ingest__pod_new_legacy>();
        let padded_raw_size = (POD_NEW_LEGACY_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy, ip), 4usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy, uid), 8usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy, owner_name), 10usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy, owner_uid), 12usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy, owner_kind), 14usize);
        assert_eq!(
            offset_of!(jb_ingest__pod_new_legacy, is_host_network),
            15usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_new_legacy2 {
    pub _rpc_id: u16,
    pub _len: u16,
    pub ip: u32,
    pub uid: u16,
    pub owner_name: u16,
    pub owner_uid: u16,
    pub ns: u16,
    pub owner_kind: u8,
    pub is_host_network: u8,
}

impl jb_ingest__pod_new_legacy2 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(414u16, true)
    }
}

impl Default for jb_ingest__pod_new_legacy2 {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_NEW_LEGACY2_WIRE_SIZE: usize = 18;

#[cfg(test)]
mod pod_new_legacy2_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_new_legacy2>();
        let align = align_of::<jb_ingest__pod_new_legacy2>();
        let padded_raw_size = (POD_NEW_LEGACY2_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, ip), 4usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, uid), 8usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, owner_name), 10usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, owner_uid), 12usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, ns), 14usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_legacy2, owner_kind), 16usize);
        assert_eq!(
            offset_of!(jb_ingest__pod_new_legacy2, is_host_network),
            17usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_new_with_name {
    pub _rpc_id: u16,
    pub _len: u16,
    pub ip: u32,
    pub uid: u16,
    pub owner_name: u16,
    pub pod_name: u16,
    pub owner_uid: u16,
    pub ns: u16,
    pub owner_kind: u8,
    pub is_host_network: u8,
}

impl jb_ingest__pod_new_with_name {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(515u16, true)
    }
}

impl Default for jb_ingest__pod_new_with_name {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_NEW_WITH_NAME_WIRE_SIZE: usize = 20;

#[cfg(test)]
mod pod_new_with_name_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_new_with_name>();
        let align = align_of::<jb_ingest__pod_new_with_name>();
        let padded_raw_size = (POD_NEW_WITH_NAME_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_new_with_name, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_new_with_name, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pod_new_with_name, ip), 4usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_with_name, uid), 8usize);
        assert_eq!(
            offset_of!(jb_ingest__pod_new_with_name, owner_name),
            10usize
        );
        assert_eq!(offset_of!(jb_ingest__pod_new_with_name, pod_name), 12usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_with_name, owner_uid), 14usize);
        assert_eq!(offset_of!(jb_ingest__pod_new_with_name, ns), 16usize);
        assert_eq!(
            offset_of!(jb_ingest__pod_new_with_name, owner_kind),
            18usize
        );
        assert_eq!(
            offset_of!(jb_ingest__pod_new_with_name, is_host_network),
            19usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_container_legacy {
    pub _rpc_id: u16,
    pub _len: u16,
    pub uid: u16,
}

impl jb_ingest__pod_container_legacy {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(400u16, true)
    }
}

impl Default for jb_ingest__pod_container_legacy {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_CONTAINER_LEGACY_WIRE_SIZE: usize = 6;

#[cfg(test)]
mod pod_container_legacy_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_container_legacy>();
        let align = align_of::<jb_ingest__pod_container_legacy>();
        let padded_raw_size = (POD_CONTAINER_LEGACY_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_container_legacy, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_container_legacy, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pod_container_legacy, uid), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_container {
    pub _rpc_id: u16,
    pub _len: u16,
    pub uid: u16,
    pub container_id: u16,
    pub container_name: u16,
}

impl jb_ingest__pod_container {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(494u16, true)
    }
}

impl Default for jb_ingest__pod_container {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_CONTAINER_WIRE_SIZE: usize = 10;

#[cfg(test)]
mod pod_container_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_container>();
        let align = align_of::<jb_ingest__pod_container>();
        let padded_raw_size = (POD_CONTAINER_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_container, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_container, _len), 2);
        assert_eq!(offset_of!(jb_ingest__pod_container, uid), 4usize);
        assert_eq!(offset_of!(jb_ingest__pod_container, container_id), 6usize);
        assert_eq!(offset_of!(jb_ingest__pod_container, container_name), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_delete {
    pub _rpc_id: u16,
    pub _len: u16,
}

impl jb_ingest__pod_delete {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(359u16, true)
    }
}

impl Default for jb_ingest__pod_delete {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_DELETE_WIRE_SIZE: usize = 4;

#[cfg(test)]
mod pod_delete_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_delete>();
        let align = align_of::<jb_ingest__pod_delete>();
        let padded_raw_size = (POD_DELETE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_delete, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_delete, _len), 2);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pod_resync {
    pub _rpc_id: u16,
    pub resync_count: u64,
}

impl jb_ingest__pod_resync {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(390u16, 16, true)
    }
}

impl Default for jb_ingest__pod_resync {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const POD_RESYNC_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod pod_resync_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__pod_resync>();
        let align = align_of::<jb_ingest__pod_resync>();
        let padded_raw_size = (POD_RESYNC_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pod_resync, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__pod_resync, resync_count), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__span_duration_info {
    pub _rpc_id: u16,
    pub duration: u64,
}

impl jb_ingest__span_duration_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(351u16, 16, true)
    }
}

impl Default for jb_ingest__span_duration_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SPAN_DURATION_INFO_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod span_duration_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__span_duration_info>();
        let align = align_of::<jb_ingest__span_duration_info>();
        let padded_raw_size = (SPAN_DURATION_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__span_duration_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__span_duration_info, duration), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__heartbeat {
    pub _rpc_id: u16,
}

impl jb_ingest__heartbeat {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(392u16, 2, true)
    }
}

impl Default for jb_ingest__heartbeat {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const HEARTBEAT_WIRE_SIZE: usize = 2;

#[cfg(test)]
mod heartbeat_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__heartbeat>();
        let align = align_of::<jb_ingest__heartbeat>();
        let padded_raw_size = (HEARTBEAT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__heartbeat, _rpc_id), 0);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__connect {
    pub _rpc_id: u16,
    pub _len: u16,
    pub collector_type: u8,
}

impl jb_ingest__connect {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(548u16, false)
    }
}

impl Default for jb_ingest__connect {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CONNECT_WIRE_SIZE: usize = 5;

#[cfg(test)]
mod connect_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__connect>();
        let align = align_of::<jb_ingest__connect>();
        let padded_raw_size = (CONNECT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__connect, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__connect, _len), 2);
        assert_eq!(offset_of!(jb_ingest__connect, collector_type), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__health_check {
    pub _rpc_id: u16,
    pub _len: u16,
    pub client_type: u8,
}

impl jb_ingest__health_check {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(409u16, false)
    }
}

impl Default for jb_ingest__health_check {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const HEALTH_CHECK_WIRE_SIZE: usize = 5;

#[cfg(test)]
mod health_check_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__health_check>();
        let align = align_of::<jb_ingest__health_check>();
        let padded_raw_size = (HEALTH_CHECK_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__health_check, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__health_check, _len), 2);
        assert_eq!(offset_of!(jb_ingest__health_check, client_type), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__log_message {
    pub _rpc_id: u16,
    pub _len: u16,
    pub log_level: u8,
}

impl jb_ingest__log_message {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(411u16, true)
    }
}

impl Default for jb_ingest__log_message {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const LOG_MESSAGE_WIRE_SIZE: usize = 5;

#[cfg(test)]
mod log_message_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__log_message>();
        let align = align_of::<jb_ingest__log_message>();
        let padded_raw_size = (LOG_MESSAGE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__log_message, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__log_message, _len), 2);
        assert_eq!(offset_of!(jb_ingest__log_message, log_level), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__agent_resource_usage {
    pub _rpc_id: u16,
    pub cpu_usage_by_agent: u16,
    pub minor_page_faults: u32,
    pub user_mode_time_us: u64,
    pub kernel_mode_time_us: u64,
    pub max_resident_set_size: u64,
    pub major_page_faults: u32,
    pub block_input_count: u32,
    pub block_output_count: u32,
    pub voluntary_context_switch_count: u32,
    pub involuntary_context_switch_count: u32,
    pub cpu_idle: u16,
}

impl jb_ingest__agent_resource_usage {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(412u16, 54, true)
    }
}

impl Default for jb_ingest__agent_resource_usage {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AGENT_RESOURCE_USAGE_WIRE_SIZE: usize = 54;

#[cfg(test)]
mod agent_resource_usage_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__agent_resource_usage>();
        let align = align_of::<jb_ingest__agent_resource_usage>();
        let padded_raw_size = (AGENT_RESOURCE_USAGE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__agent_resource_usage, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, cpu_usage_by_agent),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, minor_page_faults),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, user_mode_time_us),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, kernel_mode_time_us),
            16usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, max_resident_set_size),
            24usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, major_page_faults),
            32usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, block_input_count),
            36usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, block_output_count),
            40usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__agent_resource_usage,
                voluntary_context_switch_count
            ),
            44usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__agent_resource_usage,
                involuntary_context_switch_count
            ),
            48usize
        );
        assert_eq!(
            offset_of!(jb_ingest__agent_resource_usage, cpu_idle),
            52usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__cloud_platform {
    pub _rpc_id: u16,
    pub cloud_platform: u16,
}

impl jb_ingest__cloud_platform {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(413u16, 4, true)
    }
}

impl Default for jb_ingest__cloud_platform {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CLOUD_PLATFORM_WIRE_SIZE: usize = 4;

#[cfg(test)]
mod cloud_platform_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__cloud_platform>();
        let align = align_of::<jb_ingest__cloud_platform>();
        let padded_raw_size = (CLOUD_PLATFORM_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__cloud_platform, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__cloud_platform, cloud_platform),
            2usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__os_info_deprecated {
    pub _rpc_id: u16,
    pub _len: u16,
    pub os: u8,
    pub flavor: u8,
}

impl jb_ingest__os_info_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(419u16, true)
    }
}

impl Default for jb_ingest__os_info_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const OS_INFO_DEPRECATED_WIRE_SIZE: usize = 6;

#[cfg(test)]
mod os_info_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__os_info_deprecated>();
        let align = align_of::<jb_ingest__os_info_deprecated>();
        let padded_raw_size = (OS_INFO_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__os_info_deprecated, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__os_info_deprecated, _len), 2);
        assert_eq!(offset_of!(jb_ingest__os_info_deprecated, os), 4usize);
        assert_eq!(offset_of!(jb_ingest__os_info_deprecated, flavor), 5usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__os_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub os_version: u16,
    pub os: u8,
    pub flavor: u8,
}

impl jb_ingest__os_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(545u16, true)
    }
}

impl Default for jb_ingest__os_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const OS_INFO_WIRE_SIZE: usize = 8;

#[cfg(test)]
mod os_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__os_info>();
        let align = align_of::<jb_ingest__os_info>();
        let padded_raw_size = (OS_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__os_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__os_info, _len), 2);
        assert_eq!(offset_of!(jb_ingest__os_info, os_version), 4usize);
        assert_eq!(offset_of!(jb_ingest__os_info, os), 6usize);
        assert_eq!(offset_of!(jb_ingest__os_info, flavor), 7usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__kernel_headers_source {
    pub _rpc_id: u16,
    pub source: u8,
}

impl jb_ingest__kernel_headers_source {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(420u16, 3, true)
    }
}

impl Default for jb_ingest__kernel_headers_source {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const KERNEL_HEADERS_SOURCE_WIRE_SIZE: usize = 3;

#[cfg(test)]
mod kernel_headers_source_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__kernel_headers_source>();
        let align = align_of::<jb_ingest__kernel_headers_source>();
        let padded_raw_size = (KERNEL_HEADERS_SOURCE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__kernel_headers_source, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__kernel_headers_source, source), 2usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__entrypoint_error {
    pub _rpc_id: u16,
    pub error: u8,
}

impl jb_ingest__entrypoint_error {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(491u16, 3, true)
    }
}

impl Default for jb_ingest__entrypoint_error {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const ENTRYPOINT_ERROR_WIRE_SIZE: usize = 3;

#[cfg(test)]
mod entrypoint_error_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__entrypoint_error>();
        let align = align_of::<jb_ingest__entrypoint_error>();
        let padded_raw_size = (ENTRYPOINT_ERROR_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__entrypoint_error, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__entrypoint_error, error), 2usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__bpf_compiled {
    pub _rpc_id: u16,
}

impl jb_ingest__bpf_compiled {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(492u16, 2, true)
    }
}

impl Default for jb_ingest__bpf_compiled {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const BPF_COMPILED_WIRE_SIZE: usize = 2;

#[cfg(test)]
mod bpf_compiled_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__bpf_compiled>();
        let align = align_of::<jb_ingest__bpf_compiled>();
        let padded_raw_size = (BPF_COMPILED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__bpf_compiled, _rpc_id), 0);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__begin_telemetry {
    pub _rpc_id: u16,
}

impl jb_ingest__begin_telemetry {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(493u16, 2, true)
    }
}

impl Default for jb_ingest__begin_telemetry {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const BEGIN_TELEMETRY_WIRE_SIZE: usize = 2;

#[cfg(test)]
mod begin_telemetry_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__begin_telemetry>();
        let align = align_of::<jb_ingest__begin_telemetry>();
        let padded_raw_size = (BEGIN_TELEMETRY_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__begin_telemetry, _rpc_id), 0);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__cloud_platform_account_info {
    pub _rpc_id: u16,
    pub _len: u16,
}

impl jb_ingest__cloud_platform_account_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(495u16, true)
    }
}

impl Default for jb_ingest__cloud_platform_account_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const CLOUD_PLATFORM_ACCOUNT_INFO_WIRE_SIZE: usize = 4;

#[cfg(test)]
mod cloud_platform_account_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__cloud_platform_account_info>();
        let align = align_of::<jb_ingest__cloud_platform_account_info>();
        let padded_raw_size = (CLOUD_PLATFORM_ACCOUNT_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__cloud_platform_account_info, _rpc_id),
            0
        );
        assert_eq!(offset_of!(jb_ingest__cloud_platform_account_info, _len), 2);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__collector_health {
    pub _rpc_id: u16,
    pub status: u16,
    pub detail: u16,
}

impl jb_ingest__collector_health {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(496u16, 6, true)
    }
}

impl Default for jb_ingest__collector_health {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const COLLECTOR_HEALTH_WIRE_SIZE: usize = 6;

#[cfg(test)]
mod collector_health_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__collector_health>();
        let align = align_of::<jb_ingest__collector_health>();
        let padded_raw_size = (COLLECTOR_HEALTH_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__collector_health, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__collector_health, status), 2usize);
        assert_eq!(offset_of!(jb_ingest__collector_health, detail), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__system_wide_process_settings {
    pub _rpc_id: u16,
    pub clock_ticks_per_second: u64,
    pub memory_page_bytes: u64,
}

impl jb_ingest__system_wide_process_settings {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(498u16, 24, true)
    }
}

impl Default for jb_ingest__system_wide_process_settings {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const SYSTEM_WIDE_PROCESS_SETTINGS_WIRE_SIZE: usize = 24;

#[cfg(test)]
mod system_wide_process_settings_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__system_wide_process_settings>();
        let align = align_of::<jb_ingest__system_wide_process_settings>();
        let padded_raw_size = (SYSTEM_WIDE_PROCESS_SETTINGS_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__system_wide_process_settings, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(
                jb_ingest__system_wide_process_settings,
                clock_ticks_per_second
            ),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__system_wide_process_settings, memory_page_bytes),
            16usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__collect_blob {
    pub _rpc_id: u16,
    pub _len: u16,
    pub blob_type: u16,
    pub metadata: u16,
    pub subtype: u64,
}

impl jb_ingest__collect_blob {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(511u16, true)
    }
}

impl Default for jb_ingest__collect_blob {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const COLLECT_BLOB_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod collect_blob_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__collect_blob>();
        let align = align_of::<jb_ingest__collect_blob>();
        let padded_raw_size = (COLLECT_BLOB_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__collect_blob, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__collect_blob, _len), 2);
        assert_eq!(offset_of!(jb_ingest__collect_blob, blob_type), 4usize);
        assert_eq!(offset_of!(jb_ingest__collect_blob, metadata), 6usize);
        assert_eq!(offset_of!(jb_ingest__collect_blob, subtype), 8usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__report_cpu_cores {
    pub _rpc_id: u16,
    pub cpu_core_count: u32,
}

impl jb_ingest__report_cpu_cores {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(536u16, 8, true)
    }
}

impl Default for jb_ingest__report_cpu_cores {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const REPORT_CPU_CORES_WIRE_SIZE: usize = 8;

#[cfg(test)]
mod report_cpu_cores_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__report_cpu_cores>();
        let align = align_of::<jb_ingest__report_cpu_cores>();
        let padded_raw_size = (REPORT_CPU_CORES_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__report_cpu_cores, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__report_cpu_cores, cpu_core_count),
            4usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__bpf_log {
    pub _rpc_id: u16,
    pub _len: u16,
    pub line: u32,
    pub code: u64,
    pub arg0: u64,
    pub arg1: u64,
    pub arg2: u64,
}

impl jb_ingest__bpf_log {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(537u16, true)
    }
}

impl Default for jb_ingest__bpf_log {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const BPF_LOG_WIRE_SIZE: usize = 40;

#[cfg(test)]
mod bpf_log_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__bpf_log>();
        let align = align_of::<jb_ingest__bpf_log>();
        let padded_raw_size = (BPF_LOG_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__bpf_log, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__bpf_log, _len), 2);
        assert_eq!(offset_of!(jb_ingest__bpf_log, line), 4usize);
        assert_eq!(offset_of!(jb_ingest__bpf_log, code), 8usize);
        assert_eq!(offset_of!(jb_ingest__bpf_log, arg0), 16usize);
        assert_eq!(offset_of!(jb_ingest__bpf_log, arg1), 24usize);
        assert_eq!(offset_of!(jb_ingest__bpf_log, arg2), 32usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__aws_network_interface_start {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub ip: u128,
}

impl jb_ingest__aws_network_interface_start {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(406u16, 32, true)
    }
}

impl Default for jb_ingest__aws_network_interface_start {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AWS_NETWORK_INTERFACE_START_WIRE_SIZE: usize = 32;

#[cfg(test)]
mod aws_network_interface_start_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__aws_network_interface_start>();
        let align = align_of::<jb_ingest__aws_network_interface_start>();
        let padded_raw_size = (AWS_NETWORK_INTERFACE_START_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__aws_network_interface_start, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__aws_network_interface_start, _ref),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__aws_network_interface_start, ip),
            16usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__aws_network_interface_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl jb_ingest__aws_network_interface_end {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(407u16, 16, true)
    }
}

impl Default for jb_ingest__aws_network_interface_end {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const AWS_NETWORK_INTERFACE_END_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod aws_network_interface_end_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__aws_network_interface_end>();
        let align = align_of::<jb_ingest__aws_network_interface_end>();
        let padded_raw_size = (AWS_NETWORK_INTERFACE_END_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__aws_network_interface_end, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__aws_network_interface_end, _ref),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__network_interface_info_deprecated {
    pub _rpc_id: u16,
    pub _len: u16,
    pub interface_id: u16,
    pub interface_type: u16,
    pub _ref: u64,
    pub instance_id: u16,
    pub instance_owner_id: u16,
    pub public_dns_name: u16,
    pub private_dns_name: u16,
    pub ip_owner_id: [u8; 18],
    pub vpc_id: [u8; 22],
    pub az: [u8; 16],
}

impl jb_ingest__network_interface_info_deprecated {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(408u16, true)
    }
}

impl Default for jb_ingest__network_interface_info_deprecated {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NETWORK_INTERFACE_INFO_DEPRECATED_WIRE_SIZE: usize = 80;

#[cfg(test)]
mod network_interface_info_deprecated_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__network_interface_info_deprecated>();
        let align = align_of::<jb_ingest__network_interface_info_deprecated>();
        let padded_raw_size =
            (NETWORK_INTERFACE_INFO_DEPRECATED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, _rpc_id),
            0
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, _len),
            2
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, interface_id),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, interface_type),
            6usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, _ref),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, instance_id),
            16usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__network_interface_info_deprecated,
                instance_owner_id
            ),
            18usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__network_interface_info_deprecated,
                public_dns_name
            ),
            20usize
        );
        assert_eq!(
            offset_of!(
                jb_ingest__network_interface_info_deprecated,
                private_dns_name
            ),
            22usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, ip_owner_id),
            24usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, vpc_id),
            42usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info_deprecated, az),
            64usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__network_interface_info {
    pub _rpc_id: u16,
    pub _len: u16,
    pub ip_owner_id: u16,
    pub vpc_id: u16,
    pub _ref: u64,
    pub az: u16,
    pub interface_id: u16,
    pub interface_type: u16,
    pub instance_id: u16,
    pub instance_owner_id: u16,
    pub public_dns_name: u16,
    pub private_dns_name: u16,
}

impl jb_ingest__network_interface_info {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(417u16, true)
    }
}

impl Default for jb_ingest__network_interface_info {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const NETWORK_INTERFACE_INFO_WIRE_SIZE: usize = 30;

#[cfg(test)]
mod network_interface_info_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__network_interface_info>();
        let align = align_of::<jb_ingest__network_interface_info>();
        let padded_raw_size = (NETWORK_INTERFACE_INFO_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__network_interface_info, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__network_interface_info, _len), 2);
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, ip_owner_id),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, vpc_id),
            6usize
        );
        assert_eq!(offset_of!(jb_ingest__network_interface_info, _ref), 8usize);
        assert_eq!(offset_of!(jb_ingest__network_interface_info, az), 16usize);
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, interface_id),
            18usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, interface_type),
            20usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, instance_id),
            22usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, instance_owner_id),
            24usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, public_dns_name),
            26usize
        );
        assert_eq!(
            offset_of!(jb_ingest__network_interface_info, private_dns_name),
            28usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__udp_new_socket {
    pub _rpc_id: u16,
    pub lport: u16,
    pub pid: u32,
    pub sk_id: u32,
    pub laddr: [u8; 16],
}

impl jb_ingest__udp_new_socket {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(328u16, 28, true)
    }
}

impl Default for jb_ingest__udp_new_socket {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_NEW_SOCKET_WIRE_SIZE: usize = 28;

#[cfg(test)]
mod udp_new_socket_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__udp_new_socket>();
        let align = align_of::<jb_ingest__udp_new_socket>();
        let padded_raw_size = (UDP_NEW_SOCKET_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__udp_new_socket, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__udp_new_socket, lport), 2usize);
        assert_eq!(offset_of!(jb_ingest__udp_new_socket, pid), 4usize);
        assert_eq!(offset_of!(jb_ingest__udp_new_socket, sk_id), 8usize);
        assert_eq!(offset_of!(jb_ingest__udp_new_socket, laddr), 12usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__udp_stats_addr_unchanged {
    pub _rpc_id: u16,
    pub is_rx: u8,
    pub sk_id: u32,
    pub packets: u32,
    pub bytes: u32,
}

impl jb_ingest__udp_stats_addr_unchanged {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(330u16, 16, true)
    }
}

impl Default for jb_ingest__udp_stats_addr_unchanged {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_STATS_ADDR_UNCHANGED_WIRE_SIZE: usize = 16;

#[cfg(test)]
mod udp_stats_addr_unchanged_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__udp_stats_addr_unchanged>();
        let align = align_of::<jb_ingest__udp_stats_addr_unchanged>();
        let padded_raw_size = (UDP_STATS_ADDR_UNCHANGED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__udp_stats_addr_unchanged, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_unchanged, is_rx),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_unchanged, sk_id),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_unchanged, packets),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_unchanged, bytes),
            12usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__udp_stats_addr_changed_v4 {
    pub _rpc_id: u16,
    pub rport: u16,
    pub sk_id: u32,
    pub packets: u32,
    pub bytes: u32,
    pub raddr: u32,
    pub is_rx: u8,
}

impl jb_ingest__udp_stats_addr_changed_v4 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(341u16, 21, true)
    }
}

impl Default for jb_ingest__udp_stats_addr_changed_v4 {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_STATS_ADDR_CHANGED_V4_WIRE_SIZE: usize = 21;

#[cfg(test)]
mod udp_stats_addr_changed_v4_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__udp_stats_addr_changed_v4>();
        let align = align_of::<jb_ingest__udp_stats_addr_changed_v4>();
        let padded_raw_size = (UDP_STATS_ADDR_CHANGED_V4_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__udp_stats_addr_changed_v4, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v4, rport),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v4, sk_id),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v4, packets),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v4, bytes),
            12usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v4, raddr),
            16usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v4, is_rx),
            20usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__udp_stats_addr_changed_v6 {
    pub _rpc_id: u16,
    pub rport: u16,
    pub sk_id: u32,
    pub packets: u32,
    pub bytes: u32,
    pub is_rx: u8,
    pub raddr: [u8; 16],
}

impl jb_ingest__udp_stats_addr_changed_v6 {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(350u16, 33, true)
    }
}

impl Default for jb_ingest__udp_stats_addr_changed_v6 {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_STATS_ADDR_CHANGED_V6_WIRE_SIZE: usize = 33;

#[cfg(test)]
mod udp_stats_addr_changed_v6_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__udp_stats_addr_changed_v6>();
        let align = align_of::<jb_ingest__udp_stats_addr_changed_v6>();
        let padded_raw_size = (UDP_STATS_ADDR_CHANGED_V6_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__udp_stats_addr_changed_v6, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v6, rport),
            2usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v6, sk_id),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v6, packets),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v6, bytes),
            12usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v6, is_rx),
            16usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_addr_changed_v6, raddr),
            17usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__dns_response_dep_b {
    pub _rpc_id: u16,
    pub _len: u16,
    pub sk_id: u32,
    pub latency_ns: u64,
    pub total_dn_len: u16,
    pub domain_name: u16,
    pub ipv4_addrs: u16,
}

impl jb_ingest__dns_response_dep_b {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(402u16, true)
    }
}

impl Default for jb_ingest__dns_response_dep_b {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const DNS_RESPONSE_DEP_B_WIRE_SIZE: usize = 22;

#[cfg(test)]
mod dns_response_dep_b_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__dns_response_dep_b>();
        let align = align_of::<jb_ingest__dns_response_dep_b>();
        let padded_raw_size = (DNS_RESPONSE_DEP_B_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__dns_response_dep_b, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__dns_response_dep_b, _len), 2);
        assert_eq!(offset_of!(jb_ingest__dns_response_dep_b, sk_id), 4usize);
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_b, latency_ns),
            8usize
        );
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_b, total_dn_len),
            16usize
        );
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_b, domain_name),
            18usize
        );
        assert_eq!(
            offset_of!(jb_ingest__dns_response_dep_b, ipv4_addrs),
            20usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__dns_timeout {
    pub _rpc_id: u16,
    pub _len: u16,
    pub sk_id: u32,
    pub timeout_ns: u64,
    pub total_dn_len: u16,
}

impl jb_ingest__dns_timeout {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(403u16, true)
    }
}

impl Default for jb_ingest__dns_timeout {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const DNS_TIMEOUT_WIRE_SIZE: usize = 18;

#[cfg(test)]
mod dns_timeout_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__dns_timeout>();
        let align = align_of::<jb_ingest__dns_timeout>();
        let padded_raw_size = (DNS_TIMEOUT_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__dns_timeout, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__dns_timeout, _len), 2);
        assert_eq!(offset_of!(jb_ingest__dns_timeout, sk_id), 4usize);
        assert_eq!(offset_of!(jb_ingest__dns_timeout, timeout_ns), 8usize);
        assert_eq!(offset_of!(jb_ingest__dns_timeout, total_dn_len), 16usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__udp_stats_drops_changed {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub drops: u32,
}

impl jb_ingest__udp_stats_drops_changed {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(405u16, 12, true)
    }
}

impl Default for jb_ingest__udp_stats_drops_changed {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_STATS_DROPS_CHANGED_WIRE_SIZE: usize = 12;

#[cfg(test)]
mod udp_stats_drops_changed_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__udp_stats_drops_changed>();
        let align = align_of::<jb_ingest__udp_stats_drops_changed>();
        let padded_raw_size = (UDP_STATS_DROPS_CHANGED_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__udp_stats_drops_changed, _rpc_id), 0);
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_drops_changed, sk_id),
            4usize
        );
        assert_eq!(
            offset_of!(jb_ingest__udp_stats_drops_changed, drops),
            8usize
        );
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__dns_response {
    pub _rpc_id: u16,
    pub _len: u16,
    pub sk_id: u32,
    pub latency_ns: u64,
    pub total_dn_len: u16,
    pub domain_name: u16,
    pub ipv4_addrs: u16,
    pub client_server: u8,
}

impl jb_ingest__dns_response {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_dynamic(418u16, true)
    }
}

impl Default for jb_ingest__dns_response {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const DNS_RESPONSE_WIRE_SIZE: usize = 23;

#[cfg(test)]
mod dns_response_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__dns_response>();
        let align = align_of::<jb_ingest__dns_response>();
        let padded_raw_size = (DNS_RESPONSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__dns_response, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__dns_response, _len), 2);
        assert_eq!(offset_of!(jb_ingest__dns_response, sk_id), 4usize);
        assert_eq!(offset_of!(jb_ingest__dns_response, latency_ns), 8usize);
        assert_eq!(offset_of!(jb_ingest__dns_response, total_dn_len), 16usize);
        assert_eq!(offset_of!(jb_ingest__dns_response, domain_name), 18usize);
        assert_eq!(offset_of!(jb_ingest__dns_response, ipv4_addrs), 20usize);
        assert_eq!(offset_of!(jb_ingest__dns_response, client_server), 22usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__udp_destroy_socket {
    pub _rpc_id: u16,
    pub sk_id: u32,
}

impl jb_ingest__udp_destroy_socket {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(329u16, 8, true)
    }
}

impl Default for jb_ingest__udp_destroy_socket {
    #[inline]
    fn default() -> Self {
        unsafe { core::mem::zeroed() }
    }
}

pub const UDP_DESTROY_SOCKET_WIRE_SIZE: usize = 8;

#[cfg(test)]
mod udp_destroy_socket_layout_tests {
    use super::*;
    use core::mem::{align_of, offset_of};
    #[test]
    fn struct_size() {
        let size = size_of::<jb_ingest__udp_destroy_socket>();
        let align = align_of::<jb_ingest__udp_destroy_socket>();
        let padded_raw_size = (UDP_DESTROY_SOCKET_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__udp_destroy_socket, _rpc_id), 0);
        assert_eq!(offset_of!(jb_ingest__udp_destroy_socket, sk_id), 4usize);
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub struct jb_ingest__pulse {
    pub _rpc_id: u16,
}

impl jb_ingest__pulse {
    #[inline]
    pub fn metadata() -> render_parser::MessageMetadata {
        render_parser::MessageMetadata::new_fixed(65535u16, 2, true)
    }
}

impl Default for jb_ingest__pulse {
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
        let size = size_of::<jb_ingest__pulse>();
        let align = align_of::<jb_ingest__pulse>();
        let padded_raw_size = (PULSE_WIRE_SIZE + align - 1) / align * align;
        assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
        assert_eq!(offset_of!(jb_ingest__pulse, _rpc_id), 0);
    }
}

#[inline]
pub fn all_message_metadata() -> ::std::vec::Vec<render_parser::MessageMetadata> {
    ::std::vec![
        jb_ingest__pid_info::metadata(),
        jb_ingest__pid_close_info::metadata(),
        jb_ingest__pid_info_create_deprecated::metadata(),
        jb_ingest__pid_info_create::metadata(),
        jb_ingest__pid_cgroup_move::metadata(),
        jb_ingest__pid_set_comm::metadata(),
        jb_ingest__pid_set_cmdline::metadata(),
        jb_ingest__tracked_process_start::metadata(),
        jb_ingest__tracked_process_end::metadata(),
        jb_ingest__set_tgid::metadata(),
        jb_ingest__set_cgroup::metadata(),
        jb_ingest__set_command::metadata(),
        jb_ingest__pid_exit::metadata(),
        jb_ingest__cgroup_create_deprecated::metadata(),
        jb_ingest__cgroup_create::metadata(),
        jb_ingest__cgroup_close::metadata(),
        jb_ingest__container_metadata::metadata(),
        jb_ingest__pod_name::metadata(),
        jb_ingest__nomad_metadata::metadata(),
        jb_ingest__k8s_metadata::metadata(),
        jb_ingest__k8s_metadata_port::metadata(),
        jb_ingest__container_resource_limits_deprecated::metadata(),
        jb_ingest__container_resource_limits::metadata(),
        jb_ingest__container_annotation::metadata(),
        jb_ingest__new_sock_info::metadata(),
        jb_ingest__set_state_ipv4::metadata(),
        jb_ingest__set_state_ipv6::metadata(),
        jb_ingest__socket_stats::metadata(),
        jb_ingest__nat_remapping::metadata(),
        jb_ingest__close_sock_info::metadata(),
        jb_ingest__syn_timeout::metadata(),
        jb_ingest__http_response::metadata(),
        jb_ingest__tcp_reset::metadata(),
        jb_ingest__process_steady_state::metadata(),
        jb_ingest__socket_steady_state::metadata(),
        jb_ingest__version_info::metadata(),
        jb_ingest__set_node_info::metadata(),
        jb_ingest__set_config_label::metadata(),
        jb_ingest__set_availability_zone_deprecated::metadata(),
        jb_ingest__set_iam_role_deprecated::metadata(),
        jb_ingest__set_instance_id_deprecated::metadata(),
        jb_ingest__set_instance_type_deprecated::metadata(),
        jb_ingest__dns_response_fake::metadata(),
        jb_ingest__dns_response_dep_a_deprecated::metadata(),
        jb_ingest__set_config_label_deprecated::metadata(),
        jb_ingest__api_key::metadata(),
        jb_ingest__private_ipv4_addr::metadata(),
        jb_ingest__ipv6_addr::metadata(),
        jb_ingest__public_to_private_ipv4::metadata(),
        jb_ingest__metadata_complete::metadata(),
        jb_ingest__bpf_lost_samples::metadata(),
        jb_ingest__pod_new_legacy::metadata(),
        jb_ingest__pod_new_legacy2::metadata(),
        jb_ingest__pod_new_with_name::metadata(),
        jb_ingest__pod_container_legacy::metadata(),
        jb_ingest__pod_container::metadata(),
        jb_ingest__pod_delete::metadata(),
        jb_ingest__pod_resync::metadata(),
        jb_ingest__span_duration_info::metadata(),
        jb_ingest__heartbeat::metadata(),
        jb_ingest__connect::metadata(),
        jb_ingest__health_check::metadata(),
        jb_ingest__log_message::metadata(),
        jb_ingest__agent_resource_usage::metadata(),
        jb_ingest__cloud_platform::metadata(),
        jb_ingest__os_info_deprecated::metadata(),
        jb_ingest__os_info::metadata(),
        jb_ingest__kernel_headers_source::metadata(),
        jb_ingest__entrypoint_error::metadata(),
        jb_ingest__bpf_compiled::metadata(),
        jb_ingest__begin_telemetry::metadata(),
        jb_ingest__cloud_platform_account_info::metadata(),
        jb_ingest__collector_health::metadata(),
        jb_ingest__system_wide_process_settings::metadata(),
        jb_ingest__collect_blob::metadata(),
        jb_ingest__report_cpu_cores::metadata(),
        jb_ingest__bpf_log::metadata(),
        jb_ingest__aws_network_interface_start::metadata(),
        jb_ingest__aws_network_interface_end::metadata(),
        jb_ingest__network_interface_info_deprecated::metadata(),
        jb_ingest__network_interface_info::metadata(),
        jb_ingest__udp_new_socket::metadata(),
        jb_ingest__udp_stats_addr_unchanged::metadata(),
        jb_ingest__udp_stats_addr_changed_v4::metadata(),
        jb_ingest__udp_stats_addr_changed_v6::metadata(),
        jb_ingest__dns_response_dep_b::metadata(),
        jb_ingest__dns_timeout::metadata(),
        jb_ingest__udp_stats_drops_changed::metadata(),
        jb_ingest__dns_response::metadata(),
        jb_ingest__udp_destroy_socket::metadata(),
        jb_ingest__pulse::metadata(),
    ]
}
