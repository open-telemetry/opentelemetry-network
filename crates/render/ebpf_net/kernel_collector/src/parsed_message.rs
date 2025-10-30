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
