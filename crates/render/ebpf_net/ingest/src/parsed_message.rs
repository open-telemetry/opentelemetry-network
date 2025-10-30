/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
// For slice -> array conversions in from_ne_bytes calls
#[allow(unused_imports)]
use core::convert::TryInto;

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum DecodeError {
    BufferTooSmall,
    InvalidRpcId { got: u16 },
    InvalidLength { len: u16 },
    Utf8 { field: &'static str },
}

// Parsed struct for pid_info
pub struct pid_info {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
}

impl pid_info {
    pub const RPC_ID: u16 = 301u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 24usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());
        let comm = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            comm: comm,
        })
    }
}
// Parsed struct for pid_close_info
pub struct pid_close_info {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
}

impl pid_close_info {
    pub const RPC_ID: u16 = 306u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 24usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());
        let comm = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            comm: comm,
        })
    }
}
// Parsed struct for pid_info_create_deprecated
pub struct pid_info_create_deprecated {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
    pub cgroup: u64,
}

impl pid_info_create_deprecated {
    pub const RPC_ID: u16 = 393u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 32usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());
        let comm = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let cgroup = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            comm: comm,
            cgroup: cgroup,
        })
    }
}
// Parsed struct for pid_info_create
pub struct pid_info_create {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
    pub cgroup: u64,
    pub parent_pid: i32,
    pub cmdline: ::std::string::String,
}

impl pid_info_create {
    pub const RPC_ID: u16 = 546u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let comm = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 20usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let parent_pid = i32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 36usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cmdline = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            comm: comm,
            cgroup: cgroup,
            parent_pid: parent_pid,
            cmdline: cmdline,
        })
    }
}
// Parsed struct for pid_cgroup_move
pub struct pid_cgroup_move {
    pub _rpc_id: u16,
    pub pid: u32,
    pub cgroup: u64,
}

impl pid_cgroup_move {
    pub const RPC_ID: u16 = 397u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            cgroup: cgroup,
        })
    }
}
// Parsed struct for pid_set_comm
pub struct pid_set_comm {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
}

impl pid_set_comm {
    pub const RPC_ID: u16 = 399u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 24usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());
        let comm = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            comm: comm,
        })
    }
}
// Parsed struct for pid_set_cmdline
pub struct pid_set_cmdline {
    pub _rpc_id: u16,
    pub pid: u32,
    pub cmdline: ::std::string::String,
}

impl pid_set_cmdline {
    pub const RPC_ID: u16 = 547u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 8usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cmdline = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            cmdline: cmdline,
        })
    }
}
// Parsed struct for tracked_process_start
pub struct tracked_process_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl tracked_process_start {
    pub const RPC_ID: u16 = 500u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
        })
    }
}
// Parsed struct for tracked_process_end
pub struct tracked_process_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl tracked_process_end {
    pub const RPC_ID: u16 = 501u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
        })
    }
}
// Parsed struct for set_tgid
pub struct set_tgid {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub tgid: u32,
}

impl set_tgid {
    pub const RPC_ID: u16 = 502u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let tgid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            tgid: tgid,
        })
    }
}
// Parsed struct for set_cgroup
pub struct set_cgroup {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub cgroup: u64,
}

impl set_cgroup {
    pub const RPC_ID: u16 = 503u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 24usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            cgroup: cgroup,
        })
    }
}
// Parsed struct for set_command
pub struct set_command {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub command: ::std::string::String,
}

impl set_command {
    pub const RPC_ID: u16 = 504u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let command = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            command: command,
        })
    }
}
// Parsed struct for pid_exit
pub struct pid_exit {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub tgid: u64,
    pub pid: u32,
    pub exit_code: i32,
}

impl pid_exit {
    pub const RPC_ID: u16 = 517u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 28usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let tgid = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let exit_code = i32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            tgid: tgid,
            pid: pid,
            exit_code: exit_code,
        })
    }
}
// Parsed struct for cgroup_create_deprecated
pub struct cgroup_create_deprecated {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub cgroup_parent: u64,
    pub name: [u8; 64],
}

impl cgroup_create_deprecated {
    pub const RPC_ID: u16 = 394u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 88usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[72usize..72usize + 8].try_into().unwrap());
        let cgroup_parent = u64::from_ne_bytes(body[80usize..80usize + 8].try_into().unwrap());
        let name = {
            let mut tmp = [0u8; 64];
            let es = 1usize;
            let mut i = 0usize;
            while i < 64usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            cgroup_parent: cgroup_parent,
            name: name,
        })
    }
}
// Parsed struct for cgroup_create
pub struct cgroup_create {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub cgroup_parent: u64,
    pub name: [u8; 256],
}

impl cgroup_create {
    pub const RPC_ID: u16 = 544u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 280usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[264usize..264usize + 8].try_into().unwrap());
        let cgroup_parent = u64::from_ne_bytes(body[272usize..272usize + 8].try_into().unwrap());
        let name = {
            let mut tmp = [0u8; 256];
            let es = 1usize;
            let mut i = 0usize;
            while i < 256usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            cgroup_parent: cgroup_parent,
            name: name,
        })
    }
}
// Parsed struct for cgroup_close
pub struct cgroup_close {
    pub _rpc_id: u16,
    pub cgroup: u64,
}

impl cgroup_close {
    pub const RPC_ID: u16 = 395u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
        })
    }
}
// Parsed struct for container_metadata
pub struct container_metadata {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub id: ::std::string::String,
    pub name: ::std::string::String,
    pub image: ::std::string::String,
    pub ip_addr: ::std::string::String,
    pub cluster: ::std::string::String,
    pub container_name: ::std::string::String,
    pub task_family: ::std::string::String,
    pub task_version: ::std::string::String,
    pub ns: ::std::string::String,
}

impl container_metadata {
    pub const RPC_ID: u16 = 396u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 28usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let id = if __l_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_id]).into_owned()
        };
        __off += __l_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let name = if __l_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_name]).into_owned()
        };
        __off += __l_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[16usize..16usize + 2]);
        let __l_image = u16::from_ne_bytes(__b) as usize;
        if __off + __l_image > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let image = if __l_image == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_image]).into_owned()
        };
        __off += __l_image;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_ip_addr = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ip_addr > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ip_addr = if __l_ip_addr == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ip_addr]).into_owned()
        };
        __off += __l_ip_addr;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[20usize..20usize + 2]);
        let __l_cluster = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cluster > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cluster = if __l_cluster == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cluster]).into_owned()
        };
        __off += __l_cluster;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[22usize..22usize + 2]);
        let __l_container_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_container_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let container_name = if __l_container_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_container_name])
                .into_owned()
        };
        __off += __l_container_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[24usize..24usize + 2]);
        let __l_task_family = u16::from_ne_bytes(__b) as usize;
        if __off + __l_task_family > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let task_family = if __l_task_family == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_task_family])
                .into_owned()
        };
        __off += __l_task_family;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[26usize..26usize + 2]);
        let __l_task_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_task_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let task_version = if __l_task_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_task_version])
                .into_owned()
        };
        __off += __l_task_version;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ns = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            id: id,
            name: name,
            image: image,
            ip_addr: ip_addr,
            cluster: cluster,
            container_name: container_name,
            task_family: task_family,
            task_version: task_version,
            ns: ns,
        })
    }
}
// Parsed struct for pod_name
pub struct pod_name {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub _deprecated_pod_uid: ::std::string::String,
    pub name: ::std::string::String,
}

impl pod_name {
    pub const RPC_ID: u16 = 410u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l__deprecated_pod_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l__deprecated_pod_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let _deprecated_pod_uid = if __l__deprecated_pod_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l__deprecated_pod_uid])
                .into_owned()
        };
        __off += __l__deprecated_pod_uid;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let name = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            _deprecated_pod_uid: _deprecated_pod_uid,
            name: name,
        })
    }
}
// Parsed struct for nomad_metadata
pub struct nomad_metadata {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub ns: ::std::string::String,
    pub group_name: ::std::string::String,
    pub task_name: ::std::string::String,
    pub job_name: ::std::string::String,
}

impl nomad_metadata {
    pub const RPC_ID: u16 = 508u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 18usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_ns = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ns > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ns = if __l_ns == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ns]).into_owned()
        };
        __off += __l_ns;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_group_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_group_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let group_name = if __l_group_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_group_name])
                .into_owned()
        };
        __off += __l_group_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[16usize..16usize + 2]);
        let __l_task_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_task_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let task_name = if __l_task_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_task_name]).into_owned()
        };
        __off += __l_task_name;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let job_name = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            ns: ns,
            group_name: group_name,
            task_name: task_name,
            job_name: job_name,
        })
    }
}
// Parsed struct for k8s_metadata
pub struct k8s_metadata {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub container_name: ::std::string::String,
    pub pod_name: ::std::string::String,
    pub pod_ns: ::std::string::String,
    pub pod_uid: ::std::string::String,
    pub sandbox_uid: ::std::string::String,
}

impl k8s_metadata {
    pub const RPC_ID: u16 = 512u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 20usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_container_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_container_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let container_name = if __l_container_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_container_name])
                .into_owned()
        };
        __off += __l_container_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_pod_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_pod_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let pod_name = if __l_pod_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_pod_name]).into_owned()
        };
        __off += __l_pod_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[16usize..16usize + 2]);
        let __l_pod_ns = u16::from_ne_bytes(__b) as usize;
        if __off + __l_pod_ns > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let pod_ns = if __l_pod_ns == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_pod_ns]).into_owned()
        };
        __off += __l_pod_ns;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_pod_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_pod_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let pod_uid = if __l_pod_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_pod_uid]).into_owned()
        };
        __off += __l_pod_uid;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let sandbox_uid = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            container_name: container_name,
            pod_name: pod_name,
            pod_ns: pod_ns,
            pod_uid: pod_uid,
            sandbox_uid: sandbox_uid,
        })
    }
}
// Parsed struct for k8s_metadata_port
pub struct k8s_metadata_port {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub port: u16,
    pub protocol: u8,
    pub name: ::std::string::String,
}

impl k8s_metadata_port {
    pub const RPC_ID: u16 = 513u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let port = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        let protocol = body[6usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let name = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            port: port,
            protocol: protocol,
            name: name,
        })
    }
}
// Parsed struct for container_resource_limits_deprecated
pub struct container_resource_limits_deprecated {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub cpu_shares: u16,
    pub cpu_period: u16,
    pub cpu_quota: u16,
    pub memory_swappiness: u8,
    pub memory_limit: u64,
    pub memory_soft_limit: u64,
    pub total_memory_limit: i64,
}

impl container_resource_limits_deprecated {
    pub const RPC_ID: u16 = 514u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 41usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let cpu_shares = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let cpu_period = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        let cpu_quota = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        let memory_swappiness = body[40usize];
        let memory_limit = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let memory_soft_limit = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let total_memory_limit = i64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            cpu_shares: cpu_shares,
            cpu_period: cpu_period,
            cpu_quota: cpu_quota,
            memory_swappiness: memory_swappiness,
            memory_limit: memory_limit,
            memory_soft_limit: memory_soft_limit,
            total_memory_limit: total_memory_limit,
        })
    }
}
// Parsed struct for container_resource_limits
pub struct container_resource_limits {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub cpu_shares: u16,
    pub cpu_period: u32,
    pub cpu_quota: u32,
    pub memory_swappiness: u8,
    pub memory_limit: u64,
    pub memory_soft_limit: u64,
    pub total_memory_limit: i64,
}

impl container_resource_limits {
    pub const RPC_ID: u16 = 518u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 45usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let cpu_shares = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let cpu_period = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let cpu_quota = u32::from_ne_bytes(body[40usize..40usize + 4].try_into().unwrap());
        let memory_swappiness = body[44usize];
        let memory_limit = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let memory_soft_limit = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let total_memory_limit = i64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            cpu_shares: cpu_shares,
            cpu_period: cpu_period,
            cpu_quota: cpu_quota,
            memory_swappiness: memory_swappiness,
            memory_limit: memory_limit,
            memory_soft_limit: memory_soft_limit,
            total_memory_limit: total_memory_limit,
        })
    }
}
// Parsed struct for container_annotation
pub struct container_annotation {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub key: ::std::string::String,
    pub value: ::std::string::String,
}

impl container_annotation {
    pub const RPC_ID: u16 = 538u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cgroup = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_key = u16::from_ne_bytes(__b) as usize;
        if __off + __l_key > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let key = if __l_key == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_key]).into_owned()
        };
        __off += __l_key;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let value = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            cgroup: cgroup,
            key: key,
            value: value,
        })
    }
}
// Parsed struct for new_sock_info
pub struct new_sock_info {
    pub _rpc_id: u16,
    pub pid: u32,
    pub sk: u64,
}

impl new_sock_info {
    pub const RPC_ID: u16 = 302u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            sk: sk,
        })
    }
}
// Parsed struct for set_state_ipv4
pub struct set_state_ipv4 {
    pub _rpc_id: u16,
    pub dest: u32,
    pub src: u32,
    pub dport: u16,
    pub sport: u16,
    pub sk: u64,
    pub tx_rx: u32,
}

impl set_state_ipv4 {
    pub const RPC_ID: u16 = 303u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 26usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let dest = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let src = u32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        let dport = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let sport = u16::from_ne_bytes(body[24usize..24usize + 2].try_into().unwrap());
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let tx_rx = u32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            dest: dest,
            src: src,
            dport: dport,
            sport: sport,
            sk: sk,
            tx_rx: tx_rx,
        })
    }
}
// Parsed struct for set_state_ipv6
pub struct set_state_ipv6 {
    pub _rpc_id: u16,
    pub dest: [u8; 16],
    pub src: [u8; 16],
    pub dport: u16,
    pub sport: u16,
    pub sk: u64,
    pub tx_rx: u32,
}

impl set_state_ipv6 {
    pub const RPC_ID: u16 = 304u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 50usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let dest = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 18usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let src = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 34usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let dport = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let sport = u16::from_ne_bytes(body[16usize..16usize + 2].try_into().unwrap());
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let tx_rx = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            dest: dest,
            src: src,
            dport: dport,
            sport: sport,
            sk: sk,
            tx_rx: tx_rx,
        })
    }
}
// Parsed struct for socket_stats
pub struct socket_stats {
    pub _rpc_id: u16,
    pub sk: u64,
    pub diff_bytes: u64,
    pub diff_delivered: u32,
    pub diff_retrans: u32,
    pub max_srtt: u32,
    pub is_rx: u8,
}

impl socket_stats {
    pub const RPC_ID: u16 = 326u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 32usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let diff_bytes = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let diff_delivered = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let diff_retrans = u32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let max_srtt = u32::from_ne_bytes(body[28usize..28usize + 4].try_into().unwrap());
        let is_rx = body[2usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            diff_bytes: diff_bytes,
            diff_delivered: diff_delivered,
            diff_retrans: diff_retrans,
            max_srtt: max_srtt,
            is_rx: is_rx,
        })
    }
}
// Parsed struct for nat_remapping
pub struct nat_remapping {
    pub _rpc_id: u16,
    pub sk: u64,
    pub src: u32,
    pub dst: u32,
    pub sport: u16,
    pub dport: u16,
}

impl nat_remapping {
    pub const RPC_ID: u16 = 360u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 22usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let src = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let dst = u32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        let sport = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let dport = u16::from_ne_bytes(body[20usize..20usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            src: src,
            dst: dst,
            sport: sport,
            dport: dport,
        })
    }
}
// Parsed struct for close_sock_info
pub struct close_sock_info {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl close_sock_info {
    pub const RPC_ID: u16 = 308u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
        })
    }
}
// Parsed struct for syn_timeout
pub struct syn_timeout {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl syn_timeout {
    pub const RPC_ID: u16 = 398u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
        })
    }
}
// Parsed struct for http_response
pub struct http_response {
    pub _rpc_id: u16,
    pub sk: u64,
    pub pid: u32,
    pub code: u16,
    pub latency_ns: u64,
    pub client_server: u8,
}

impl http_response {
    pub const RPC_ID: u16 = 401u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 25usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let code = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let latency_ns = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let client_server = body[24usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            pid: pid,
            code: code,
            latency_ns: latency_ns,
            client_server: client_server,
        })
    }
}
// Parsed struct for tcp_reset
pub struct tcp_reset {
    pub _rpc_id: u16,
    pub sk: u64,
    pub is_rx: u8,
}

impl tcp_reset {
    pub const RPC_ID: u16 = 519u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let is_rx = body[2usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            is_rx: is_rx,
        })
    }
}
// Parsed struct for process_steady_state
pub struct process_steady_state {
    pub _rpc_id: u16,
    pub time: u64,
}

impl process_steady_state {
    pub const RPC_ID: u16 = 307u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let time = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            time: time,
        })
    }
}
// Parsed struct for socket_steady_state
pub struct socket_steady_state {
    pub _rpc_id: u16,
    pub time: u64,
}

impl socket_steady_state {
    pub const RPC_ID: u16 = 309u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let time = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            time: time,
        })
    }
}
// Parsed struct for version_info
pub struct version_info {
    pub _rpc_id: u16,
    pub major: u32,
    pub minor: u32,
    pub patch: u32,
}

impl version_info {
    pub const RPC_ID: u16 = 310u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let major = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let minor = u32::from_ne_bytes(body[8usize..8usize + 4].try_into().unwrap());
        let patch = u32::from_ne_bytes(body[12usize..12usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            major: major,
            minor: minor,
            patch: patch,
        })
    }
}
// Parsed struct for set_node_info
pub struct set_node_info {
    pub _rpc_id: u16,
    pub az: ::std::string::String,
    pub role: ::std::string::String,
    pub instance_id: ::std::string::String,
    pub instance_type: ::std::string::String,
}

impl set_node_info {
    pub const RPC_ID: u16 = 415u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 10usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_az = u16::from_ne_bytes(__b) as usize;
        if __off + __l_az > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let az = if __l_az == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_az]).into_owned()
        };
        __off += __l_az;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_role = u16::from_ne_bytes(__b) as usize;
        if __off + __l_role > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let role = if __l_role == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_role]).into_owned()
        };
        __off += __l_role;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[8usize..8usize + 2]);
        let __l_instance_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_instance_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let instance_id = if __l_instance_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_instance_id])
                .into_owned()
        };
        __off += __l_instance_id;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let instance_type = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            az: az,
            role: role,
            instance_id: instance_id,
            instance_type: instance_type,
        })
    }
}
// Parsed struct for set_config_label
pub struct set_config_label {
    pub _rpc_id: u16,
    pub key: ::std::string::String,
    pub value: ::std::string::String,
}

impl set_config_label {
    pub const RPC_ID: u16 = 416u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 6usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_key = u16::from_ne_bytes(__b) as usize;
        if __off + __l_key > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let key = if __l_key == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_key]).into_owned()
        };
        __off += __l_key;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let value = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            key: key,
            value: value,
        })
    }
}
// Parsed struct for set_availability_zone_deprecated
pub struct set_availability_zone_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub az: [u8; 16],
}

impl set_availability_zone_deprecated {
    pub const RPC_ID: u16 = 321u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 19usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let retcode = body[2usize];
        let az = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 3usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            retcode: retcode,
            az: az,
        })
    }
}
// Parsed struct for set_iam_role_deprecated
pub struct set_iam_role_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub role: [u8; 64],
}

impl set_iam_role_deprecated {
    pub const RPC_ID: u16 = 322u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 67usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let retcode = body[2usize];
        let role = {
            let mut tmp = [0u8; 64];
            let es = 1usize;
            let mut i = 0usize;
            while i < 64usize {
                let off = 3usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            retcode: retcode,
            role: role,
        })
    }
}
// Parsed struct for set_instance_id_deprecated
pub struct set_instance_id_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub id: [u8; 17],
}

impl set_instance_id_deprecated {
    pub const RPC_ID: u16 = 323u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 20usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let retcode = body[2usize];
        let id = {
            let mut tmp = [0u8; 17];
            let es = 1usize;
            let mut i = 0usize;
            while i < 17usize {
                let off = 3usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            retcode: retcode,
            id: id,
        })
    }
}
// Parsed struct for set_instance_type_deprecated
pub struct set_instance_type_deprecated {
    pub _rpc_id: u16,
    pub retcode: u8,
    pub val: [u8; 17],
}

impl set_instance_type_deprecated {
    pub const RPC_ID: u16 = 324u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 20usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let retcode = body[2usize];
        let val = {
            let mut tmp = [0u8; 17];
            let es = 1usize;
            let mut i = 0usize;
            while i < 17usize {
                let off = 3usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            retcode: retcode,
            val: val,
        })
    }
}
// Parsed struct for dns_response_fake
pub struct dns_response_fake {
    pub _rpc_id: u16,
    pub total_dn_len: u16,
    pub ips: ::std::string::String,
    pub domain_name: ::std::string::String,
}

impl dns_response_fake {
    pub const RPC_ID: u16 = 325u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let total_dn_len = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 8usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_ips = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ips > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ips = if __l_ips == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ips]).into_owned()
        };
        __off += __l_ips;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let domain_name = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            total_dn_len: total_dn_len,
            ips: ips,
            domain_name: domain_name,
        })
    }
}
// Parsed struct for dns_response_dep_a_deprecated
pub struct dns_response_dep_a_deprecated {
    pub _rpc_id: u16,
    pub total_dn_len: u16,
    pub domain_name: ::std::string::String,
    pub ipv4_addrs: ::std::string::String,
    pub ipv6_addrs: ::std::string::String,
}

impl dns_response_dep_a_deprecated {
    pub const RPC_ID: u16 = 391u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let total_dn_len = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 10usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_domain_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_domain_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let domain_name = if __l_domain_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_domain_name])
                .into_owned()
        };
        __off += __l_domain_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[8usize..8usize + 2]);
        let __l_ipv4_addrs = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ipv4_addrs > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ipv4_addrs = if __l_ipv4_addrs == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ipv4_addrs])
                .into_owned()
        };
        __off += __l_ipv4_addrs;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ipv6_addrs = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            total_dn_len: total_dn_len,
            domain_name: domain_name,
            ipv4_addrs: ipv4_addrs,
            ipv6_addrs: ipv6_addrs,
        })
    }
}
// Parsed struct for set_config_label_deprecated
pub struct set_config_label_deprecated {
    pub _rpc_id: u16,
    pub key: [u8; 20],
    pub val: [u8; 40],
}

impl set_config_label_deprecated {
    pub const RPC_ID: u16 = 327u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 62usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let key = {
            let mut tmp = [0u8; 20];
            let es = 1usize;
            let mut i = 0usize;
            while i < 20usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let val = {
            let mut tmp = [0u8; 40];
            let es = 1usize;
            let mut i = 0usize;
            while i < 40usize {
                let off = 22usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            key: key,
            val: val,
        })
    }
}
// Parsed struct for api_key
pub struct api_key {
    pub _rpc_id: u16,
    pub tenant: [u8; 20],
    pub api_key: [u8; 64],
}

impl api_key {
    pub const RPC_ID: u16 = 352u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 86usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let tenant = {
            let mut tmp = [0u8; 20];
            let es = 1usize;
            let mut i = 0usize;
            while i < 20usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let api_key = {
            let mut tmp = [0u8; 64];
            let es = 1usize;
            let mut i = 0usize;
            while i < 64usize {
                let off = 22usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            tenant: tenant,
            api_key: api_key,
        })
    }
}
// Parsed struct for private_ipv4_addr
pub struct private_ipv4_addr {
    pub _rpc_id: u16,
    pub addr: u32,
    pub vpc_id: [u8; 22],
}

impl private_ipv4_addr {
    pub const RPC_ID: u16 = 353u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 28usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let addr = u32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let vpc_id = {
            let mut tmp = [0u8; 22];
            let es = 1usize;
            let mut i = 0usize;
            while i < 22usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            addr: addr,
            vpc_id: vpc_id,
        })
    }
}
// Parsed struct for ipv6_addr
pub struct ipv6_addr {
    pub _rpc_id: u16,
    pub addr: [u8; 16],
    pub vpc_id: [u8; 22],
}

impl ipv6_addr {
    pub const RPC_ID: u16 = 354u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 40usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let addr = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let vpc_id = {
            let mut tmp = [0u8; 22];
            let es = 1usize;
            let mut i = 0usize;
            while i < 22usize {
                let off = 18usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            addr: addr,
            vpc_id: vpc_id,
        })
    }
}
// Parsed struct for public_to_private_ipv4
pub struct public_to_private_ipv4 {
    pub _rpc_id: u16,
    pub public_addr: u32,
    pub private_addr: u32,
    pub vpc_id: [u8; 22],
}

impl public_to_private_ipv4 {
    pub const RPC_ID: u16 = 355u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 32usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let public_addr = u32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let private_addr = u32::from_ne_bytes(body[28usize..28usize + 4].try_into().unwrap());
        let vpc_id = {
            let mut tmp = [0u8; 22];
            let es = 1usize;
            let mut i = 0usize;
            while i < 22usize {
                let off = 2usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            public_addr: public_addr,
            private_addr: private_addr,
            vpc_id: vpc_id,
        })
    }
}
// Parsed struct for metadata_complete
pub struct metadata_complete {
    pub _rpc_id: u16,
    pub time: u64,
}

impl metadata_complete {
    pub const RPC_ID: u16 = 356u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let time = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            time: time,
        })
    }
}
// Parsed struct for bpf_lost_samples
pub struct bpf_lost_samples {
    pub _rpc_id: u16,
    pub count: u64,
}

impl bpf_lost_samples {
    pub const RPC_ID: u16 = 357u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let count = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            count: count,
        })
    }
}
// Parsed struct for pod_new_legacy
pub struct pod_new_legacy {
    pub _rpc_id: u16,
    pub uid: ::std::string::String,
    pub ip: u32,
    pub owner_name: ::std::string::String,
    pub owner_kind: u8,
    pub owner_uid: ::std::string::String,
    pub is_host_network: u8,
    pub ns: ::std::string::String,
}

impl pod_new_legacy {
    pub const RPC_ID: u16 = 358u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        let ip = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        // dynamic string; decode later from payload
        let owner_kind = body[14usize];
        // dynamic string; decode later from payload
        let is_host_network = body[15usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[8usize..8usize + 2]);
        let __l_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let uid = if __l_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_uid]).into_owned()
        };
        __off += __l_uid;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[10usize..10usize + 2]);
        let __l_owner_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_owner_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let owner_name = if __l_owner_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_owner_name])
                .into_owned()
        };
        __off += __l_owner_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[12usize..12usize + 2]);
        let __l_owner_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_owner_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let owner_uid = if __l_owner_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_owner_uid]).into_owned()
        };
        __off += __l_owner_uid;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ns = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            uid: uid,
            ip: ip,
            owner_name: owner_name,
            owner_kind: owner_kind,
            owner_uid: owner_uid,
            is_host_network: is_host_network,
            ns: ns,
        })
    }
}
// Parsed struct for pod_new_legacy2
pub struct pod_new_legacy2 {
    pub _rpc_id: u16,
    pub uid: ::std::string::String,
    pub ip: u32,
    pub owner_name: ::std::string::String,
    pub owner_kind: u8,
    pub owner_uid: ::std::string::String,
    pub is_host_network: u8,
    pub ns: ::std::string::String,
    pub version: ::std::string::String,
}

impl pod_new_legacy2 {
    pub const RPC_ID: u16 = 414u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        let ip = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        // dynamic string; decode later from payload
        let owner_kind = body[16usize];
        // dynamic string; decode later from payload
        let is_host_network = body[17usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 18usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[8usize..8usize + 2]);
        let __l_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let uid = if __l_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_uid]).into_owned()
        };
        __off += __l_uid;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[10usize..10usize + 2]);
        let __l_owner_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_owner_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let owner_name = if __l_owner_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_owner_name])
                .into_owned()
        };
        __off += __l_owner_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[12usize..12usize + 2]);
        let __l_owner_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_owner_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let owner_uid = if __l_owner_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_owner_uid]).into_owned()
        };
        __off += __l_owner_uid;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[14usize..14usize + 2]);
        let __l_ns = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ns > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ns = if __l_ns == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ns]).into_owned()
        };
        __off += __l_ns;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let version = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            uid: uid,
            ip: ip,
            owner_name: owner_name,
            owner_kind: owner_kind,
            owner_uid: owner_uid,
            is_host_network: is_host_network,
            ns: ns,
            version: version,
        })
    }
}
// Parsed struct for pod_new_with_name
pub struct pod_new_with_name {
    pub _rpc_id: u16,
    pub uid: ::std::string::String,
    pub ip: u32,
    pub owner_name: ::std::string::String,
    pub pod_name: ::std::string::String,
    pub owner_kind: u8,
    pub owner_uid: ::std::string::String,
    pub is_host_network: u8,
    pub ns: ::std::string::String,
    pub version: ::std::string::String,
}

impl pod_new_with_name {
    pub const RPC_ID: u16 = 515u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        let ip = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let owner_kind = body[18usize];
        // dynamic string; decode later from payload
        let is_host_network = body[19usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 20usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[8usize..8usize + 2]);
        let __l_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let uid = if __l_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_uid]).into_owned()
        };
        __off += __l_uid;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[10usize..10usize + 2]);
        let __l_owner_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_owner_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let owner_name = if __l_owner_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_owner_name])
                .into_owned()
        };
        __off += __l_owner_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[12usize..12usize + 2]);
        let __l_pod_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_pod_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let pod_name = if __l_pod_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_pod_name]).into_owned()
        };
        __off += __l_pod_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[14usize..14usize + 2]);
        let __l_owner_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_owner_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let owner_uid = if __l_owner_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_owner_uid]).into_owned()
        };
        __off += __l_owner_uid;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[16usize..16usize + 2]);
        let __l_ns = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ns > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ns = if __l_ns == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ns]).into_owned()
        };
        __off += __l_ns;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let version = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            uid: uid,
            ip: ip,
            owner_name: owner_name,
            pod_name: pod_name,
            owner_kind: owner_kind,
            owner_uid: owner_uid,
            is_host_network: is_host_network,
            ns: ns,
            version: version,
        })
    }
}
// Parsed struct for pod_container_legacy
pub struct pod_container_legacy {
    pub _rpc_id: u16,
    pub uid: ::std::string::String,
    pub container_id: ::std::string::String,
}

impl pod_container_legacy {
    pub const RPC_ID: u16 = 400u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 6usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let uid = if __l_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_uid]).into_owned()
        };
        __off += __l_uid;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let container_id = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            uid: uid,
            container_id: container_id,
        })
    }
}
// Parsed struct for pod_container
pub struct pod_container {
    pub _rpc_id: u16,
    pub uid: ::std::string::String,
    pub container_id: ::std::string::String,
    pub container_name: ::std::string::String,
    pub container_image: ::std::string::String,
}

impl pod_container {
    pub const RPC_ID: u16 = 494u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 10usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_uid = u16::from_ne_bytes(__b) as usize;
        if __off + __l_uid > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let uid = if __l_uid == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_uid]).into_owned()
        };
        __off += __l_uid;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_container_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_container_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let container_id = if __l_container_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_container_id])
                .into_owned()
        };
        __off += __l_container_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[8usize..8usize + 2]);
        let __l_container_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_container_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let container_name = if __l_container_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_container_name])
                .into_owned()
        };
        __off += __l_container_name;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let container_image = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            uid: uid,
            container_id: container_id,
            container_name: container_name,
            container_image: container_image,
        })
    }
}
// Parsed struct for pod_delete
pub struct pod_delete {
    pub _rpc_id: u16,
    pub uid: ::std::string::String,
}

impl pod_delete {
    pub const RPC_ID: u16 = 359u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 4usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let uid = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            uid: uid,
        })
    }
}
// Parsed struct for pod_resync
pub struct pod_resync {
    pub _rpc_id: u16,
    pub resync_count: u64,
}

impl pod_resync {
    pub const RPC_ID: u16 = 390u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let resync_count = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            resync_count: resync_count,
        })
    }
}
// Parsed struct for span_duration_info
pub struct span_duration_info {
    pub _rpc_id: u16,
    pub duration: u64,
}

impl span_duration_info {
    pub const RPC_ID: u16 = 351u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let duration = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            duration: duration,
        })
    }
}
// Parsed struct for heartbeat
pub struct heartbeat {
    pub _rpc_id: u16,
}

impl heartbeat {
    pub const RPC_ID: u16 = 392u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 2usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields

        // Decode dynamic payload strings

        Ok(Self { _rpc_id: rpc })
    }
}
// Parsed struct for connect
pub struct connect {
    pub _rpc_id: u16,
    pub collector_type: u8,
    pub hostname: ::std::string::String,
}

impl connect {
    pub const RPC_ID: u16 = 548u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let collector_type = body[4usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 5usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let hostname = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            collector_type: collector_type,
            hostname: hostname,
        })
    }
}
// Parsed struct for health_check
pub struct health_check {
    pub _rpc_id: u16,
    pub client_type: u8,
    pub origin: ::std::string::String,
}

impl health_check {
    pub const RPC_ID: u16 = 409u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let client_type = body[4usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 5usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let origin = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            client_type: client_type,
            origin: origin,
        })
    }
}
// Parsed struct for log_message
pub struct log_message {
    pub _rpc_id: u16,
    pub log_level: u8,
    pub message: ::std::string::String,
}

impl log_message {
    pub const RPC_ID: u16 = 411u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let log_level = body[4usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 5usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let message = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            log_level: log_level,
            message: message,
        })
    }
}
// Parsed struct for agent_resource_usage
pub struct agent_resource_usage {
    pub _rpc_id: u16,
    pub user_mode_time_us: u64,
    pub kernel_mode_time_us: u64,
    pub max_resident_set_size: u64,
    pub minor_page_faults: u32,
    pub major_page_faults: u32,
    pub block_input_count: u32,
    pub block_output_count: u32,
    pub voluntary_context_switch_count: u32,
    pub involuntary_context_switch_count: u32,
    pub cpu_usage_by_agent: u16,
    pub cpu_idle: u16,
}

impl agent_resource_usage {
    pub const RPC_ID: u16 = 412u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 54usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let user_mode_time_us = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let kernel_mode_time_us =
            u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let max_resident_set_size =
            u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let minor_page_faults = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let major_page_faults = u32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());
        let block_input_count = u32::from_ne_bytes(body[36usize..36usize + 4].try_into().unwrap());
        let block_output_count = u32::from_ne_bytes(body[40usize..40usize + 4].try_into().unwrap());
        let voluntary_context_switch_count =
            u32::from_ne_bytes(body[44usize..44usize + 4].try_into().unwrap());
        let involuntary_context_switch_count =
            u32::from_ne_bytes(body[48usize..48usize + 4].try_into().unwrap());
        let cpu_usage_by_agent = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let cpu_idle = u16::from_ne_bytes(body[52usize..52usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            user_mode_time_us: user_mode_time_us,
            kernel_mode_time_us: kernel_mode_time_us,
            max_resident_set_size: max_resident_set_size,
            minor_page_faults: minor_page_faults,
            major_page_faults: major_page_faults,
            block_input_count: block_input_count,
            block_output_count: block_output_count,
            voluntary_context_switch_count: voluntary_context_switch_count,
            involuntary_context_switch_count: involuntary_context_switch_count,
            cpu_usage_by_agent: cpu_usage_by_agent,
            cpu_idle: cpu_idle,
        })
    }
}
// Parsed struct for cloud_platform
pub struct cloud_platform {
    pub _rpc_id: u16,
    pub cloud_platform: u16,
}

impl cloud_platform {
    pub const RPC_ID: u16 = 413u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cloud_platform = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            cloud_platform: cloud_platform,
        })
    }
}
// Parsed struct for os_info_deprecated
pub struct os_info_deprecated {
    pub _rpc_id: u16,
    pub os: u8,
    pub flavor: u8,
    pub kernel_version: ::std::string::String,
}

impl os_info_deprecated {
    pub const RPC_ID: u16 = 419u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let os = body[4usize];
        let flavor = body[5usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 6usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            os: os,
            flavor: flavor,
            kernel_version: kernel_version,
        })
    }
}
// Parsed struct for os_info
pub struct os_info {
    pub _rpc_id: u16,
    pub os: u8,
    pub flavor: u8,
    pub os_version: ::std::string::String,
    pub kernel_version: ::std::string::String,
}

impl os_info {
    pub const RPC_ID: u16 = 545u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let os = body[6usize];
        let flavor = body[7usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 8usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_os_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os_version = if __l_os_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os_version])
                .into_owned()
        };
        __off += __l_os_version;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            os: os,
            flavor: flavor,
            os_version: os_version,
            kernel_version: kernel_version,
        })
    }
}
// Parsed struct for kernel_headers_source
pub struct kernel_headers_source {
    pub _rpc_id: u16,
    pub source: u8,
}

impl kernel_headers_source {
    pub const RPC_ID: u16 = 420u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 3usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let source = body[2usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            source: source,
        })
    }
}
// Parsed struct for entrypoint_error
pub struct entrypoint_error {
    pub _rpc_id: u16,
    pub error: u8,
}

impl entrypoint_error {
    pub const RPC_ID: u16 = 491u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 3usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let error = body[2usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            error: error,
        })
    }
}
// Parsed struct for bpf_compiled
pub struct bpf_compiled {
    pub _rpc_id: u16,
}

impl bpf_compiled {
    pub const RPC_ID: u16 = 492u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 2usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields

        // Decode dynamic payload strings

        Ok(Self { _rpc_id: rpc })
    }
}
// Parsed struct for begin_telemetry
pub struct begin_telemetry {
    pub _rpc_id: u16,
}

impl begin_telemetry {
    pub const RPC_ID: u16 = 493u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 2usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields

        // Decode dynamic payload strings

        Ok(Self { _rpc_id: rpc })
    }
}
// Parsed struct for cloud_platform_account_info
pub struct cloud_platform_account_info {
    pub _rpc_id: u16,
    pub account_id: ::std::string::String,
}

impl cloud_platform_account_info {
    pub const RPC_ID: u16 = 495u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 4usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let account_id = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            account_id: account_id,
        })
    }
}
// Parsed struct for collector_health
pub struct collector_health {
    pub _rpc_id: u16,
    pub status: u16,
    pub detail: u16,
}

impl collector_health {
    pub const RPC_ID: u16 = 496u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 6usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let status = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let detail = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            status: status,
            detail: detail,
        })
    }
}
// Parsed struct for system_wide_process_settings
pub struct system_wide_process_settings {
    pub _rpc_id: u16,
    pub clock_ticks_per_second: u64,
    pub memory_page_bytes: u64,
}

impl system_wide_process_settings {
    pub const RPC_ID: u16 = 498u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 24usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let clock_ticks_per_second =
            u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let memory_page_bytes = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            clock_ticks_per_second: clock_ticks_per_second,
            memory_page_bytes: memory_page_bytes,
        })
    }
}
// Parsed struct for collect_blob
pub struct collect_blob {
    pub _rpc_id: u16,
    pub blob_type: u16,
    pub subtype: u64,
    pub metadata: ::std::string::String,
    pub blob: ::std::string::String,
}

impl collect_blob {
    pub const RPC_ID: u16 = 511u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let blob_type = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        let subtype = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_metadata = u16::from_ne_bytes(__b) as usize;
        if __off + __l_metadata > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let metadata = if __l_metadata == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_metadata]).into_owned()
        };
        __off += __l_metadata;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let blob = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            blob_type: blob_type,
            subtype: subtype,
            metadata: metadata,
            blob: blob,
        })
    }
}
// Parsed struct for report_cpu_cores
pub struct report_cpu_cores {
    pub _rpc_id: u16,
    pub cpu_core_count: u32,
}

impl report_cpu_cores {
    pub const RPC_ID: u16 = 536u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 8usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let cpu_core_count = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            cpu_core_count: cpu_core_count,
        })
    }
}
// Parsed struct for bpf_log
pub struct bpf_log {
    pub _rpc_id: u16,
    pub filename: ::std::string::String,
    pub line: u32,
    pub code: u64,
    pub arg0: u64,
    pub arg1: u64,
    pub arg2: u64,
}

impl bpf_log {
    pub const RPC_ID: u16 = 537u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        // dynamic string; decode later from payload
        let line = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let code = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let arg0 = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let arg1 = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let arg2 = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 40usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let filename = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            filename: filename,
            line: line,
            code: code,
            arg0: arg0,
            arg1: arg1,
            arg2: arg2,
        })
    }
}
// Parsed struct for aws_network_interface_start
pub struct aws_network_interface_start {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub ip: u128,
}

impl aws_network_interface_start {
    pub const RPC_ID: u16 = 406u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 32usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let ip = u128::from_ne_bytes(body[16usize..16usize + 16].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            ip: ip,
        })
    }
}
// Parsed struct for aws_network_interface_end
pub struct aws_network_interface_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl aws_network_interface_end {
    pub const RPC_ID: u16 = 407u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
        })
    }
}
// Parsed struct for network_interface_info_deprecated
pub struct network_interface_info_deprecated {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub ip_owner_id: [u8; 18],
    pub vpc_id: [u8; 22],
    pub az: [u8; 16],
    pub interface_id: ::std::string::String,
    pub interface_type: u16,
    pub instance_id: ::std::string::String,
    pub instance_owner_id: ::std::string::String,
    pub public_dns_name: ::std::string::String,
    pub private_dns_name: ::std::string::String,
    pub interface_description: ::std::string::String,
}

impl network_interface_info_deprecated {
    pub const RPC_ID: u16 = 408u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let ip_owner_id = {
            let mut tmp = [0u8; 18];
            let es = 1usize;
            let mut i = 0usize;
            while i < 18usize {
                let off = 24usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let vpc_id = {
            let mut tmp = [0u8; 22];
            let es = 1usize;
            let mut i = 0usize;
            while i < 22usize {
                let off = 42usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let az = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 64usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        // dynamic string; decode later from payload
        let interface_type = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 80usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_interface_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_interface_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let interface_id = if __l_interface_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_interface_id])
                .into_owned()
        };
        __off += __l_interface_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[16usize..16usize + 2]);
        let __l_instance_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_instance_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let instance_id = if __l_instance_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_instance_id])
                .into_owned()
        };
        __off += __l_instance_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_instance_owner_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_instance_owner_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let instance_owner_id = if __l_instance_owner_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_instance_owner_id])
                .into_owned()
        };
        __off += __l_instance_owner_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[20usize..20usize + 2]);
        let __l_public_dns_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_public_dns_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let public_dns_name = if __l_public_dns_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_public_dns_name])
                .into_owned()
        };
        __off += __l_public_dns_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[22usize..22usize + 2]);
        let __l_private_dns_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_private_dns_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let private_dns_name = if __l_private_dns_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_private_dns_name])
                .into_owned()
        };
        __off += __l_private_dns_name;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let interface_description = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            ip_owner_id: ip_owner_id,
            vpc_id: vpc_id,
            az: az,
            interface_id: interface_id,
            interface_type: interface_type,
            instance_id: instance_id,
            instance_owner_id: instance_owner_id,
            public_dns_name: public_dns_name,
            private_dns_name: private_dns_name,
            interface_description: interface_description,
        })
    }
}
// Parsed struct for network_interface_info
pub struct network_interface_info {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub ip_owner_id: ::std::string::String,
    pub vpc_id: ::std::string::String,
    pub az: ::std::string::String,
    pub interface_id: ::std::string::String,
    pub interface_type: u16,
    pub instance_id: ::std::string::String,
    pub instance_owner_id: ::std::string::String,
    pub public_dns_name: ::std::string::String,
    pub private_dns_name: ::std::string::String,
    pub interface_description: ::std::string::String,
}

impl network_interface_info {
    pub const RPC_ID: u16 = 417u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let interface_type = u16::from_ne_bytes(body[20usize..20usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 30usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_ip_owner_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ip_owner_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ip_owner_id = if __l_ip_owner_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ip_owner_id])
                .into_owned()
        };
        __off += __l_ip_owner_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_vpc_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_vpc_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let vpc_id = if __l_vpc_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_vpc_id]).into_owned()
        };
        __off += __l_vpc_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[16usize..16usize + 2]);
        let __l_az = u16::from_ne_bytes(__b) as usize;
        if __off + __l_az > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let az = if __l_az == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_az]).into_owned()
        };
        __off += __l_az;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_interface_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_interface_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let interface_id = if __l_interface_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_interface_id])
                .into_owned()
        };
        __off += __l_interface_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[22usize..22usize + 2]);
        let __l_instance_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_instance_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let instance_id = if __l_instance_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_instance_id])
                .into_owned()
        };
        __off += __l_instance_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[24usize..24usize + 2]);
        let __l_instance_owner_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_instance_owner_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let instance_owner_id = if __l_instance_owner_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_instance_owner_id])
                .into_owned()
        };
        __off += __l_instance_owner_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[26usize..26usize + 2]);
        let __l_public_dns_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_public_dns_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let public_dns_name = if __l_public_dns_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_public_dns_name])
                .into_owned()
        };
        __off += __l_public_dns_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[28usize..28usize + 2]);
        let __l_private_dns_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_private_dns_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let private_dns_name = if __l_private_dns_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_private_dns_name])
                .into_owned()
        };
        __off += __l_private_dns_name;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let interface_description = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            ip_owner_id: ip_owner_id,
            vpc_id: vpc_id,
            az: az,
            interface_id: interface_id,
            interface_type: interface_type,
            instance_id: instance_id,
            instance_owner_id: instance_owner_id,
            public_dns_name: public_dns_name,
            private_dns_name: private_dns_name,
            interface_description: interface_description,
        })
    }
}
// Parsed struct for udp_new_socket
pub struct udp_new_socket {
    pub _rpc_id: u16,
    pub pid: u32,
    pub sk_id: u32,
    pub laddr: [u8; 16],
    pub lport: u16,
}

impl udp_new_socket {
    pub const RPC_ID: u16 = 328u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 28usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let sk_id = u32::from_ne_bytes(body[8usize..8usize + 4].try_into().unwrap());
        let laddr = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 12usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let lport = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            sk_id: sk_id,
            laddr: laddr,
            lport: lport,
        })
    }
}
// Parsed struct for udp_stats_addr_unchanged
pub struct udp_stats_addr_unchanged {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub is_rx: u8,
    pub packets: u32,
    pub bytes: u32,
}

impl udp_stats_addr_unchanged {
    pub const RPC_ID: u16 = 330u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 16usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let is_rx = body[2usize];
        let packets = u32::from_ne_bytes(body[8usize..8usize + 4].try_into().unwrap());
        let bytes = u32::from_ne_bytes(body[12usize..12usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
            is_rx: is_rx,
            packets: packets,
            bytes: bytes,
        })
    }
}
// Parsed struct for udp_stats_addr_changed_v4
pub struct udp_stats_addr_changed_v4 {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub is_rx: u8,
    pub packets: u32,
    pub bytes: u32,
    pub raddr: u32,
    pub rport: u16,
}

impl udp_stats_addr_changed_v4 {
    pub const RPC_ID: u16 = 341u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 21usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let is_rx = body[20usize];
        let packets = u32::from_ne_bytes(body[8usize..8usize + 4].try_into().unwrap());
        let bytes = u32::from_ne_bytes(body[12usize..12usize + 4].try_into().unwrap());
        let raddr = u32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        let rport = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
            is_rx: is_rx,
            packets: packets,
            bytes: bytes,
            raddr: raddr,
            rport: rport,
        })
    }
}
// Parsed struct for udp_stats_addr_changed_v6
pub struct udp_stats_addr_changed_v6 {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub is_rx: u8,
    pub packets: u32,
    pub bytes: u32,
    pub raddr: [u8; 16],
    pub rport: u16,
}

impl udp_stats_addr_changed_v6 {
    pub const RPC_ID: u16 = 350u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 33usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let is_rx = body[16usize];
        let packets = u32::from_ne_bytes(body[8usize..8usize + 4].try_into().unwrap());
        let bytes = u32::from_ne_bytes(body[12usize..12usize + 4].try_into().unwrap());
        let raddr = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 17usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let rport = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
            is_rx: is_rx,
            packets: packets,
            bytes: bytes,
            raddr: raddr,
            rport: rport,
        })
    }
}
// Parsed struct for dns_response_dep_b
pub struct dns_response_dep_b {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub total_dn_len: u16,
    pub domain_name: ::std::string::String,
    pub ipv4_addrs: ::std::string::String,
    pub ipv6_addrs: ::std::string::String,
    pub latency_ns: u64,
}

impl dns_response_dep_b {
    pub const RPC_ID: u16 = 402u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let total_dn_len = u16::from_ne_bytes(body[16usize..16usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let latency_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 22usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_domain_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_domain_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let domain_name = if __l_domain_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_domain_name])
                .into_owned()
        };
        __off += __l_domain_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[20usize..20usize + 2]);
        let __l_ipv4_addrs = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ipv4_addrs > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ipv4_addrs = if __l_ipv4_addrs == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ipv4_addrs])
                .into_owned()
        };
        __off += __l_ipv4_addrs;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ipv6_addrs = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
            total_dn_len: total_dn_len,
            domain_name: domain_name,
            ipv4_addrs: ipv4_addrs,
            ipv6_addrs: ipv6_addrs,
            latency_ns: latency_ns,
        })
    }
}
// Parsed struct for dns_timeout
pub struct dns_timeout {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub total_dn_len: u16,
    pub domain_name: ::std::string::String,
    pub timeout_ns: u64,
}

impl dns_timeout {
    pub const RPC_ID: u16 = 403u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let total_dn_len = u16::from_ne_bytes(body[16usize..16usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        let timeout_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 18usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let domain_name = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
            total_dn_len: total_dn_len,
            domain_name: domain_name,
            timeout_ns: timeout_ns,
        })
    }
}
// Parsed struct for udp_stats_drops_changed
pub struct udp_stats_drops_changed {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub drops: u32,
}

impl udp_stats_drops_changed {
    pub const RPC_ID: u16 = 405u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 12usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let drops = u32::from_ne_bytes(body[8usize..8usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
            drops: drops,
        })
    }
}
// Parsed struct for dns_response
pub struct dns_response {
    pub _rpc_id: u16,
    pub sk_id: u32,
    pub total_dn_len: u16,
    pub domain_name: ::std::string::String,
    pub ipv4_addrs: ::std::string::String,
    pub ipv6_addrs: ::std::string::String,
    pub latency_ns: u64,
    pub client_server: u8,
}

impl dns_response {
    pub const RPC_ID: u16 = 418u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 4 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[2..4]);
        let __len = u16::from_ne_bytes(b2);
        if __len < 4 {
            return Err(DecodeError::InvalidLength { len: __len });
        }
        if body.len() < __len as usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let total_dn_len = u16::from_ne_bytes(body[16usize..16usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let latency_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let client_server = body[22usize];

        // Decode dynamic payload strings
        let mut __off = 23usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_domain_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_domain_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let domain_name = if __l_domain_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_domain_name])
                .into_owned()
        };
        __off += __l_domain_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[20usize..20usize + 2]);
        let __l_ipv4_addrs = u16::from_ne_bytes(__b) as usize;
        if __off + __l_ipv4_addrs > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ipv4_addrs = if __l_ipv4_addrs == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_ipv4_addrs])
                .into_owned()
        };
        __off += __l_ipv4_addrs;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let ipv6_addrs = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
            total_dn_len: total_dn_len,
            domain_name: domain_name,
            ipv4_addrs: ipv4_addrs,
            ipv6_addrs: ipv6_addrs,
            latency_ns: latency_ns,
            client_server: client_server,
        })
    }
}
// Parsed struct for udp_destroy_socket
pub struct udp_destroy_socket {
    pub _rpc_id: u16,
    pub sk_id: u32,
}

impl udp_destroy_socket {
    pub const RPC_ID: u16 = 329u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 8usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let sk_id = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk_id: sk_id,
        })
    }
}
// Parsed struct for pulse
pub struct pulse {
    pub _rpc_id: u16,
}

impl pulse {
    pub const RPC_ID: u16 = 65535u16;

    #[inline]
    pub fn decode(body: &[u8]) -> Result<Self, DecodeError> {
        // Require rpc_id
        if body.len() < 2 {
            return Err(DecodeError::BufferTooSmall);
        }
        let mut b2 = [0u8; 2];
        b2.copy_from_slice(&body[0..2]);
        let rpc = u16::from_ne_bytes(b2);
        if rpc != Self::RPC_ID {
            return Err(DecodeError::InvalidRpcId { got: rpc });
        }

        if body.len() < 2usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields

        // Decode dynamic payload strings

        Ok(Self { _rpc_id: rpc })
    }
}
