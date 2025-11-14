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

// Parsed struct for logger_start
pub struct logger_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl logger_start {
    pub const RPC_ID: u16 = 600u16;

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
// Parsed struct for logger_end
pub struct logger_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl logger_end {
    pub const RPC_ID: u16 = 601u16;

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
// Parsed struct for agent_lost_events
pub struct agent_lost_events {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub count: u32,
    pub client_hostname: ::std::string::String,
}

impl agent_lost_events {
    pub const RPC_ID: u16 = 602u16;

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
        let count = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            count: count,
            client_hostname: client_hostname,
        })
    }
}
// Parsed struct for pod_not_found
pub struct pod_not_found {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub uid: ::std::string::String,
    pub on_delete: u8,
}

impl pod_not_found {
    pub const RPC_ID: u16 = 603u16;

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
        let on_delete = body[4usize];

        // Decode dynamic payload strings
        let mut __off = 16usize;
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
            _ref: _ref,
            uid: uid,
            on_delete: on_delete,
        })
    }
}
// Parsed struct for cgroup_not_found
pub struct cgroup_not_found {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub cgroup: u64,
}

impl cgroup_not_found {
    pub const RPC_ID: u16 = 604u16;

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
// Parsed struct for rewriting_private_to_public_ip_mapping
pub struct rewriting_private_to_public_ip_mapping {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub private_addr: ::std::string::String,
    pub existing_public_addr: ::std::string::String,
    pub new_public_addr: ::std::string::String,
}

impl rewriting_private_to_public_ip_mapping {
    pub const RPC_ID: u16 = 605u16;

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
        let __l_private_addr = u16::from_ne_bytes(__b) as usize;
        if __off + __l_private_addr > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let private_addr = if __l_private_addr == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_private_addr])
                .into_owned()
        };
        __off += __l_private_addr;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[6usize..6usize + 2]);
        let __l_existing_public_addr = u16::from_ne_bytes(__b) as usize;
        if __off + __l_existing_public_addr > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let existing_public_addr = if __l_existing_public_addr == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_existing_public_addr])
                .into_owned()
        };
        __off += __l_existing_public_addr;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let new_public_addr = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            private_addr: private_addr,
            existing_public_addr: existing_public_addr,
            new_public_addr: new_public_addr,
        })
    }
}
// Parsed struct for private_ip_in_private_to_public_ip_mapping
pub struct private_ip_in_private_to_public_ip_mapping {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub private_addr: ::std::string::String,
    pub existing_public_addr: ::std::string::String,
}

impl private_ip_in_private_to_public_ip_mapping {
    pub const RPC_ID: u16 = 606u16;

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

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_private_addr = u16::from_ne_bytes(__b) as usize;
        if __off + __l_private_addr > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let private_addr = if __l_private_addr == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_private_addr])
                .into_owned()
        };
        __off += __l_private_addr;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let existing_public_addr = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            private_addr: private_addr,
            existing_public_addr: existing_public_addr,
        })
    }
}
// Parsed struct for failed_to_insert_dns_record
pub struct failed_to_insert_dns_record {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl failed_to_insert_dns_record {
    pub const RPC_ID: u16 = 607u16;

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
// Parsed struct for tcp_socket_failed_getting_process_reference
pub struct tcp_socket_failed_getting_process_reference {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub pid: u32,
}

impl tcp_socket_failed_getting_process_reference {
    pub const RPC_ID: u16 = 608u16;

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
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            pid: pid,
        })
    }
}
// Parsed struct for udp_socket_failed_getting_process_reference
pub struct udp_socket_failed_getting_process_reference {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub pid: u32,
}

impl udp_socket_failed_getting_process_reference {
    pub const RPC_ID: u16 = 609u16;

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
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            pid: pid,
        })
    }
}
// Parsed struct for socket_address_already_assigned
pub struct socket_address_already_assigned {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl socket_address_already_assigned {
    pub const RPC_ID: u16 = 610u16;

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
// Parsed struct for ingest_decompression_error
pub struct ingest_decompression_error {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub client_type: u8,
    pub client_hostname: ::std::string::String,
    pub error: ::std::string::String,
}

impl ingest_decompression_error {
    pub const RPC_ID: u16 = 611u16;

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
        let client_type = body[6usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_client_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_client_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __l_client_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_client_hostname])
                .into_owned()
        };
        __off += __l_client_hostname;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let error = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            client_type: client_type,
            client_hostname: client_hostname,
            error: error,
        })
    }
}
// Parsed struct for ingest_processing_error
pub struct ingest_processing_error {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub client_type: u8,
    pub client_hostname: ::std::string::String,
    pub error: ::std::string::String,
}

impl ingest_processing_error {
    pub const RPC_ID: u16 = 612u16;

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
        let client_type = body[6usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_client_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_client_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __l_client_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_client_hostname])
                .into_owned()
        };
        __off += __l_client_hostname;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let error = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            client_type: client_type,
            client_hostname: client_hostname,
            error: error,
        })
    }
}
// Parsed struct for ingest_connection_error
pub struct ingest_connection_error {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub client_type: u8,
    pub client_hostname: ::std::string::String,
    pub error: ::std::string::String,
}

impl ingest_connection_error {
    pub const RPC_ID: u16 = 613u16;

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
        let client_type = body[6usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_client_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_client_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __l_client_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_client_hostname])
                .into_owned()
        };
        __off += __l_client_hostname;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let error = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            client_type: client_type,
            client_hostname: client_hostname,
            error: error,
        })
    }
}
// Parsed struct for agent_auth_success
pub struct agent_auth_success {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub client_type: u8,
    pub client_hostname: ::std::string::String,
}

impl agent_auth_success {
    pub const RPC_ID: u16 = 614u16;

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
        let client_type = body[4usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            client_type: client_type,
            client_hostname: client_hostname,
        })
    }
}
// Parsed struct for agent_auth_failure
pub struct agent_auth_failure {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub client_type: u8,
    pub client_hostname: ::std::string::String,
    pub error: ::std::string::String,
}

impl agent_auth_failure {
    pub const RPC_ID: u16 = 615u16;

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
        let client_type = body[6usize];
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_client_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_client_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __l_client_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_client_hostname])
                .into_owned()
        };
        __off += __l_client_hostname;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let error = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            client_type: client_type,
            client_hostname: client_hostname,
            error: error,
        })
    }
}
// Parsed struct for agent_attempting_auth_using_api_key
pub struct agent_attempting_auth_using_api_key {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub client_type: u8,
    pub client_hostname: ::std::string::String,
}

impl agent_attempting_auth_using_api_key {
    pub const RPC_ID: u16 = 616u16;

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
        let client_type = body[4usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            client_type: client_type,
            client_hostname: client_hostname,
        })
    }
}
// Parsed struct for k8s_container_pod_not_found
pub struct k8s_container_pod_not_found {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub pod_uid_suffix: [u8; 64],
    pub pod_uid_hash: u64,
}

impl k8s_container_pod_not_found {
    pub const RPC_ID: u16 = 617u16;

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
        let pod_uid_suffix = {
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
        let pod_uid_hash = u64::from_ne_bytes(body[72usize..72usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            pod_uid_suffix: pod_uid_suffix,
            pod_uid_hash: pod_uid_hash,
        })
    }
}
// Parsed struct for agent_connect_success
pub struct agent_connect_success {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub client_type: u8,
    pub client_hostname: ::std::string::String,
}

impl agent_connect_success {
    pub const RPC_ID: u16 = 618u16;

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
        let client_type = body[4usize];
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 16usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_hostname = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            client_type: client_type,
            client_hostname: client_hostname,
        })
    }
}
// Parsed struct for core_stats_start
pub struct core_stats_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl core_stats_start {
    pub const RPC_ID: u16 = 619u16;

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
// Parsed struct for core_stats_end
pub struct core_stats_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl core_stats_end {
    pub const RPC_ID: u16 = 620u16;

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
// Parsed struct for span_utilization_stats
pub struct span_utilization_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub span_name: ::std::string::String,
    pub module: ::std::string::String,
    pub shard: u16,
    pub allocated: u16,
    pub max_allocated: u16,
    pub pool_size_: u16,
    pub time_ns: u64,
}

impl span_utilization_stats {
    pub const RPC_ID: u16 = 621u16;

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
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        let allocated = u16::from_ne_bytes(body[24usize..24usize + 2].try_into().unwrap());
        let max_allocated = u16::from_ne_bytes(body[26usize..26usize + 2].try_into().unwrap());
        let pool_size_ = u16::from_ne_bytes(body[28usize..28usize + 2].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 30usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_span_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_span_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let span_name = if __l_span_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_span_name]).into_owned()
        };
        __off += __l_span_name;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            span_name: span_name,
            module: module,
            shard: shard,
            allocated: allocated,
            max_allocated: max_allocated,
            pool_size_: pool_size_,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for connection_message_stats
pub struct connection_message_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub msg_: ::std::string::String,
    pub shard: u16,
    pub severity_: u32,
    pub conn: u16,
    pub time_ns: u64,
    pub count: u64,
}

impl connection_message_stats {
    pub const RPC_ID: u16 = 622u16;

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
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[34usize..34usize + 2].try_into().unwrap());
        let severity_ = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let conn = u16::from_ne_bytes(body[36usize..36usize + 2].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let count = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 38usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[32usize..32usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let msg_ = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            msg_: msg_,
            shard: shard,
            severity_: severity_,
            conn: conn,
            time_ns: time_ns,
            count: count,
        })
    }
}
// Parsed struct for connection_message_error_stats
pub struct connection_message_error_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub conn: u16,
    pub msg_: ::std::string::String,
    pub error: ::std::string::String,
    pub count: u64,
    pub time_ns: u64,
}

impl connection_message_error_stats {
    pub const RPC_ID: u16 = 623u16;

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
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        let conn = u16::from_ne_bytes(body[32usize..32usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let count = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 36usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[34usize..34usize + 2]);
        let __l_msg_ = u16::from_ne_bytes(__b) as usize;
        if __off + __l_msg_ > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let msg_ = if __l_msg_ == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_msg_]).into_owned()
        };
        __off += __l_msg_;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let error = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            conn: conn,
            msg_: msg_,
            error: error,
            count: count,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for status_stats
pub struct status_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub program: ::std::string::String,
    pub version: ::std::string::String,
    pub status: u8,
    pub time_ns: u64,
}

impl status_stats {
    pub const RPC_ID: u16 = 624u16;

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
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let status = body[26usize];
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 27usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[24usize..24usize + 2]);
        let __l_program = u16::from_ne_bytes(__b) as usize;
        if __off + __l_program > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let program = if __l_program == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_program]).into_owned()
        };
        __off += __l_program;
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
            _ref: _ref,
            module: module,
            shard: shard,
            program: program,
            version: version,
            status: status,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for rpc_receive_stats
pub struct rpc_receive_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub receiver_app: ::std::string::String,
    pub shard: u16,
    pub sender_app: ::std::string::String,
    pub max_latency_ns: u64,
    pub time_ns: u64,
}

impl rpc_receive_stats {
    pub const RPC_ID: u16 = 625u16;

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
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        let max_latency_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 32usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_receiver_app = u16::from_ne_bytes(__b) as usize;
        if __off + __l_receiver_app > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let receiver_app = if __l_receiver_app == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_receiver_app])
                .into_owned()
        };
        __off += __l_receiver_app;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let sender_app = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            receiver_app: receiver_app,
            shard: shard,
            sender_app: sender_app,
            max_latency_ns: max_latency_ns,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for rpc_write_stalls_stats
pub struct rpc_write_stalls_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub sender_app: ::std::string::String,
    pub shard: u16,
    pub receiver_app: ::std::string::String,
    pub count: u64,
    pub time_ns: u64,
}

impl rpc_write_stalls_stats {
    pub const RPC_ID: u16 = 626u16;

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
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        let count = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 32usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_sender_app = u16::from_ne_bytes(__b) as usize;
        if __off + __l_sender_app > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let sender_app = if __l_sender_app == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_sender_app])
                .into_owned()
        };
        __off += __l_sender_app;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let receiver_app = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            sender_app: sender_app,
            shard: shard,
            receiver_app: receiver_app,
            count: count,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for rpc_write_utilization_stats
pub struct rpc_write_utilization_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub sender_app: ::std::string::String,
    pub shard: u16,
    pub receiver_app: ::std::string::String,
    pub max_buf_used: u32,
    pub max_buf_util: u64,
    pub max_elem_util: u64,
    pub time_ns: u64,
}

impl rpc_write_utilization_stats {
    pub const RPC_ID: u16 = 627u16;

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
        let _ref = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[42usize..42usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        let max_buf_used = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let max_buf_util = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let max_elem_util = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 44usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[40usize..40usize + 2]);
        let __l_sender_app = u16::from_ne_bytes(__b) as usize;
        if __off + __l_sender_app > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let sender_app = if __l_sender_app == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_sender_app])
                .into_owned()
        };
        __off += __l_sender_app;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let receiver_app = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            sender_app: sender_app,
            shard: shard,
            receiver_app: receiver_app,
            max_buf_used: max_buf_used,
            max_buf_util: max_buf_util,
            max_elem_util: max_elem_util,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for code_timing_stats
pub struct code_timing_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub name: ::std::string::String,
    pub filename: ::std::string::String,
    pub line: u16,
    pub index_string: u64,
    pub count: u64,
    pub avg_ns: u64,
    pub min_ns: u64,
    pub sum_ns: u64,
    pub max_ns: u64,
    pub time_ns: u64,
}

impl code_timing_stats {
    pub const RPC_ID: u16 = 628u16;

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
        let _ref = u64::from_ne_bytes(body[64usize..64usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let line = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        let index_string = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let count = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let avg_ns = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let min_ns = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        let sum_ns = u64::from_ne_bytes(body[48usize..48usize + 8].try_into().unwrap());
        let max_ns = u64::from_ne_bytes(body[40usize..40usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[56usize..56usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 72usize;
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
        let filename = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            name: name,
            filename: filename,
            line: line,
            index_string: index_string,
            count: count,
            avg_ns: avg_ns,
            min_ns: min_ns,
            sum_ns: sum_ns,
            max_ns: max_ns,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for agg_core_stats_start
pub struct agg_core_stats_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl agg_core_stats_start {
    pub const RPC_ID: u16 = 629u16;

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
// Parsed struct for agg_core_stats_end
pub struct agg_core_stats_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl agg_core_stats_end {
    pub const RPC_ID: u16 = 630u16;

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
// Parsed struct for agg_root_truncation_stats
pub struct agg_root_truncation_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub field: ::std::string::String,
    pub count: u64,
    pub time_ns: u64,
}

impl agg_root_truncation_stats {
    pub const RPC_ID: u16 = 631u16;

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
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        let count = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 32usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let field = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            field: field,
            count: count,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for agg_prometheus_bytes_stats
pub struct agg_prometheus_bytes_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub prometheus_bytes_written: u64,
    pub prometheus_bytes_discarded: u64,
    pub time_ns: u64,
}

impl agg_prometheus_bytes_stats {
    pub const RPC_ID: u16 = 632u16;

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
        let _ref = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        let prometheus_bytes_written =
            u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let prometheus_bytes_discarded =
            u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 40usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            prometheus_bytes_written: prometheus_bytes_written,
            prometheus_bytes_discarded: prometheus_bytes_discarded,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for agg_otlp_grpc_stats
pub struct agg_otlp_grpc_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub client_type: ::std::string::String,
    pub bytes_failed: u64,
    pub bytes_sent: u64,
    pub data_points_failed: u64,
    pub data_points_sent: u64,
    pub requests_failed: u64,
    pub requests_sent: u64,
    pub unknown_response_tags: u64,
    pub time_ns: u64,
}

impl agg_otlp_grpc_stats {
    pub const RPC_ID: u16 = 644u16;

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
        let _ref = u64::from_ne_bytes(body[72usize..72usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        let bytes_failed = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let bytes_sent = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let data_points_failed = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let data_points_sent = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        let requests_failed = u64::from_ne_bytes(body[40usize..40usize + 8].try_into().unwrap());
        let requests_sent = u64::from_ne_bytes(body[48usize..48usize + 8].try_into().unwrap());
        let unknown_response_tags =
            u64::from_ne_bytes(body[56usize..56usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[64usize..64usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 80usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let client_type = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            client_type: client_type,
            bytes_failed: bytes_failed,
            bytes_sent: bytes_sent,
            data_points_failed: data_points_failed,
            data_points_sent: data_points_sent,
            requests_failed: requests_failed,
            requests_sent: requests_sent,
            unknown_response_tags: unknown_response_tags,
            time_ns: time_ns,
        })
    }
}
// Parsed struct for ingest_core_stats_start
pub struct ingest_core_stats_start {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl ingest_core_stats_start {
    pub const RPC_ID: u16 = 633u16;

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
// Parsed struct for ingest_core_stats_end
pub struct ingest_core_stats_end {
    pub _rpc_id: u16,
    pub _ref: u64,
}

impl ingest_core_stats_end {
    pub const RPC_ID: u16 = 634u16;

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
// Parsed struct for client_handle_pool_stats
pub struct client_handle_pool_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub span_name: ::std::string::String,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub agent_hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub client_handle_pool: u64,
    pub client_handle_pool_fraction: u64,
}

impl client_handle_pool_stats {
    pub const RPC_ID: u16 = 635u16;

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
        let _ref = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[56usize..56usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let client_handle_pool = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let client_handle_pool_fraction =
            u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 62usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[40usize..40usize + 2]);
        let __l_span_name = u16::from_ne_bytes(__b) as usize;
        if __off + __l_span_name > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let span_name = if __l_span_name == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_span_name]).into_owned()
        };
        __off += __l_span_name;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[42usize..42usize + 2]);
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
        __b.copy_from_slice(&body[44usize..44usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[46usize..46usize + 2]);
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
        __b.copy_from_slice(&body[48usize..48usize + 2]);
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
        __b.copy_from_slice(&body[50usize..50usize + 2]);
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
        __b.copy_from_slice(&body[52usize..52usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[54usize..54usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[58usize..58usize + 2]);
        let __l_agent_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_agent_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let agent_hostname = if __l_agent_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_agent_hostname])
                .into_owned()
        };
        __off += __l_agent_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[60usize..60usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os_version = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            span_name: span_name,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            agent_hostname: agent_hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            client_handle_pool: client_handle_pool,
            client_handle_pool_fraction: client_handle_pool_fraction,
        })
    }
}
// Parsed struct for agent_connection_message_stats
pub struct agent_connection_message_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub agent_hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub message: ::std::string::String,
    pub severity_: u16,
    pub count: u64,
}

impl agent_connection_message_stats {
    pub const RPC_ID: u16 = 636u16;

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
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[46usize..46usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let severity_ = u16::from_ne_bytes(body[54usize..54usize + 2].try_into().unwrap());
        let count = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 56usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[32usize..32usize + 2]);
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
        __b.copy_from_slice(&body[34usize..34usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[36usize..36usize + 2]);
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
        __b.copy_from_slice(&body[38usize..38usize + 2]);
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
        __b.copy_from_slice(&body[40usize..40usize + 2]);
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
        __b.copy_from_slice(&body[42usize..42usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[44usize..44usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[48usize..48usize + 2]);
        let __l_agent_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_agent_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let agent_hostname = if __l_agent_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_agent_hostname])
                .into_owned()
        };
        __off += __l_agent_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[50usize..50usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[52usize..52usize + 2]);
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
        let message = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            agent_hostname: agent_hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            message: message,
            severity_: severity_,
            count: count,
        })
    }
}
// Parsed struct for agent_connection_message_error_stats
pub struct agent_connection_message_error_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub agent_hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub message: ::std::string::String,
    pub error: ::std::string::String,
    pub count: u64,
}

impl agent_connection_message_error_stats {
    pub const RPC_ID: u16 = 637u16;

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
        let _ref = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[46usize..46usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let count = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 56usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[32usize..32usize + 2]);
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
        __b.copy_from_slice(&body[34usize..34usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[36usize..36usize + 2]);
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
        __b.copy_from_slice(&body[38usize..38usize + 2]);
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
        __b.copy_from_slice(&body[40usize..40usize + 2]);
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
        __b.copy_from_slice(&body[42usize..42usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[44usize..44usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[48usize..48usize + 2]);
        let __l_agent_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_agent_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let agent_hostname = if __l_agent_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_agent_hostname])
                .into_owned()
        };
        __off += __l_agent_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[50usize..50usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[52usize..52usize + 2]);
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
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[54usize..54usize + 2]);
        let __l_message = u16::from_ne_bytes(__b) as usize;
        if __off + __l_message > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let message = if __l_message == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_message]).into_owned()
        };
        __off += __l_message;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let error = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            agent_hostname: agent_hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            message: message,
            error: error,
            count: count,
        })
    }
}
// Parsed struct for connection_stats
pub struct connection_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub agent_hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub time_since_last_message_ns: u64,
    pub clock_offset_ns: u64,
}

impl connection_stats {
    pub const RPC_ID: u16 = 638u16;

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
        let _ref = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[54usize..54usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let time_since_last_message_ns =
            u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let clock_offset_ns = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 60usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[40usize..40usize + 2]);
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
        __b.copy_from_slice(&body[42usize..42usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[44usize..44usize + 2]);
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
        __b.copy_from_slice(&body[46usize..46usize + 2]);
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
        __b.copy_from_slice(&body[48usize..48usize + 2]);
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
        __b.copy_from_slice(&body[50usize..50usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[52usize..52usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[56usize..56usize + 2]);
        let __l_agent_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_agent_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let agent_hostname = if __l_agent_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_agent_hostname])
                .into_owned()
        };
        __off += __l_agent_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[58usize..58usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os_version = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            agent_hostname: agent_hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            time_since_last_message_ns: time_since_last_message_ns,
            clock_offset_ns: clock_offset_ns,
        })
    }
}
// Parsed struct for collector_log_stats
pub struct collector_log_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub agent_hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub severity_: ::std::string::String,
    pub count: u32,
}

impl collector_log_stats {
    pub const RPC_ID: u16 = 639u16;

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
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[26usize..26usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[42usize..42usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let count = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 50usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[24usize..24usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[28usize..28usize + 2]);
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
        __b.copy_from_slice(&body[30usize..30usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[32usize..32usize + 2]);
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
        __b.copy_from_slice(&body[34usize..34usize + 2]);
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
        __b.copy_from_slice(&body[36usize..36usize + 2]);
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
        __b.copy_from_slice(&body[38usize..38usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[40usize..40usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[44usize..44usize + 2]);
        let __l_agent_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_agent_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let agent_hostname = if __l_agent_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_agent_hostname])
                .into_owned()
        };
        __off += __l_agent_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[46usize..46usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[48usize..48usize + 2]);
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
        let severity_ = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            agent_hostname: agent_hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            severity_: severity_,
            count: count,
        })
    }
}
// Parsed struct for entry_point_stats
pub struct entry_point_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub agent_hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub kernel_headers_source: ::std::string::String,
    pub entrypoint_error: ::std::string::String,
    pub entrypoint_info: u8,
}

impl entry_point_stats {
    pub const RPC_ID: u16 = 640u16;

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
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[38usize..38usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let entrypoint_info = body[48usize];

        // Decode dynamic payload strings
        let mut __off = 49usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[24usize..24usize + 2]);
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
        __b.copy_from_slice(&body[26usize..26usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[28usize..28usize + 2]);
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
        __b.copy_from_slice(&body[30usize..30usize + 2]);
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
        __b.copy_from_slice(&body[32usize..32usize + 2]);
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
        __b.copy_from_slice(&body[34usize..34usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[36usize..36usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[40usize..40usize + 2]);
        let __l_agent_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_agent_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let agent_hostname = if __l_agent_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_agent_hostname])
                .into_owned()
        };
        __off += __l_agent_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[42usize..42usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[44usize..44usize + 2]);
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
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[46usize..46usize + 2]);
        let __l_kernel_headers_source = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_headers_source > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_headers_source = if __l_kernel_headers_source == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_headers_source])
                .into_owned()
        };
        __off += __l_kernel_headers_source;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let entrypoint_error = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            agent_hostname: agent_hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            kernel_headers_source: kernel_headers_source,
            entrypoint_error: entrypoint_error,
            entrypoint_info: entrypoint_info,
        })
    }
}
// Parsed struct for collector_health_stats
pub struct collector_health_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub status: ::std::string::String,
    pub status_detail: ::std::string::String,
}

impl collector_health_stats {
    pub const RPC_ID: u16 = 641u16;

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
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[38usize..38usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 48usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[24usize..24usize + 2]);
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
        __b.copy_from_slice(&body[26usize..26usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[28usize..28usize + 2]);
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
        __b.copy_from_slice(&body[30usize..30usize + 2]);
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
        __b.copy_from_slice(&body[32usize..32usize + 2]);
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
        __b.copy_from_slice(&body[34usize..34usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[36usize..36usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[40usize..40usize + 2]);
        let __l_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let hostname = if __l_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_hostname]).into_owned()
        };
        __off += __l_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[42usize..42usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[44usize..44usize + 2]);
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
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[46usize..46usize + 2]);
        let __l_status = u16::from_ne_bytes(__b) as usize;
        if __off + __l_status > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let status = if __l_status == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_status]).into_owned()
        };
        __off += __l_status;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let status_detail = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            hostname: hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            status: status,
            status_detail: status_detail,
        })
    }
}
// Parsed struct for bpf_log_stats
pub struct bpf_log_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub shard: u16,
    pub version: ::std::string::String,
    pub cloud: ::std::string::String,
    pub env: ::std::string::String,
    pub role: ::std::string::String,
    pub az: ::std::string::String,
    pub node_id: ::std::string::String,
    pub kernel_version: ::std::string::String,
    pub client_type: u16,
    pub hostname: ::std::string::String,
    pub os: ::std::string::String,
    pub os_version: ::std::string::String,
    pub time_ns: u64,
    pub filename: ::std::string::String,
    pub line: ::std::string::String,
    pub code: ::std::string::String,
    pub arg0: ::std::string::String,
    pub arg1: ::std::string::String,
    pub arg2: ::std::string::String,
}

impl bpf_log_stats {
    pub const RPC_ID: u16 = 642u16;

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
        let _ref = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let shard = u16::from_ne_bytes(body[6usize..6usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let client_type = u16::from_ne_bytes(body[38usize..38usize + 2].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        let time_ns = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload
        // dynamic string; decode later from payload

        // Decode dynamic payload strings
        let mut __off = 56usize;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[4usize..4usize + 2]);
        let __l_module = u16::from_ne_bytes(__b) as usize;
        if __off + __l_module > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __l_module == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_module]).into_owned()
        };
        __off += __l_module;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[24usize..24usize + 2]);
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
        __b.copy_from_slice(&body[26usize..26usize + 2]);
        let __l_cloud = u16::from_ne_bytes(__b) as usize;
        if __off + __l_cloud > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let cloud = if __l_cloud == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_cloud]).into_owned()
        };
        __off += __l_cloud;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[28usize..28usize + 2]);
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
        __b.copy_from_slice(&body[30usize..30usize + 2]);
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
        __b.copy_from_slice(&body[32usize..32usize + 2]);
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
        __b.copy_from_slice(&body[34usize..34usize + 2]);
        let __l_node_id = u16::from_ne_bytes(__b) as usize;
        if __off + __l_node_id > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let node_id = if __l_node_id == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_node_id]).into_owned()
        };
        __off += __l_node_id;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[36usize..36usize + 2]);
        let __l_kernel_version = u16::from_ne_bytes(__b) as usize;
        if __off + __l_kernel_version > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let kernel_version = if __l_kernel_version == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_kernel_version])
                .into_owned()
        };
        __off += __l_kernel_version;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[40usize..40usize + 2]);
        let __l_hostname = u16::from_ne_bytes(__b) as usize;
        if __off + __l_hostname > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let hostname = if __l_hostname == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_hostname]).into_owned()
        };
        __off += __l_hostname;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[42usize..42usize + 2]);
        let __l_os = u16::from_ne_bytes(__b) as usize;
        if __off + __l_os > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let os = if __l_os == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_os]).into_owned()
        };
        __off += __l_os;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[44usize..44usize + 2]);
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
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[46usize..46usize + 2]);
        let __l_filename = u16::from_ne_bytes(__b) as usize;
        if __off + __l_filename > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let filename = if __l_filename == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_filename]).into_owned()
        };
        __off += __l_filename;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[48usize..48usize + 2]);
        let __l_line = u16::from_ne_bytes(__b) as usize;
        if __off + __l_line > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let line = if __l_line == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_line]).into_owned()
        };
        __off += __l_line;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[50usize..50usize + 2]);
        let __l_code = u16::from_ne_bytes(__b) as usize;
        if __off + __l_code > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let code = if __l_code == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_code]).into_owned()
        };
        __off += __l_code;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[52usize..52usize + 2]);
        let __l_arg0 = u16::from_ne_bytes(__b) as usize;
        if __off + __l_arg0 > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let arg0 = if __l_arg0 == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_arg0]).into_owned()
        };
        __off += __l_arg0;
        // length from header
        let mut __b = [0u8; 2];
        __b.copy_from_slice(&body[54usize..54usize + 2]);
        let __l_arg1 = u16::from_ne_bytes(__b) as usize;
        if __off + __l_arg1 > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let arg1 = if __l_arg1 == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __l_arg1]).into_owned()
        };
        __off += __l_arg1;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let arg2 = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            shard: shard,
            version: version,
            cloud: cloud,
            env: env,
            role: role,
            az: az,
            node_id: node_id,
            kernel_version: kernel_version,
            client_type: client_type,
            hostname: hostname,
            os: os,
            os_version: os_version,
            time_ns: time_ns,
            filename: filename,
            line: line,
            code: code,
            arg0: arg0,
            arg1: arg1,
            arg2: arg2,
        })
    }
}
// Parsed struct for server_stats
pub struct server_stats {
    pub _rpc_id: u16,
    pub _ref: u64,
    pub module: ::std::string::String,
    pub connection_counter: u64,
    pub disconnect_counter: u64,
    pub time_ns: u64,
}

impl server_stats {
    pub const RPC_ID: u16 = 643u16;

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
        let _ref = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let connection_counter = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let disconnect_counter = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let time_ns = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());

        // Decode dynamic payload strings
        let mut __off = 40usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let module = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            _ref: _ref,
            module: module,
            connection_counter: connection_counter,
            disconnect_counter: disconnect_counter,
            time_ns: time_ns,
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
