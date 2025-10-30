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

// Parsed struct for dns_packet
pub struct dns_packet {
    pub _rpc_id: u16,
    pub sk: u64,
    pub pkt: ::std::string::String,
    pub total_len: u16,
    pub is_rx: u8,
}

impl dns_packet {
    pub const RPC_ID: u16 = 331u16;

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
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        // dynamic string; decode later from payload
        let total_len = u16::from_ne_bytes(body[4usize..4usize + 2].try_into().unwrap());
        let is_rx = body[6usize];

        // Decode dynamic payload strings
        let mut __off = 16usize;
        let __tail = (__len as usize).saturating_sub(__off);
        if __off + __tail > body.len() {
            return Err(DecodeError::BufferTooSmall);
        }
        let pkt = if __tail == 0 {
            ::std::string::String::new()
        } else {
            ::std::string::String::from_utf8_lossy(&body[__off..__off + __tail]).into_owned()
        };

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            pkt: pkt,
            total_len: total_len,
            is_rx: is_rx,
        })
    }
}
// Parsed struct for reset_tcp_counters
pub struct reset_tcp_counters {
    pub _rpc_id: u16,
    pub sk: u64,
    pub bytes_acked: u64,
    pub packets_delivered: u32,
    pub packets_retrans: u32,
    pub bytes_received: u64,
    pub pid: u32,
}

impl reset_tcp_counters {
    pub const RPC_ID: u16 = 332u16;

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
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let bytes_acked = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let packets_delivered = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let packets_retrans = u32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());
        let bytes_received = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let pid = u32::from_ne_bytes(body[36usize..36usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            bytes_acked: bytes_acked,
            packets_delivered: packets_delivered,
            packets_retrans: packets_retrans,
            bytes_received: bytes_received,
            pid: pid,
        })
    }
}
// Parsed struct for new_sock_created
pub struct new_sock_created {
    pub _rpc_id: u16,
    pub pid: u32,
    pub sk: u64,
}

impl new_sock_created {
    pub const RPC_ID: u16 = 333u16;

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
// Parsed struct for udp_new_socket
pub struct udp_new_socket {
    pub _rpc_id: u16,
    pub pid: u32,
    pub sk: u64,
    pub laddr: [u8; 16],
    pub lport: u16,
}

impl udp_new_socket {
    pub const RPC_ID: u16 = 334u16;

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
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let laddr = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 16usize + i * es;
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
            sk: sk,
            laddr: laddr,
            lport: lport,
        })
    }
}
// Parsed struct for udp_destroy_socket
pub struct udp_destroy_socket {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl udp_destroy_socket {
    pub const RPC_ID: u16 = 335u16;

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
// Parsed struct for udp_stats
pub struct udp_stats {
    pub _rpc_id: u16,
    pub sk: u64,
    pub raddr: [u8; 16],
    pub packets: u32,
    pub bytes: u32,
    pub changed_af: u8,
    pub rport: u16,
    pub is_rx: u8,
    pub laddr: [u8; 16],
    pub lport: u16,
    pub drops: u32,
}

impl udp_stats {
    pub const RPC_ID: u16 = 336u16;

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
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let raddr = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 26usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let packets = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let bytes = u32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        let changed_af = body[42usize];
        let rport = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let is_rx = body[43usize];
        let laddr = {
            let mut tmp = [0u8; 16];
            let es = 1usize;
            let mut i = 0usize;
            while i < 16usize {
                let off = 44usize + i * es;
                tmp[i] = body[off];
                i += 1;
            }
            tmp
        };
        let lport = u16::from_ne_bytes(body[24usize..24usize + 2].try_into().unwrap());
        let drops = u32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            raddr: raddr,
            packets: packets,
            bytes: bytes,
            changed_af: changed_af,
            rport: rport,
            is_rx: is_rx,
            laddr: laddr,
            lport: lport,
            drops: drops,
        })
    }
}
// Parsed struct for pid_info
pub struct pid_info {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
    pub cgroup: u64,
    pub parent_pid: i32,
}

impl pid_info {
    pub const RPC_ID: u16 = 337u16;

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
        let parent_pid = i32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            pid: pid,
            comm: comm,
            cgroup: cgroup,
            parent_pid: parent_pid,
        })
    }
}
// Parsed struct for pid_close
pub struct pid_close {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
}

impl pid_close {
    pub const RPC_ID: u16 = 338u16;

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
// Parsed struct for pid_set_comm
pub struct pid_set_comm {
    pub _rpc_id: u16,
    pub pid: u32,
    pub comm: [u8; 16],
}

impl pid_set_comm {
    pub const RPC_ID: u16 = 371u16;

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
    pub const RPC_ID: u16 = 339u16;

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
    pub const RPC_ID: u16 = 340u16;

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
// Parsed struct for rtt_estimator
pub struct rtt_estimator {
    pub _rpc_id: u16,
    pub srtt: u32,
    pub snd_cwnd: u32,
    pub bytes_acked: u64,
    pub ca_state: u8,
    pub sk: u64,
    pub packets_in_flight: u32,
    pub packets_delivered: u32,
    pub packets_retrans: u32,
    pub rcv_holes: u32,
    pub bytes_received: u64,
    pub rcv_delivered: u32,
    pub rcv_rtt: u32,
}

impl rtt_estimator {
    pub const RPC_ID: u16 = 361u16;

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
        let srtt = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let snd_cwnd = u32::from_ne_bytes(body[32usize..32usize + 4].try_into().unwrap());
        let bytes_acked = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let ca_state = body[2usize];
        let sk = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let packets_in_flight = u32::from_ne_bytes(body[36usize..36usize + 4].try_into().unwrap());
        let packets_delivered = u32::from_ne_bytes(body[40usize..40usize + 4].try_into().unwrap());
        let packets_retrans = u32::from_ne_bytes(body[44usize..44usize + 4].try_into().unwrap());
        let rcv_holes = u32::from_ne_bytes(body[48usize..48usize + 4].try_into().unwrap());
        let bytes_received = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let rcv_delivered = u32::from_ne_bytes(body[52usize..52usize + 4].try_into().unwrap());
        let rcv_rtt = u32::from_ne_bytes(body[56usize..56usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            srtt: srtt,
            snd_cwnd: snd_cwnd,
            bytes_acked: bytes_acked,
            ca_state: ca_state,
            sk: sk,
            packets_in_flight: packets_in_flight,
            packets_delivered: packets_delivered,
            packets_retrans: packets_retrans,
            rcv_holes: rcv_holes,
            bytes_received: bytes_received,
            rcv_delivered: rcv_delivered,
            rcv_rtt: rcv_rtt,
        })
    }
}
// Parsed struct for close_sock_info
pub struct close_sock_info {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl close_sock_info {
    pub const RPC_ID: u16 = 362u16;

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
// Parsed struct for kill_css
pub struct kill_css {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub cgroup_parent: u64,
    pub name: [u8; 256],
}

impl kill_css {
    pub const RPC_ID: u16 = 363u16;

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
// Parsed struct for css_populate_dir
pub struct css_populate_dir {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub cgroup_parent: u64,
    pub name: [u8; 256],
}

impl css_populate_dir {
    pub const RPC_ID: u16 = 364u16;

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
// Parsed struct for existing_cgroup_probe
pub struct existing_cgroup_probe {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub cgroup_parent: u64,
    pub name: [u8; 256],
}

impl existing_cgroup_probe {
    pub const RPC_ID: u16 = 365u16;

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
// Parsed struct for cgroup_attach_task
pub struct cgroup_attach_task {
    pub _rpc_id: u16,
    pub cgroup: u64,
    pub pid: u32,
    pub comm: [u8; 16],
}

impl cgroup_attach_task {
    pub const RPC_ID: u16 = 366u16;

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
        let cgroup = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
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
            cgroup: cgroup,
            pid: pid,
            comm: comm,
        })
    }
}
// Parsed struct for nf_conntrack_alter_reply
pub struct nf_conntrack_alter_reply {
    pub _rpc_id: u16,
    pub ct: u64,
    pub src_ip: u32,
    pub src_port: u16,
    pub dst_ip: u32,
    pub dst_port: u16,
    pub proto: u8,
    pub nat_src_ip: u32,
    pub nat_src_port: u16,
    pub nat_dst_ip: u32,
    pub nat_dst_port: u16,
    pub nat_proto: u8,
}

impl nf_conntrack_alter_reply {
    pub const RPC_ID: u16 = 367u16;

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
        let ct = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let src_ip = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let src_port = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let dst_ip = u32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        let dst_port = u16::from_ne_bytes(body[28usize..28usize + 2].try_into().unwrap());
        let proto = body[34usize];
        let nat_src_ip = u32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());
        let nat_src_port = u16::from_ne_bytes(body[30usize..30usize + 2].try_into().unwrap());
        let nat_dst_ip = u32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let nat_dst_port = u16::from_ne_bytes(body[32usize..32usize + 2].try_into().unwrap());
        let nat_proto = body[35usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            ct: ct,
            src_ip: src_ip,
            src_port: src_port,
            dst_ip: dst_ip,
            dst_port: dst_port,
            proto: proto,
            nat_src_ip: nat_src_ip,
            nat_src_port: nat_src_port,
            nat_dst_ip: nat_dst_ip,
            nat_dst_port: nat_dst_port,
            nat_proto: nat_proto,
        })
    }
}
// Parsed struct for nf_nat_cleanup_conntrack
pub struct nf_nat_cleanup_conntrack {
    pub _rpc_id: u16,
    pub ct: u64,
    pub src_ip: u32,
    pub src_port: u16,
    pub dst_ip: u32,
    pub dst_port: u16,
    pub proto: u8,
}

impl nf_nat_cleanup_conntrack {
    pub const RPC_ID: u16 = 368u16;

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

        if body.len() < 23usize {
            return Err(DecodeError::BufferTooSmall);
        }

        // Decode fixed header fields
        let ct = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let src_ip = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let src_port = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let dst_ip = u32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        let dst_port = u16::from_ne_bytes(body[20usize..20usize + 2].try_into().unwrap());
        let proto = body[22usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            ct: ct,
            src_ip: src_ip,
            src_port: src_port,
            dst_ip: dst_ip,
            dst_port: dst_port,
            proto: proto,
        })
    }
}
// Parsed struct for existing_conntrack_tuple
pub struct existing_conntrack_tuple {
    pub _rpc_id: u16,
    pub ct: u64,
    pub src_ip: u32,
    pub src_port: u16,
    pub dst_ip: u32,
    pub dst_port: u16,
    pub proto: u8,
    pub dir: u8,
}

impl existing_conntrack_tuple {
    pub const RPC_ID: u16 = 369u16;

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
        let ct = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let src_ip = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let src_port = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let dst_ip = u32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());
        let dst_port = u16::from_ne_bytes(body[20usize..20usize + 2].try_into().unwrap());
        let proto = body[22usize];
        let dir = body[23usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            ct: ct,
            src_ip: src_ip,
            src_port: src_port,
            dst_ip: dst_ip,
            dst_port: dst_port,
            proto: proto,
            dir: dir,
        })
    }
}
// Parsed struct for tcp_syn_timeout
pub struct tcp_syn_timeout {
    pub _rpc_id: u16,
    pub sk: u64,
}

impl tcp_syn_timeout {
    pub const RPC_ID: u16 = 370u16;

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
    pub const RPC_ID: u16 = 372u16;

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
// Parsed struct for bpf_log
pub struct bpf_log {
    pub _rpc_id: u16,
    pub filelineid: u64,
    pub code: u64,
    pub arg0: u64,
    pub arg1: u64,
    pub arg2: u64,
}

impl bpf_log {
    pub const RPC_ID: u16 = 373u16;

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
        let filelineid = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let code = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let arg0 = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let arg1 = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());
        let arg2 = u64::from_ne_bytes(body[40usize..40usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            filelineid: filelineid,
            code: code,
            arg0: arg0,
            arg1: arg1,
            arg2: arg2,
        })
    }
}
// Parsed struct for stack_trace
pub struct stack_trace {
    pub _rpc_id: u16,
    pub kernel_stack_id: i32,
    pub user_stack_id: i32,
    pub tgid: u32,
    pub comm: [u8; 16],
}

impl stack_trace {
    pub const RPC_ID: u16 = 374u16;

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
        let kernel_stack_id = i32::from_ne_bytes(body[20usize..20usize + 4].try_into().unwrap());
        let user_stack_id = i32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let tgid = u32::from_ne_bytes(body[28usize..28usize + 4].try_into().unwrap());
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
            kernel_stack_id: kernel_stack_id,
            user_stack_id: user_stack_id,
            tgid: tgid,
            comm: comm,
        })
    }
}
// Parsed struct for tcp_data
pub struct tcp_data {
    pub _rpc_id: u16,
    pub sk: u64,
    pub pid: u32,
    pub length: u32,
    pub offset: u64,
    pub stream_type: u8,
    pub client_server: u8,
}

impl tcp_data {
    pub const RPC_ID: u16 = 375u16;

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
        let sk = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let length = u32::from_ne_bytes(body[24usize..24usize + 4].try_into().unwrap());
        let offset = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let stream_type = body[2usize];
        let client_server = body[3usize];

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            sk: sk,
            pid: pid,
            length: length,
            offset: offset,
            stream_type: stream_type,
            client_server: client_server,
        })
    }
}
// Parsed struct for pid_exit
pub struct pid_exit {
    pub _rpc_id: u16,
    pub tgid: u64,
    pub pid: u32,
    pub exit_code: i32,
}

impl pid_exit {
    pub const RPC_ID: u16 = 377u16;

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
        let tgid = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let pid = u32::from_ne_bytes(body[4usize..4usize + 4].try_into().unwrap());
        let exit_code = i32::from_ne_bytes(body[16usize..16usize + 4].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            tgid: tgid,
            pid: pid,
            exit_code: exit_code,
        })
    }
}
// Parsed struct for report_debug_event
pub struct report_debug_event {
    pub _rpc_id: u16,
    pub event: u16,
    pub arg1: u64,
    pub arg2: u64,
    pub arg3: u64,
    pub arg4: u64,
}

impl report_debug_event {
    pub const RPC_ID: u16 = 378u16;

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
        let event = u16::from_ne_bytes(body[2usize..2usize + 2].try_into().unwrap());
        let arg1 = u64::from_ne_bytes(body[8usize..8usize + 8].try_into().unwrap());
        let arg2 = u64::from_ne_bytes(body[16usize..16usize + 8].try_into().unwrap());
        let arg3 = u64::from_ne_bytes(body[24usize..24usize + 8].try_into().unwrap());
        let arg4 = u64::from_ne_bytes(body[32usize..32usize + 8].try_into().unwrap());

        // Decode dynamic payload strings

        Ok(Self {
            _rpc_id: rpc,
            event: event,
            arg1: arg1,
            arg2: arg2,
            arg3: arg3,
            arg4: arg4,
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
    pub const RPC_ID: u16 = 379u16;

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
