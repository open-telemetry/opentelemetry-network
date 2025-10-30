use std::os::raw::{c_char, c_int};

// Ensure encoder crates are linked so C++ static libs can resolve their
// extern "C" symbols via our dependency graph.
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_ingest;

extern "C" {
    pub fn otn_k8s_relay_main(argc: c_int, argv: *const *const c_char) -> c_int;
}

pub fn run_with_args<I, S>(args: I) -> i32
where
    I: IntoIterator<Item = S>,
    S: Into<std::ffi::OsString>,
{
    use std::ffi::CString;

    let mut c_strings: Vec<CString> = Vec::new();
    for arg in args {
        let s: std::ffi::OsString = arg.into();
        let bytes = std::os::unix::ffi::OsStringExt::into_vec(s);
        c_strings.push(CString::new(bytes).expect("argv contains NUL byte"));
    }

    let ptrs: Vec<*const c_char> = c_strings.iter().map(|s| s.as_ptr()).collect();
    unsafe { otn_k8s_relay_main(ptrs.len() as c_int, ptrs.as_ptr()) }
}
