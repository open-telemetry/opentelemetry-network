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

// Parsed struct for agg_root_start
pub struct agg_root_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl agg_root_start {
    pub const RPC_ID: u16 = 461u16;

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
// Parsed struct for agg_root_end
pub struct agg_root_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl agg_root_end {
    pub const RPC_ID: u16 = 462u16;

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
// Parsed struct for update_node
pub struct update_node {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub side: u8,
    pub id: ::std::string::String,
    pub az: ::std::string::String,
    pub role: ::std::string::String,
    pub version: ::std::string::String,
    pub env: ::std::string::String,
    pub ns: ::std::string::String,
    pub node_type: u8,
    pub address: ::std::string::String,
    pub process: ::std::string::String,
    pub container: ::std::string::String,
    pub pod_name: ::std::string::String,
    pub role_uid: ::std::string::String,
}

impl update_node {
    pub const RPC_ID: u16 = 463u16;

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
        let side = body[32usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let node_type = body[33usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 34usize;
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
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[20usize..20usize + 2]);
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
        __b.copy_from_slice(&body[22usize..22usize + 2]);
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
        __b.copy_from_slice(&body[24usize..24usize + 2]);
        let __l_address = u16::from_ne_bytes(__b) as usize;
        if __off + __l_address > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let address = if __l_address == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_address]).into_owned()
        };
        __off += __l_address;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[26usize..26usize + 2]);
        let __l_process = u16::from_ne_bytes(__b) as usize;
        if __off + __l_process > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let process = if __l_process == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_process]).into_owned()
        };
        __off += __l_process;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[28usize..28usize + 2]);
        let __l_container = u16::from_ne_bytes(__b) as usize;
        if __off + __l_container > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let container = if __l_container == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_container]).into_owned()
        };
        __off += __l_container;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[30usize..30usize + 2]);
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
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let role_uid = if __tail == 0 {
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
            role: role,
            version: version,
            env: env,
            ns: ns,
            node_type: node_type,
            address: address,
            process: process,
            container: container,
            pod_name: pod_name,
            role_uid: role_uid,
        })
    }
}
// Parsed struct for update_tcp_metrics
pub struct update_tcp_metrics {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub direction: u8,
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

impl update_tcp_metrics {
    pub const RPC_ID: u16 = 465u16;

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
        let direction = body[2usize];
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
            direction: direction,
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
// Parsed struct for update_udp_metrics
pub struct update_udp_metrics {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub direction: u8,
    pub active_sockets: u32,
    pub addr_changes: u32,
    pub packets: u32,
    pub bytes: u64,
    pub drops: u32,
}

impl update_udp_metrics {
    pub const RPC_ID: u16 = 466u16;

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
        let direction = body[2usize];
        let active_sockets = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let addr_changes = u32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let packets = u32::from_ne_bytes(body[28usize..28usize + 4].try_into().unwrap());
        let bytes = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let drops = u32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            direction: direction,
            active_sockets: active_sockets,
            addr_changes: addr_changes,
            packets: packets,
            bytes: bytes,
            drops: drops,
        })
    }
}
// Parsed struct for update_http_metrics
pub struct update_http_metrics {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub direction: u8,
    pub active_sockets: u32,
    pub sum_code_200: u32,
    pub sum_code_400: u32,
    pub sum_code_500: u32,
    pub sum_code_other: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
}

impl update_http_metrics {
    pub const RPC_ID: u16 = 467u16;

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
        let direction = body[2usize];
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
            direction: direction,
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
// Parsed struct for update_dns_metrics
pub struct update_dns_metrics {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub direction: u8,
    pub active_sockets: u32,
    pub requests_a: u32,
    pub requests_aaaa: u32,
    pub responses: u32,
    pub timeouts: u32,
    pub sum_total_time_ns: u64,
    pub sum_processing_time_ns: u64,
}

impl update_dns_metrics {
    pub const RPC_ID: u16 = 468u16;

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
        let direction = body[2usize];
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
            direction: direction,
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
