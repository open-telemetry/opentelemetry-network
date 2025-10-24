#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_variables)]

use core::ffi::c_char;

#[repr(C)]
pub struct JbBlob {
    pub buf: *const c_char,
    pub len: u16,
}

// Include generated modules from src/
#[allow(dead_code)]
pub mod wire_messages {
    include!(concat!(env!("CARGO_MANIFEST_DIR"), "/src/wire_messages.rs"));
}

pub mod encoder {
    include!(concat!(env!("CARGO_MANIFEST_DIR"), "/src/encoder.rs"));
}
