/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "labels.h"

#include <reducer/latency_accumulator.h>

#include <generated/ebpf_net/aggregation/weak_refs.h>
#include <generated/ebpf_net/metrics.h>

namespace reducer::aggregation {

class PercentileLatencies {
public:
  using LatencyAccumulator = ::reducer::LatencyAccumulator<FlowLabels>;

  LatencyAccumulator const &tcp() const { return tcp_; }
  LatencyAccumulator const &dns() const { return dns_; }
  LatencyAccumulator const &http() const { return http_; }

  std::string_view aggregation_name() const;

  void
  operator()(u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::tcp_metrics &metrics, u64 interval);
  void
  operator()(u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::udp_metrics &metrics, u64 interval);
  void
  operator()(u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::dns_metrics &metrics, u64 interval);
  void
  operator()(u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::http_metrics &metrics, u64 interval);

private:
  LatencyAccumulator tcp_;
  LatencyAccumulator dns_;
  LatencyAccumulator http_;
};

} // namespace reducer::aggregation
