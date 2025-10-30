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

// Parsed struct for flow_start
pub struct flow_start {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub addr1: u128,
    pub port1: u16,
    pub addr2: u128,
    pub port2: u16,
}

impl flow_start {
    pub const RPC_ID: u16 = 421u16;

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

        if body.len() < 48usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let addr1 = u128::from_ne_bytes(body[16usize..16usize + 16].try_into().unwrap());
        let port1 = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let addr2 = u128::from_ne_bytes(body[32usize..32usize + 16].try_into().unwrap());
        let port2 = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            addr1: addr1,
            port1: port1,
            addr2: addr2,
            port2: port2,
        })
    }
}
// Parsed struct for flow_end
pub struct flow_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl flow_end {
    pub const RPC_ID: u16 = 422u16;

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
// Parsed struct for agent_info
pub struct agent_info {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub id: ::std::string::String,
    pub az: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub ns: ::std::string::String,
}

impl agent_info {
    pub const RPC_ID: u16 = 423u16;

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
        let side = body[20usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 21usize;
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
        __b.copy_from_slice(&body[16usize..16usize + 2]);
        let __l_env = u16::from_ne_bytes(__b) as usize;
        if __off + __l_env > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let env = if __l_env == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_env]).into_owned()
        };
        __off += __l_env;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[18usize..18usize + 2]);
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
            _ref: _ref,
            side: side,
            id: id,
            az: az,
            env: env,
            role: role,
            ns: ns,
        })
    }
}
// Parsed struct for task_info
pub struct task_info {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub comm: ::std::string::String,
    pub cgroup_name: ::std::string::String,
}

impl task_info {
    pub const RPC_ID: u16 = 424u16;

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
        let side = body[6usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_comm = u16::from_ne_bytes(__b) as usize;
        if __off + __l_comm > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let comm = if __l_comm == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_comm]).into_owned()
        };
        __off += __l_comm;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cgroup_name = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            side: side,
            comm: comm,
            cgroup_name: cgroup_name,
        })
    }
}
// Parsed struct for socket_info
pub struct socket_info {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub local_addr: [u8; 16],
    pub local_port: u16,
    pub remote_addr: [u8; 16],
    pub remote_port: u16,
    pub is_connector: u8,
    pub remote_dns_name: ::std::string::String,
}

impl socket_info {
    pub const RPC_ID: u16 = 425u16;

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
        let side = body[16usize];
        let local_addr = {
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
        let local_port = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        let remote_addr = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 33usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let remote_port = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        let is_connector = body[49usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 50usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let remote_dns_name = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            side: side,
            local_addr: local_addr,
            local_port: local_port,
            remote_addr: remote_addr,
            remote_port: remote_port,
            is_connector: is_connector,
            remote_dns_name: remote_dns_name,
        })
    }
}
// Parsed struct for k8s_info
pub struct k8s_info {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub pod_uid_suffix: [u8; 64],
    pub pod_uid_hash: u64,
}

impl k8s_info {
    pub const RPC_ID: u16 = 426u16;

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
        let _ref = u64::from_ne_bytes(body[80usize..80usize + 8].try_into().unwrap());
        let side = body[2usize];
        let pod_uid_suffix = {
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
        let pod_uid_hash = u64::from_ne_bytes(body[72usize..72usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            side: side,
            pod_uid_suffix: pod_uid_suffix,
            pod_uid_hash: pod_uid_hash,
        })
    }
}
// Parsed struct for tcp_update
pub struct tcp_update {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub is_rx: u8,
    pub active_sockets: u32,
    pub sum_retrans: u32,
    pub sum_bytes: u64,
    pub sum_srtt: u64,
    pub sum_delivered: u64,
    pub active_rtts: u32,
    pub syn_timeouts: u32,
    pub new_sockets: u32,
    pub tcp_resets: u32,
}

impl tcp_update {
    pub const RPC_ID: u16 = 427u16;

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

        if body.len() < 60usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        let side = body[2usize];
        let is_rx = body[3usize];
        let active_sockets = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let sum_retrans = u32::from_ne_bytes(body[40usize..40usize + 4].try_into().unwrap());
        let sum_bytes = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let sum_srtt = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let sum_delivered = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let active_rtts = u32::from_ne_bytes(body[44usize..44usize + 4].try_into().unwrap());
        let syn_timeouts = u32::from_ne_bytes(body[48usize..48usize + 4].try_into().unwrap());
        let new_sockets = u32::from_ne_bytes(body[52usize..52usize + 4].try_into().unwrap());
        let tcp_resets = u32::from_ne_bytes(body[56usize..56usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            side: side,
            is_rx: is_rx,
            active_sockets: active_sockets,
            sum_retrans: sum_retrans,
            sum_bytes: sum_bytes,
            sum_srtt: sum_srtt,
            sum_delivered: sum_delivered,
            active_rtts: active_rtts,
            syn_timeouts: syn_timeouts,
            new_sockets: new_sockets,
            tcp_resets: tcp_resets,
        })
    }
}
// Parsed struct for udp_update
pub struct udp_update {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub is_rx: u8,
    pub active_sockets: u32,
    pub addr_changes: u32,
    pub packets: u32,
    pub bytes: u64,
    pub drops: u32,
}

impl udp_update {
    pub const RPC_ID: u16 = 428u16;

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

        if body.len() < 36usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let side = body[2usize];
        let is_rx = body[3usize];
        let active_sockets = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let addr_changes = u32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let packets = u32::from_ne_bytes(body[28usize..28usize + 4].try_into().unwrap());
        let bytes = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let drops = u32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            side: side,
            is_rx: is_rx,
            active_sockets: active_sockets,
            addr_changes: addr_changes,
            packets: packets,
            bytes: bytes,
            drops: drops,
        })
    }
}
// Parsed struct for http_update
pub struct http_update {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub client_server: u8,
    pub active_sockets: u32,
    pub sum_code_200: u32,
    pub sum_code_400: u32,
    pub sum_code_500: u32,
    pub sum_code_other: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
}

impl http_update {
    pub const RPC_ID: u16 = 429u16;

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

        if body.len() < 48usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let side = body[2usize];
        let client_server = body[3usize];
        let active_sockets = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let sum_code_200 = u32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());
        let sum_code_400 = u32::from_ne_bytes(body[36usize..36usize + 4].try_into().unwrap());
        let sum_code_500 = u32::from_ne_bytes(body[40usize..40usize + 4].try_into().unwrap());
        let sum_code_other = u32::from_ne_bytes(body[44usize..44usize + 4].try_into().unwrap());
        let sum_total_time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let sum_processing_time_ns =
            u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            side: side,
            client_server: client_server,
            active_sockets: active_sockets,
            sum_code_200: sum_code_200,
            sum_code_400: sum_code_400,
            sum_code_500: sum_code_500,
            sum_code_other: sum_code_other,
            sum_total_time_ns: sum_total_time_ns,
            sum_processing_time_ns: sum_processing_time_ns,
        })
    }
}
// Parsed struct for dns_update
pub struct dns_update {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub client_server: u8,
    pub active_sockets: u32,
    pub requests_a: u32,
    pub requests_aaaa: u32,
    pub responses: u32,
    pub timeouts: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
}

impl dns_update {
    pub const RPC_ID: u16 = 430u16;

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

        if body.len() < 48usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let side = body[2usize];
        let client_server = body[3usize];
        let active_sockets = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let requests_a = u32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());
        let requests_aaaa = u32::from_ne_bytes(body[36usize..36usize + 4].try_into().unwrap());
        let responses = u32::from_ne_bytes(body[40usize..40usize + 4].try_into().unwrap());
        let timeouts = u32::from_ne_bytes(body[44usize..44usize + 4].try_into().unwrap());
        let sum_total_time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let sum_processing_time_ns =
            u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            side: side,
            client_server: client_server,
            active_sockets: active_sockets,
            requests_a: requests_a,
            requests_aaaa: requests_aaaa,
            responses: responses,
            timeouts: timeouts,
            sum_total_time_ns: sum_total_time_ns,
            sum_processing_time_ns: sum_processing_time_ns,
        })
    }
}
// Parsed struct for container_info
pub struct container_info {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub name: ::std::string::String,
    pub pod: ::std::string::String,
    pub role: ::std::string::String,
    pub version: ::std::string::String,
    pub ns: ::std::string::String,
    pub node_type: u8,
}

impl container_info {
    pub const RPC_ID: u16 = 434u16;

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
        let side = body[20usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let node_type = body[21usize];

        // Decode dynamic payload strings
        let mut __off = 22usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
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
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_pod = u16::from_ne_bytes(__b) as usize;
        if __off + __l_pod > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let pod = if __l_pod == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_pod]).into_owned()
        };
        __off += __l_pod;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[16usize..16usize + 2]);
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
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let version = if __l_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_version]).into_owned()
        };
        __off += __l_version;
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
            _ref: _ref,
            side: side,
            name: name,
            pod: pod,
            role: role,
            version: version,
            ns: ns,
            node_type: node_type,
        })
    }
}
// Parsed struct for service_info
pub struct service_info {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub name: ::std::string::String,
}

impl service_info {
    pub const RPC_ID: u16 = 435u16;

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
        let side = body[4usize];
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
            _ref: _ref,
            side: side,
            name: name,
        })
    }
}
// Parsed struct for aws_enrichment_start
pub struct aws_enrichment_start {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub ip: u128,
}

impl aws_enrichment_start {
    pub const RPC_ID: u16 = 431u16;

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
// Parsed struct for aws_enrichment_end
pub struct aws_enrichment_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl aws_enrichment_end {
    pub const RPC_ID: u16 = 432u16;

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
// Parsed struct for aws_enrichment
pub struct aws_enrichment {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub id: ::std::string::String,
}

impl aws_enrichment {
    pub const RPC_ID: u16 = 433u16;

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

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
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
        __b.copy_from_slice(&body[6usize..6usize + 2]);
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
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let id = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            role: role,
            az: az,
            id: id,
        })
    }
}
// Parsed struct for k8s_pod_start
pub struct k8s_pod_start {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub uid_suffix: [u8; 64],
    pub uid_hash: u64,
}

impl k8s_pod_start {
    pub const RPC_ID: u16 = 436u16;

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
        let _ref = u64::from_ne_bytes(body[80usize..80usize + 8].try_into().unwrap());
        let uid_suffix = {
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
        let uid_hash = u64::from_ne_bytes(body[72usize..72usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            uid_suffix: uid_suffix,
            uid_hash: uid_hash,
        })
    }
}
// Parsed struct for k8s_pod_end
pub struct k8s_pod_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl k8s_pod_end {
    pub const RPC_ID: u16 = 437u16;

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
// Parsed struct for set_pod_detail
pub struct set_pod_detail {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub owner_name: ::std::string::String,
    pub pod_name: ::std::string::String,
    pub ns: ::std::string::String,
    pub version: ::std::string::String,
    pub owner_uid: ::std::string::String,
}

impl set_pod_detail {
    pub const RPC_ID: u16 = 438u16;

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
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 20usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
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
        __b.copy_from_slice(&body[18usize..18usize + 2]);
        let __l_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let version = if __l_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_version]).into_owned()
        };
        __off += __l_version;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let owner_uid = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            owner_name: owner_name,
            pod_name: pod_name,
            ns: ns,
            version: version,
            owner_uid: owner_uid,
        })
    }
}
// Parsed struct for k8s_container_start
pub struct k8s_container_start {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub uid_suffix: [u8; 64],
    pub uid_hash: u64,
}

impl k8s_container_start {
    pub const RPC_ID: u16 = 439u16;

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
        let _ref = u64::from_ne_bytes(body[80usize..80usize + 8].try_into().unwrap());
        let uid_suffix = {
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
        let uid_hash = u64::from_ne_bytes(body[72usize..72usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            uid_suffix: uid_suffix,
            uid_hash: uid_hash,
        })
    }
}
// Parsed struct for k8s_container_end
pub struct k8s_container_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl k8s_container_end {
    pub const RPC_ID: u16 = 440u16;

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
// Parsed struct for set_container_pod
pub struct set_container_pod {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub pod_uid_suffix: [u8; 64],
    pub pod_uid_hash: u64,
    pub name: ::std::string::String,
    pub image: ::std::string::String,
}

impl set_container_pod {
    pub const RPC_ID: u16 = 471u16;

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
        let _ref = u64::from_ne_bytes(body[80usize..80usize + 8].try_into().unwrap());
        let pod_uid_suffix = {
            let mut tmp = [0u8; 64];
            let es = 1usize;
            let mut i = 0usize;
            while i < 64usize {
                let off = 6usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let pod_uid_hash = u64::from_ne_bytes(body[72usize..72usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 88usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
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
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let image = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            pod_uid_suffix: pod_uid_suffix,
            pod_uid_hash: pod_uid_hash,
            name: name,
            image: image,
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
