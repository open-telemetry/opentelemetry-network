#![forbid(unsafe_op_in_unsafe_fn)]

/// Linux-style negative error codes preserved for parity with C implementation.
use crate::layout::ElementQueueSharedOps;
pub mod errno {
    pub const EINVAL: i32 = -22;
    pub const ENOSPC: i32 = -28;
    pub const ENOENT: i32 = -2;
    pub const EAGAIN: i32 = -11;
    pub const ENOSYS: i32 = -38;
}

pub mod layout;
pub mod raw;

// Re-export for backwards-compatibility with the previous single-module layout.
pub use layout::{contig_size, ElementQueueShared};
pub use raw::{ElementQueue, EqError, ReadBatch, WriteBatch};

/// Owned contiguous storage for an element queue, similar to MemElementQueueStorage.
pub struct MemElementQueueStorage {
    buf: Vec<u64>,
    n_elems: u32,
    buf_len: u32,
}

impl MemElementQueueStorage {
    pub fn new(n_elems: u32, buf_len: u32) -> Self {
        let size = contig_size(n_elems, buf_len);
        // Allocate with u64 alignment and round up size to u64 words
        let words = (size + 7) / 8;
        let mut buf = vec![0u64; words];
        // Initialize shared header
        let ptr = buf.as_mut_ptr() as *mut u8;
        (ptr as *mut ElementQueueShared).init_zero();
        Self {
            buf,
            n_elems,
            buf_len,
        }
    }

    pub fn n_elems(&self) -> u32 {
        self.n_elems
    }
    pub fn buf_len(&self) -> u32 {
        self.buf_len
    }
    pub fn data_ptr(&self) -> *mut u8 {
        self.buf.as_ptr() as *mut u8
    }

    pub fn make_queue(&self) -> Result<ElementQueue, EqError> {
        // Safety: `self.buf` is contiguous and has the expected layout
        unsafe { ElementQueue::new_from_contiguous(self.n_elems, self.buf_len, self.data_ptr()) }
    }

    /// Construct a queue over an external contiguous buffer.
    /// Safety: `data` must have the expected layout and size.
    pub unsafe fn queue_from_ptr(
        data: *mut u8,
        n_elems: u32,
        buf_len: u32,
    ) -> Result<ElementQueue, EqError> {
        unsafe { ElementQueue::new_from_contiguous(n_elems, buf_len, data) }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn layout_size() {
        let n_elems = 8u32;
        let buf_len = 64u32;
        let size = contig_size(n_elems, buf_len);
        assert_eq!(
            size,
            core::mem::size_of::<ElementQueueShared>() + (n_elems as usize) * 4 + buf_len as usize
        );
    }

    #[test]
    fn write_read_basic() {
        let storage = MemElementQueueStorage::new(8, 128);
        let mut q = storage.make_queue().unwrap();

        // Writer via guard
        let mut wb = q.start_write();
        wb.write(5).unwrap().copy_from_slice(b"hello");
        wb.write(8).unwrap().copy_from_slice(b"world!!!");
        let _ = wb.finish();

        // Reader via guard
        let rb = q.start_read();
        // peek first
        assert_eq!(rb.peek_len().unwrap(), 5);
        let v1 = rb.read().unwrap();
        assert_eq!(v1, b"hello");
        // second
        let v2 = rb.read().unwrap();
        assert_eq!(v2, b"world!!!");
        let _ = rb.finish();
    }

    #[test]
    fn wrap_around_alignment() {
        let storage = MemElementQueueStorage::new(8, 64);
        let mut q = storage.make_queue().unwrap();

        // Fill close to end to force wrap
        let mut wb = q.start_write();
        wb.write(30).unwrap().copy_from_slice(&vec![0u8; 30]);
        wb.write(5).unwrap().copy_from_slice(&vec![1u8; 5]); // 5 -> aligned to 8
        let _ = wb.finish();

        let rb = q.start_read();
        let _ = rb.read().unwrap();
        let second = rb.read().unwrap();
        assert_eq!(second.len(), 5);
        assert!(second.iter().all(|&b| b == 1));
        let _ = rb.finish();
    }

    #[test]
    fn peek_value_u64() {
        let storage = MemElementQueueStorage::new(8, 128);
        let mut q = storage.make_queue().unwrap();

        let ts: u64 = 0x1122334455667788;
        let mut wb = q.start_write();
        // write timestamp as 8 bytes
        wb.write(8).unwrap().copy_from_slice(&ts.to_le_bytes());
        let _ = wb.finish();

        let rb = q.start_read();
        let v: u64 = rb.peek_value::<u64>().unwrap();
        assert_eq!(v, ts.to_le());
        let got = rb.read().unwrap();
        assert_eq!(got.len(), 8);
        let _ = rb.finish();
    }
}
