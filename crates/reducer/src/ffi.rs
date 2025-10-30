#[cxx::bridge(namespace = "reducer_agg")]
mod ffi {
    // Plain descriptor for contiguous element-queue storage
    #[derive(Debug)]
    pub struct EqView {
        pub data: *mut u8, // base pointer to contiguous storage (shared)
        pub n_elems: u32,  // ring size (power of two)
        pub buf_len: u32,  // data buffer size (power of two)
    }

    extern "Rust" {
        type AggregationCore;

        /// Create a new AggregationCore from element-queue descriptors.
        fn aggregation_core_new(
            queues: &CxxVector<EqView>,
            shard: u32,
            enable_id_id: bool,
            enable_az_id: bool,
            endpoint: &str,
            disable_node_ip_field: bool,
            enable_metric_descriptions: bool,
        ) -> Box<AggregationCore>;
        /// Run the core loop until stopped.
        fn aggregation_core_run(self: Pin<&mut AggregationCore>);
        /// Request cooperative stop.
        fn aggregation_core_stop(self: Pin<&mut AggregationCore>);
    }
}

use crate::aggregation_core::AggregationCore;

impl AggregationCore {
    fn from_views(
        views: &cxx::CxxVector<ffi::EqView>,
        shard: u32,
        enable_id_id: bool,
        enable_az_id: bool,
        endpoint: &str,
        disable_node_ip_field: bool,
        enable_metric_descriptions: bool,
    ) -> Self {
        let mut v = Vec::with_capacity(views.len());
        for ev in views {
            v.push((ev.data as usize, ev.n_elems, ev.buf_len));
        }
        AggregationCore::new(
            &v[..],
            shard,
            enable_id_id,
            enable_az_id,
            endpoint,
            disable_node_ip_field,
            enable_metric_descriptions,
        )
    }
}

fn aggregation_core_new(
    queues: &cxx::CxxVector<ffi::EqView>,
    shard: u32,
    enable_id_id: bool,
    enable_az_id: bool,
    endpoint: &str,
    disable_node_ip_field: bool,
    enable_metric_descriptions: bool,
) -> Box<AggregationCore> {
    Box::new(AggregationCore::from_views(
        queues,
        shard,
        enable_id_id,
        enable_az_id,
        endpoint,
        disable_node_ip_field,
        enable_metric_descriptions,
    ))
}

impl AggregationCore {
    fn aggregation_core_run(self: core::pin::Pin<&mut Self>) {
        // Safe because we don't move after pin
        let core_ref: &mut AggregationCore = unsafe { self.get_unchecked_mut() };
        core_ref.run();
    }

    fn aggregation_core_stop(self: core::pin::Pin<&mut Self>) {
        let core_ref: &mut AggregationCore = unsafe { self.get_unchecked_mut() };
        core_ref.stop();
    }
}
