//! Generic render parser for incoming messages.
//!
//! Overview
//! - Maintains a perfect-hash map from `_rpc_id` to user-provided value `V`
//!   together with message metadata (`MessageMetadata`).
//! - Parses native-endian timestamp (`u64`) + message body as emitted by the
//!   generated encoders.
//! - Determines message length using `MessageMetadata::size()` for fixed-size
//!   messages or the `_len: u16` header for dynamic-size messages.
//!
//! Message format (as produced by generated encoders)
//! - `timestamp: u64` (native-endian)
//! - `message: [u8]` starting at byte 8, which always begins with:
//!   - `_rpc_id: u16`
//!   - for dynamic-size messages only: `_len: u16` describing the total
//!     message length in bytes after the timestamp (i.e., length of the
//!     returned `message` slice). The parser enforces `_len >= 4` so the
//!     returned `message` slice always contains `_rpc_id` and `_len`.
//!
//! Errors and panics
//! - `Error::BufferTooSmall`: the provided buffer does not contain enough
//!   bytes to complete parsing for the selected branch.
//! - `Error::MessageNotRegistered`: there is no registered metadata for the
//!   parsed `_rpc_id`.
//! - `Error::InvalidLength { .. }`: for dynamic messages when `_len < 4`.
//! - Panics can occur when using a broken perfect hash function that maps a
//!   key outside `[0, hash_size)`, e.g. in `Parser::add_message` through the
//!   underlying `PerfectHashMap`.
//!
//! Invariants
//! - On success, `HandleOk::message` is a zero-copy slice into the original
//!   buffer covering exactly the message body starting at byte 8.
//! - For dynamic messages, `HandleOk::message.len() == _len` and `_len >= 4`.
//! - For fixed messages, `HandleOk::message.len()` equals the registered fixed
//!   size. The parser does not enforce a minimum fixed size, but consumers
//!   typically encode `_rpc_id` at the start of the body.

#![deny(missing_docs)]

mod message;
pub use message::{MessageMetadata, Size};

use perfect_hash_map::{Key, PerfectHashMap};

/// Error type for `Parser::handle`.
#[derive(thiserror::Error, Debug, PartialEq, Eq)]
pub enum Error {
    /// Not enough data in the buffer to make progress (like `-EAGAIN`).
    #[error("buffer too small")]
    BufferTooSmall,
    /// Handler was not registered for the RPC ID (like `-ENOENT`).
    #[error("message not registered: rpc_id={0}")]
    MessageNotRegistered(u16),
    /// Dynamic message declares an invalid `_len` smaller than header size.
    #[error("invalid dynamic length: rpc_id={rpc_id} len={len}")]
    InvalidLength {
        /// The parsed RPC ID.
        rpc_id: u16,
        /// The `_len` value.
        len: u16,
    },
}

/// Internal entry mapping rpc_id to metadata and user value.
pub struct Entry<V> {
    /// The message metadata.
    pub metadata: MessageMetadata,
    /// The user-supplied value associated with this message.
    pub value: V,
}

/// Successful result from `Parser::handle`.
#[derive(Debug)]
pub struct HandleOk<'a, V> {
    /// Slice to the full message body (excluding the 8-byte timestamp).
    pub message: &'a [u8],
    /// Reference to the user value associated with the message type.
    pub value: &'a V,
    /// Timestamp extracted from the message (native-endian u64).
    pub timestamp: u64,
}

/// Parser that maintains message metadata and values keyed by RPC ID.
pub struct Parser<V, F = perfect_hash_map::HashFn>
where
    F: Fn(Key) -> u32 + Copy,
{
    map: PerfectHashMap<Entry<V>, F>,
}

impl<V, F> Parser<V, F>
where
    F: Fn(Key) -> u32 + Copy,
{
    /// Creates a new parser with the given perfect-hash size and function.
    ///
    /// Panics
    /// - If subsequent calls into the underlying map use a hash function that
    ///   returns an out-of-range slot (>= `hash_size`).
    pub fn new(hash_size: usize, hash: F) -> Self {
        Self {
            map: PerfectHashMap::new(hash_size, hash),
        }
    }

    /// Adds or replaces a message entry for the given metadata and value.
    ///
    /// Returns
    /// - `Ok(None)` if the key was not present.
    /// - `Ok(Some(old))` if the same key was present and the value was replaced.
    /// - `Err(CollisionError)` if the slot is occupied by a different key.
    ///
    /// Panics
    /// - If the hash function returns an out-of-range slot (>= capacity()).
    pub fn add_message(
        &mut self,
        metadata: MessageMetadata,
        value: V,
    ) -> Result<Option<V>, perfect_hash_map::CollisionError> {
        let key = metadata.rpc_id as u32;
        self.map
            .insert(key, Entry { metadata, value })
            .map(|opt| opt.map(|e| e.value))
    }

    /// Handles a single message from `data`.
    ///
    /// Expected format (as produced by the generated encoders):
    /// - 8-byte native-endian timestamp
    /// - message body starting with `_rpc_id: u16`
    /// - for dynamic messages, `_len: u16` giving the total message length
    ///   (in bytes) after the timestamp. Must be at least 4.
    ///
    /// Errors
    /// - `BufferTooSmall` if the buffer does not contain enough bytes for the
    ///   relevant branch.
    /// - `MessageNotRegistered` if the parsed `_rpc_id` is unknown.
    /// - `InvalidLength { .. }` if a dynamic message declares `_len < 4`.
    pub fn handle<'a>(&'a self, data: &'a [u8]) -> Result<HandleOk<'a, V>, Error> {
        // Need at least timestamp + rpc_id
        if data.len() < 8 + 2 {
            return Err(Error::BufferTooSmall);
        }

        let timestamp = {
            let mut buf = [0u8; 8];
            buf.copy_from_slice(&data[..8]);
            u64::from_ne_bytes(buf)
        };

        let rpc_id = {
            let mut b = [0u8; 2];
            b.copy_from_slice(&data[8..10]);
            u16::from_ne_bytes(b)
        };

        let key = rpc_id as u32;
        let entry = match self.map.get(&key) {
            Some(e) => e,
            None => return Err(Error::MessageNotRegistered(rpc_id)),
        };

        let msg_slice = match entry.metadata.size() {
            Size::Fixed(n) => {
                // Require full fixed-size body
                let need = 8usize + n;
                if data.len() < need {
                    return Err(Error::BufferTooSmall);
                }
                &data[8..need]
            }
            Size::Dynamic => {
                // Need timestamp + rpc_id + _len
                if data.len() < 8 + 4 {
                    return Err(Error::BufferTooSmall);
                }
                let mut b = [0u8; 2];
                b.copy_from_slice(&data[10..12]);
                let len_field = u16::from_ne_bytes(b);
                if len_field < 4 {
                    return Err(Error::InvalidLength {
                        rpc_id,
                        len: len_field,
                    });
                }
                let total_len_after_ts = len_field as usize;
                let need = 8usize + total_len_after_ts;
                if data.len() < need {
                    return Err(Error::BufferTooSmall);
                }
                &data[8..need]
            }
        };

        Ok(HandleOk {
            message: msg_slice,
            value: &entry.value,
            timestamp,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn hash_mod8(k: Key) -> u32 {
        (k % 8) as u32
    }

    /// Fixed-size happy path: exact-size buffer, zero-copy slice, fields.
    #[test]
    fn fixed_ok() {
        let mut p: Parser<&'static str, _> = Parser::new(8, hash_mod8);
        let md = MessageMetadata::new_fixed(1, 6, false);
        p.add_message(md, "ok").unwrap();

        // Build a buffer: ts(8) + rpc_id(2) + 6 bytes body
        let mut buf = Vec::new();
        buf.extend_from_slice(&123_u64.to_ne_bytes());
        buf.extend_from_slice(&1_u16.to_ne_bytes());
        buf.extend_from_slice(&[0u8; 6]);

        let out = p.handle(&buf).unwrap();
        assert_eq!(out.timestamp, 123);
        assert_eq!(out.value, &"ok");
        assert_eq!(out.message.len(), 6);
    }

    /// Dynamic-size happy path: `_len` respected, zero-copy slice, fields.
    #[test]
    fn dynamic_ok() {
        let mut p: Parser<&'static str, _> = Parser::new(8, hash_mod8);
        let md = MessageMetadata::new_dynamic(2, false);
        p.add_message(md, "dyn").unwrap();

        // Build: ts(8) + rpc_id(2) + _len(2) + body(_len bytes)
        let body_len: u16 = 10;
        let mut buf = Vec::new();
        buf.extend_from_slice(&111_u64.to_ne_bytes());
        buf.extend_from_slice(&2_u16.to_ne_bytes());
        buf.extend_from_slice(&body_len.to_ne_bytes());
        buf.extend_from_slice(&vec![0u8; body_len as usize]);

        let out = p.handle(&buf).unwrap();
        assert_eq!(out.timestamp, 111);
        assert_eq!(out.value, &"dyn");
        assert_eq!(out.message.len(), body_len as usize);
    }

    /// Dynamic messages must declare `_len >= 4`; `_len < 4` is invalid.
    #[test]
    fn dynamic_invalid_len_too_small() {
        let mut p: Parser<&'static str, _> = Parser::new(8, hash_mod8);
        let md = MessageMetadata::new_dynamic(9, false);
        p.add_message(md, "dyn").unwrap();

        // ts(8) + rpc_id(2) + _len(2=2) (no payload)
        let mut buf = Vec::new();
        buf.extend_from_slice(&42_u64.to_ne_bytes());
        buf.extend_from_slice(&9_u16.to_ne_bytes());
        buf.extend_from_slice(&2u16.to_ne_bytes());

        let err = p.handle(&buf).unwrap_err();
        assert_eq!(err, Error::InvalidLength { rpc_id: 9, len: 2 });
    }

    /// Missing `_len` field for dynamic messages yields BufferTooSmall.
    #[test]
    fn dynamic_missing_len_field() {
        let mut p: Parser<&'static str, _> = Parser::new(8, hash_mod8);
        let md = MessageMetadata::new_dynamic(10, false);
        p.add_message(md, "dyn").unwrap();

        // Only ts + rpc_id present
        let mut buf = Vec::new();
        buf.extend_from_slice(&7_u64.to_ne_bytes());
        buf.extend_from_slice(&10_u16.to_ne_bytes());

        assert_eq!(p.handle(&buf).unwrap_err(), Error::BufferTooSmall);
    }

    /// If `_len` claims more bytes than provided, BufferTooSmall is returned.
    #[test]
    fn dynamic_len_exceeds_buffer() {
        let mut p: Parser<&'static str, _> = Parser::new(8, hash_mod8);
        let md = MessageMetadata::new_dynamic(11, false);
        p.add_message(md, "dyn").unwrap();

        // ts + rpc_id + _len=10 but only 5 bytes of payload provided
        let mut buf = Vec::new();
        buf.extend_from_slice(&1_u64.to_ne_bytes());
        buf.extend_from_slice(&11_u16.to_ne_bytes());
        buf.extend_from_slice(&10u16.to_ne_bytes());
        buf.extend_from_slice(&vec![0u8; 5]);

        assert_eq!(p.handle(&buf).unwrap_err(), Error::BufferTooSmall);
    }

    /// Trailing bytes after the declared dynamic length are ignored.
    #[test]
    fn dynamic_trailing_bytes_ignored() {
        let mut p: Parser<&'static str, _> = Parser::new(8, hash_mod8);
        let md = MessageMetadata::new_dynamic(12, false);
        p.add_message(md, "dyn").unwrap();

        // _len = 6, but provide extra trailing bytes beyond that
        let mut buf = Vec::new();
        buf.extend_from_slice(&99_u64.to_ne_bytes());
        buf.extend_from_slice(&12_u16.to_ne_bytes());
        buf.extend_from_slice(&6u16.to_ne_bytes());
        buf.extend_from_slice(&vec![0u8; 6]);
        buf.extend_from_slice(&[1, 2, 3, 4, 5, 6]);

        let out = p.handle(&buf).unwrap();
        assert_eq!(out.message.len(), 6);
        // First two bytes of the message are rpc_id in native-endian
        assert_eq!(&out.message[0..2], &12_u16.to_ne_bytes());
    }

    /// Fixed-size buffer too small: returns BufferTooSmall.
    #[test]
    fn need_more() {
        let mut p: Parser<(), _> = Parser::new(8, hash_mod8);
        let md = MessageMetadata::new_fixed(3, 2, false);
        p.add_message(md, ()).unwrap();
        let b = [0u8; 9]; // not enough for ts + body
        assert_eq!(p.handle(&b).unwrap_err(), Error::BufferTooSmall);
    }

    /// Unregistered rpc_id: returns MessageNotRegistered.
    #[test]
    fn not_registered() {
        let p: Parser<(), _> = Parser::new(8, hash_mod8);
        // ts + rpc_id=7 + len
        let mut buf = vec![0; 12];
        buf[..8].copy_from_slice(&0_u64.to_ne_bytes());
        buf[8..10].copy_from_slice(&7_u16.to_ne_bytes());
        assert_eq!(p.handle(&buf).unwrap_err(), Error::MessageNotRegistered(7));
    }

    /// Replacing an existing rpc_id updates metadata and returns the old value.
    #[test]
    fn replace_same_rpc_returns_old_and_updates_metadata() {
        fn hash_mod2(k: Key) -> u32 {
            (k % 2) as u32
        }
        let mut p: Parser<&'static str, _> = Parser::new(2, hash_mod2);

        // Insert fixed with rpc_id=5
        p.add_message(MessageMetadata::new_fixed(5, 6, false), "old")
            .unwrap();

        // Replace with dynamic metadata and new value
        let prev = p
            .add_message(MessageMetadata::new_dynamic(5, false), "new")
            .unwrap();
        assert_eq!(prev, Some("old"));

        // Build a dynamic buffer for rpc_id=5 with len=4 (header only)
        let mut buf = Vec::new();
        buf.extend_from_slice(&0_u64.to_ne_bytes());
        buf.extend_from_slice(&5_u16.to_ne_bytes());
        buf.extend_from_slice(&4u16.to_ne_bytes());

        let out = p.handle(&buf).unwrap();
        assert_eq!(out.value, &"new");
        assert_eq!(out.message.len(), 4);
    }

    /// Collisions on different keys return an error and preserve existing data.
    #[test]
    fn collision_different_rpc_yields_error_and_preserves_existing() {
        fn hash_mod2(k: Key) -> u32 {
            (k % 2) as u32
        }
        let mut p: Parser<&'static str, _> = Parser::new(2, hash_mod2);

        p.add_message(MessageMetadata::new_fixed(1, 2, false), "one")
            .unwrap();
        let err = p
            .add_message(MessageMetadata::new_fixed(3, 2, false), "three")
            .unwrap_err();
        assert_eq!(err.existing_key, 1);

        // Ensure original is still usable
        let mut buf = Vec::new();
        buf.extend_from_slice(&0_u64.to_ne_bytes());
        buf.extend_from_slice(&1_u16.to_ne_bytes());
        buf.extend_from_slice(&[0u8; 2]);
        let out = p.handle(&buf).unwrap();
        assert_eq!(out.value, &"one");
    }

    /// A broken hash that returns out-of-range indices will panic on insert.
    #[test]
    #[should_panic]
    fn broken_hash_panics_on_insert() {
        fn bad_hash(_: Key) -> u32 {
            9999
        }
        let mut p: Parser<&'static str, _> = Parser::new(2, bad_hash);
        let _ = p.add_message(MessageMetadata::new_fixed(1, 2, false), "x");
    }
}
