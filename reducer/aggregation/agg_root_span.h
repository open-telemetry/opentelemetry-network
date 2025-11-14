/*
 * Temporary stub while AggregationCore runs in Rust.
 * Render-generated aggregation spans include this header to define AggRootSpan.
 */
#pragma once

#include <generated/ebpf_net/aggregation/span_base.h>

namespace reducer {
namespace aggregation {

// Minimal stub: inherit default no-op handlers so generated code can link.
class AggRootSpan : public ::ebpf_net::aggregation::AggRootSpanBase {
public:
  AggRootSpan() = default;
};

} // namespace aggregation
} // namespace reducer
