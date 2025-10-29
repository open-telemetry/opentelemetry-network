/**********************************************
 * !!! render-generated code, do not modify !!!
 **********************************************/

#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]

  #[repr(C)]
  #[derive(Copy, Clone)]
  pub struct jb_cloud_collector__pulse {
    pub _rpc_id: u16,
  }

  impl Default for jb_cloud_collector__pulse {
    #[inline]
    fn default() -> Self { unsafe { core::mem::zeroed() } }
  }

  pub const PULSE_WIRE_SIZE: usize = 2;

  #[cfg(test)]
  mod pulse_layout_tests {
    use super::*;
    use core::mem::{offset_of, align_of};
    #[test]
    fn struct_size() {
      let size = size_of::<jb_cloud_collector__pulse>();
      let align = align_of::<jb_cloud_collector__pulse>();
      let padded_raw_size = (PULSE_WIRE_SIZE + align - 1) / align * align;
      assert_eq!(size, padded_raw_size);
    }
    #[test]
    fn field_offsets() {
      assert_eq!(offset_of!(jb_cloud_collector__pulse, _rpc_id), 0);
    }
  }

