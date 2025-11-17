#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_extern_crates)]
extern crate encoder_ebpf_net_agent_internal as _crate_agent_internal;
extern crate encoder_ebpf_net_aggregation as _crate_aggregation;
extern crate encoder_ebpf_net_cloud_collector as _crate_cloud_collector;
extern crate encoder_ebpf_net_ingest as _crate_ingest;
extern crate encoder_ebpf_net_kernel_collector as _crate_kernel_collector;
extern crate encoder_ebpf_net_logging as _crate_logging;
extern crate encoder_ebpf_net_matching as _crate_matching;
#[no_mangle]
pub extern "C" fn __encoder_ebpf_net_bundle_marker() {}
