//! Message metadata and size descriptors.

/// Size descriptor for a message.
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Size {
    /// Fixed-size wire message (size in bytes, after the timestamp).
    Fixed(usize),
    /// Dynamic-size wire message; read `_len: u16` from the header.
    Dynamic,
}

/// Metadata describing a message.
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub struct MessageMetadata {
    pub(crate) rpc_id: u16,
    pub(crate) size: Size,
}

impl MessageMetadata {
    /// Creates a metadata entry for a fixed-size message.
    /// The third parameter is deprecated and ignored.
    pub const fn new_fixed(rpc_id: u16, size: usize, _needs_auth: bool) -> Self {
        Self {
            rpc_id,
            size: Size::Fixed(size),
        }
    }

    /// Creates a metadata entry for a dynamic-size message.
    /// The second parameter is deprecated and ignored.
    pub const fn new_dynamic(rpc_id: u16, _needs_auth: bool) -> Self {
        Self {
            rpc_id,
            size: Size::Dynamic,
        }
    }

    /// Returns the RPC ID for this message.
    #[inline]
    pub const fn rpc_id(&self) -> u16 {
        self.rpc_id
    }

    /// Returns the size descriptor.
    #[inline]
    pub const fn size(&self) -> Size {
        self.size
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// new_fixed sets rpc_id and Size::Fixed(size)
    ///
    /// Verifies that constructor stores the provided values and that getters
    /// return them unchanged.
    #[test]
    fn new_fixed_sets_fields() {
        let md = MessageMetadata::new_fixed(10, 6, false);
        assert_eq!(md.rpc_id(), 10);
        assert_eq!(md.size(), Size::Fixed(6));
    }

    /// new_dynamic sets rpc_id and Size::Dynamic
    ///
    /// Verifies that constructor stores the provided rpc_id and marks the
    /// entry as dynamic.
    #[test]
    fn new_dynamic_sets_fields() {
        let md = MessageMetadata::new_dynamic(11, false);
        assert_eq!(md.rpc_id(), 11);
        assert_eq!(md.size(), Size::Dynamic);
    }

    /// metadata equality and Debug implementations
    ///
    /// Asserts that deriving PartialEq works and that formatting with Debug
    /// does not panic (smoke test via format!).
    #[test]
    fn metadata_equality_debug() {
        let a = MessageMetadata::new_fixed(1, 2, false);
        let b = MessageMetadata::new_fixed(1, 2, false);
        let c = MessageMetadata::new_dynamic(1, false);
        assert_eq!(a, b);
        assert_ne!(a, c);
        let _ = format!("{:?}", a);
    }
}
