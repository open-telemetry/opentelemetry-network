#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_variables)]

use core::ffi::c_char;

#[repr(C)]
pub struct JbBlob {
    pub buf: *const c_char,
    pub len: u16,
}

// Modules use the standard Rust module system; files live under src/
pub mod encoder;
#[allow(dead_code)]
pub mod hash;
#[allow(dead_code)]
pub mod parsed_message;
#[allow(dead_code)]
pub mod wire_messages;
