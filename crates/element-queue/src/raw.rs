use core::cell::Cell;
use core::ptr::{self, NonNull};
use core::sync::atomic::{fence, Ordering};

use crate::errno;
use crate::layout::{ElementQueueShared, ElementQueueSharedOps};

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum EqError {
    InvalidArg,
    NoSpace,
    NoEntry,
    TryAgain,
    Unexpected,
}

impl EqError {
    pub fn code(self) -> i32 {
        use EqError::*;
        match self {
            InvalidArg => errno::EINVAL,
            NoSpace => errno::ENOSPC,
            NoEntry => errno::ENOENT,
            TryAgain => errno::EAGAIN,
            Unexpected => errno::ENOSYS,
        }
    }
}

/// Unsafe, low-level queue core. Mirrors the C implementation closely.
///
/// Safety: Methods that dereference raw pointers encapsulate the same
/// assumptions as the original C code. Public, higher-level wrappers should
/// be preferred when possible.
pub struct ElementQueue {
    // Masks (size - 1), must be power-of-two sizes.
    elem_mask: u32,
    buf_mask: u32,

    // Local cached indices, following batching protocol.
    elem_head: u32,
    buf_head: u32,
    elem_tail: u32,
    buf_tail: u32,

    // Raw pointers into contiguous storage.
    shared: *mut ElementQueueShared,
    elems: NonNull<[u32]>,
    data: NonNull<[u8]>,
}

impl core::fmt::Debug for ElementQueue {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        f.debug_struct("ElementQueue")
            .field("elem_mask", &self.elem_mask)
            .field("buf_mask", &self.buf_mask)
            .field("elem_head", &self.elem_head)
            .field("buf_head", &self.buf_head)
            .field("elem_tail", &self.elem_tail)
            .field("buf_tail", &self.buf_tail)
            .finish()
    }
}

impl ElementQueue {
    /// Create a queue over contiguous memory ([shared][elems][data]).
    ///
    /// Safety: `data` must be a valid, writable pointer to at least
    /// `contig_size(n_elems, buf_len)` bytes with the expected layout.
    pub unsafe fn new_from_contiguous(
        n_elems: u32,
        buf_len: u32,
        data: *mut u8,
    ) -> Result<Self, EqError> {
        if data.is_null() {
            return Err(EqError::InvalidArg);
        }
        if n_elems == 0 || n_elems & (n_elems - 1) != 0 {
            return Err(EqError::InvalidArg);
        }
        if buf_len == 0 || buf_len & (buf_len - 1) != 0 {
            return Err(EqError::InvalidArg);
        }

        // Layout: shared, then elem ring, then data buffer.
        let shared = data as *mut ElementQueueShared;
        // SAFETY: pointer arithmetic within the contiguous buffer layout
        let elems = unsafe { shared.add(1) } as *mut u32;
        let data_ptr = unsafe { elems.add(n_elems as usize) } as *mut u8;

        // Create raw slice pointers for elems and data (unsafe boundary here only).
        let elems_slice: *mut [u32] = ptr::slice_from_raw_parts_mut(elems, n_elems as usize);
        let data_slice: *mut [u8] = ptr::slice_from_raw_parts_mut(data_ptr, buf_len as usize);

        // Load current shared indices using volatile reads (per-field like C).
        let elem_head = shared.get_elem_head();
        let buf_head = shared.get_buf_head();
        let elem_tail = shared.get_elem_tail();
        let buf_tail = shared.get_buf_tail();

        Ok(ElementQueue {
            elem_mask: n_elems - 1,
            buf_mask: buf_len - 1,
            elem_head,
            buf_head,
            elem_tail,
            buf_tail,
            shared,
            elems: unsafe { NonNull::new_unchecked(elems_slice) },
            data: unsafe { NonNull::new_unchecked(data_slice) },
        })
    }
}

/// Write batch guard with discard-on-drop semantics.
///
/// - Holds local copies of tails which are advanced during the batch.
/// - `finish(self)` commits local tails to the queue and publishes to shared.
/// - Dropping without `finish()` discards local progress (no commit/publish).
pub struct WriteBatch<'q> {
    q: &'q mut ElementQueue,
    elem_tail: Cell<u32>,
    buf_tail: Cell<u32>,
}

impl ElementQueue {
    /// Start a write batch (producer) and return a guard.
    /// Loads consumer-published heads to check space.
    pub fn start_write<'q>(&'q mut self) -> WriteBatch<'q> {
        // Read the heads published by consumer (volatile per-field to mirror ACCESS_ONCE).
        self.elem_head = self.shared.get_elem_head();
        self.buf_head = self.shared.get_buf_head();

        debug_assert!((self.buf_tail as i64 - self.buf_head as i64) >= 0);
        debug_assert!((self.elem_tail as i64 - self.elem_head as i64) >= 0);
        WriteBatch {
            elem_tail: Cell::new(self.elem_tail),
            buf_tail: Cell::new(self.buf_tail),
            q: self,
        }
    }
}

impl<'q> WriteBatch<'q> {
    pub fn write<'a>(&'a mut self, len: u32) -> Result<&'a mut [u8], EqError> {
        let aligned_len = (len + 7) & !7;
        if aligned_len > self.q.buf_mask {
            return Err(EqError::InvalidArg);
        }
        // Element ring full?
        if self.elem_tail.get().wrapping_sub(self.q.elem_head) >= (self.q.elem_mask + 1) {
            return Err(EqError::NoSpace);
        }

        let buf_mask = self.q.buf_mask;
        let buf_tail =
            ElementQueue::__next_offset_by_len(self.buf_tail.get(), buf_mask, aligned_len);

        // Enough space in data buffer? Use wrapping arithmetic to mirror C semantics.
        let used = buf_tail
            .wrapping_add(aligned_len)
            .wrapping_sub(self.q.buf_head);
        if used > buf_mask + 1 {
            return Err(EqError::NoSpace);
        }

        // Reserve locally: update tails
        self.buf_tail.set(buf_tail.wrapping_add(aligned_len));
        let idx = (self.elem_tail.get() & self.q.elem_mask) as usize;
        // record element length in ring (visible to reader after publish)
        self.q.elems_mut()[idx] = len;
        self.elem_tail.set(self.elem_tail.get().wrapping_add(1));

        let start = (buf_tail & buf_mask) as usize;
        let end = start + len as usize;
        Ok(&mut self.q.data_mut()[start..end])
    }

    pub fn finish(self) -> &'q mut ElementQueue {
        // Commit local tails and publish to shared with release ordering
        self.q.elem_tail = self.elem_tail.get();
        self.q.buf_tail = self.buf_tail.get();
        fence(Ordering::Release);
        self.q.shared.set_buf_tail(self.q.buf_tail);
        self.q.shared.set_elem_tail(self.q.elem_tail);
        self.q
    }

    /// Move one element from `reader` to this writer within their active batches.
    pub fn move_from(&mut self, reader: &ReadBatch<'_>) -> Result<usize, EqError> {
        let src_len = reader.peek_len()? as usize;
        let dst = self.write(src_len as u32)?;
        let src = reader.read()?;
        debug_assert_eq!(src.len(), src_len);
        dst.copy_from_slice(src);
        Ok(src_len)
    }
}

/// Read batch guard with discard-on-drop semantics.
///
/// - Holds local copies of heads which are advanced during the batch.
/// - `finish(self)` commits local heads to the queue and publishes to shared.
/// - Dropping without `finish()` discards local progress (no commit/publish).
pub struct ReadBatch<'q> {
    q: &'q mut ElementQueue,
    elem_head: Cell<u32>,
    buf_head: Cell<u32>,
}

impl ElementQueue {
    /// Start a read batch (consumer) and return a guard.
    /// Loads writer-published tail and establishes acquire ordering.
    pub fn start_read<'q>(&'q mut self) -> ReadBatch<'q> {
        // We trust sizes in elem ring; only need elem_tail here.
        self.elem_tail = self.shared.get_elem_tail();
        // Acquire barrier: subsequent reads of elems/data happen-after.
        fence(Ordering::Acquire);
        ReadBatch {
            elem_head: Cell::new(self.elem_head),
            buf_head: Cell::new(self.buf_head),
            q: self,
        }
    }
}

impl<'q> ReadBatch<'q> {
    pub fn peek_len(&self) -> Result<u32, EqError> {
        if self.q.elem_tail == self.elem_head.get() {
            return Err(EqError::NoEntry);
        }
        let idx = (self.elem_head.get() & self.q.elem_mask) as usize;
        Ok(self.q.elems_ref()[idx])
    }

    pub fn peek(&self) -> Result<&[u8], EqError> {
        let len = self.peek_len()?;

        let aligned_len = (len + 7) & !7;
        let offset =
            ElementQueue::__next_offset_by_len(self.buf_head.get(), self.q.buf_mask, aligned_len)
                & self.q.buf_mask;
        let start = offset as usize;
        let end = start + len as usize;
        Ok(&self.q.data_ref()[start..end])
    }

    /// Peek a typed value from the next element without advancing the queue.
    /// Returns Err(NoEntry) if empty or Err(InvalidArg) if the element is too small.
    pub fn peek_value<T: Copy>(&self) -> Result<T, EqError> {
        let bytes = self.peek()?;
        let need = core::mem::size_of::<T>();
        if bytes.len() < need {
            return Err(EqError::InvalidArg);
        }
        // Copy bytes into an uninitialized T and assume init.
        let mut tmp = core::mem::MaybeUninit::<T>::uninit();
        unsafe {
            ptr::copy_nonoverlapping(bytes.as_ptr(), tmp.as_mut_ptr() as *mut u8, need);
            Ok(tmp.assume_init())
        }
    }

    /// Read next element, returning a slice into the data buffer.
    pub fn read(&self) -> Result<&[u8], EqError> {
        let len = self.peek_len()?;
        let aligned_len = (len + 7) & !7;
        let offset =
            ElementQueue::__next_offset_by_len(self.buf_head.get(), self.q.buf_mask, aligned_len);
        let start = (offset & self.q.buf_mask) as usize;
        let end = start + len as usize;
        // advance local heads
        self.elem_head.set(self.elem_head.get().wrapping_add(1));
        self.buf_head.set(offset.wrapping_add(aligned_len));
        Ok(&self.q.data_ref()[start..end])
    }

    pub fn finish(self) -> &'q mut ElementQueue {
        // Commit local heads and publish to shared with release ordering
        self.q.elem_head = self.elem_head.get();
        self.q.buf_head = self.buf_head.get();
        fence(Ordering::Release);
        self.q.shared.set_buf_head(self.q.buf_head);
        self.q.shared.set_elem_head(self.q.elem_head);
        self.q
    }
}

impl ElementQueue {
    #[inline]
    pub fn elem_count(&self) -> u32 {
        self.elem_tail.wrapping_sub(self.elem_head)
    }

    #[inline]
    pub fn elem_capacity(&self) -> u32 {
        self.elem_mask + 1
    }

    #[inline]
    pub fn buf_used(&self) -> u32 {
        self.buf_tail.wrapping_sub(self.buf_head)
    }

    #[inline]
    pub fn buf_capacity(&self) -> u32 {
        self.buf_mask + 1
    }

    #[inline]
    fn __next_offset_by_len(buf_offset: u32, buf_mask: u32, len: u32) -> u32 {
        // If this write would cross the end of buffer page, wrap to the start.
        let end = buf_offset.wrapping_add(len).wrapping_sub(1);
        let page_end = end & !buf_mask;
        let page_start = buf_offset & !buf_mask;
        if ((page_end.wrapping_sub(page_start)) as i32) > 0 {
            page_end
        } else {
            buf_offset
        }
    }

    #[inline]
    fn elems_ref(&self) -> &[u32] {
        // SAFETY: elems points to valid initialized slice for the life of self
        unsafe { self.elems.as_ref() }
    }

    #[inline]
    fn elems_mut(&mut self) -> &mut [u32] {
        // SAFETY: unique &mut self borrow guarantees exclusive access
        unsafe { self.elems.as_mut() }
    }

    #[inline]
    fn data_ref(&self) -> &[u8] {
        // SAFETY: data points to valid initialized slice for the life of self
        unsafe { self.data.as_ref() }
    }

    #[inline]
    fn data_mut(&mut self) -> &mut [u8] {
        // SAFETY: unique &mut self borrow guarantees exclusive access
        unsafe { self.data.as_mut() }
    }
}
