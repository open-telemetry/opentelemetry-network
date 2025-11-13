use core::ptr;
use core::ptr::{addr_of, addr_of_mut};

/// C-compatible shared indices for ElementQueue contiguous layout.
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct ElementQueueShared {
    pub elem_head: u32,
    pub buf_head: u32,
    pub elem_tail: u32,
    pub buf_tail: u32,
}

impl ElementQueueShared {
    /// Initialize shared indices to zero using volatile writes to match C semantics.
    pub unsafe fn init(shared: *mut ElementQueueShared) {
        shared.init_zero()
    }
}

/// Trait providing volatile read/write accessors on raw pointers to the
/// shared header. Enables pointer-style calls like `self.shared.set_buf_head(v)`.
pub(crate) trait ElementQueueSharedOps {
    fn init_zero(self);

    // Volatile getters (READ_ONCE semantics)
    fn get_elem_head(self) -> u32;
    fn get_buf_head(self) -> u32;
    fn get_elem_tail(self) -> u32;
    fn get_buf_tail(self) -> u32;

    // Volatile setters (WRITE_ONCE semantics)
    fn set_elem_head(self, v: u32);
    fn set_buf_head(self, v: u32);
    fn set_elem_tail(self, v: u32);
    fn set_buf_tail(self, v: u32);
}

impl ElementQueueSharedOps for *mut ElementQueueShared {
    #[inline]
    fn init_zero(self) {
        unsafe {
            ptr::write_volatile(addr_of_mut!((*self).elem_head), 0);
            ptr::write_volatile(addr_of_mut!((*self).buf_head), 0);
            ptr::write_volatile(addr_of_mut!((*self).elem_tail), 0);
            ptr::write_volatile(addr_of_mut!((*self).buf_tail), 0);
        }
    }

    #[inline]
    fn get_elem_head(self) -> u32 {
        unsafe { ptr::read_volatile(addr_of!((*self).elem_head)) }
    }
    #[inline]
    fn get_buf_head(self) -> u32 {
        unsafe { ptr::read_volatile(addr_of!((*self).buf_head)) }
    }
    #[inline]
    fn get_elem_tail(self) -> u32 {
        unsafe { ptr::read_volatile(addr_of!((*self).elem_tail)) }
    }
    #[inline]
    fn get_buf_tail(self) -> u32 {
        unsafe { ptr::read_volatile(addr_of!((*self).buf_tail)) }
    }

    #[inline]
    fn set_elem_head(self, v: u32) {
        unsafe { ptr::write_volatile(addr_of_mut!((*self).elem_head), v) }
    }
    #[inline]
    fn set_buf_head(self, v: u32) {
        unsafe { ptr::write_volatile(addr_of_mut!((*self).buf_head), v) }
    }
    #[inline]
    fn set_elem_tail(self, v: u32) {
        unsafe { ptr::write_volatile(addr_of_mut!((*self).elem_tail), v) }
    }
    #[inline]
    fn set_buf_tail(self, v: u32) {
        unsafe { ptr::write_volatile(addr_of_mut!((*self).buf_tail), v) }
    }
}

/// Compute the size of a contiguous element queue buffer in bytes.
pub fn contig_size(n_elems: u32, buf_len: u32) -> usize {
    core::mem::size_of::<ElementQueueShared>()
        + (n_elems as usize) * core::mem::size_of::<u32>()
        + (buf_len as usize)
}
