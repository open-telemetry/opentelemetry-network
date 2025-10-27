#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_ingest;
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_matching;
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_aggregation;
#[allow(unused_extern_crates)]
extern crate encoder_ebpf_net_logging;
#[allow(unused_extern_crates)]
extern crate otlp_export;

extern "C" {
    pub fn otn_reducer_main(argc: c_int, argv: *const *const c_char) -> c_int;
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
    unsafe { otn_reducer_main(ptrs.len() as c_int, ptrs.as_ptr()) }
}
