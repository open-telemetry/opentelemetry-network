// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "percentile_latencies.h"
#include "labels.h"

namespace reducer::aggregation {

std::string_view PercentileLatencies::aggregation_name() const
{
  // We calculate percentile latencies only on the az-az aggregation.
  return "az_az";
}

void PercentileLatencies::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::tcp_metrics &metrics, u64 interval)
{
  FlowLabels key{az_az.az1(), az_az.az2()};
  double latency = metrics.active_rtts == 0 ? 0 : (double)metrics.sum_srtt / 8 / 1000 / metrics.active_rtts;
  // RTTs are measured in units of 1/8 microseconds.

  tcp_.add(t, std::move(key), latency);
}

void PercentileLatencies::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::udp_metrics &metrics, u64 interval)
{
  // not used
}

void PercentileLatencies::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::dns_metrics &metrics, u64 interval)
{
  FlowLabels key{az_az.az1(), az_az.az2()};
  double latency = metrics.active_sockets == 0 ? 0 : (double)metrics.sum_total_time_ns / 1000 / 1000 / metrics.active_sockets;

  dns_.add(t, std::move(key), latency);
}

void PercentileLatencies::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::http_metrics &metrics, u64 interval)
{
  FlowLabels key{az_az.az1(), az_az.az2()};
  double latency = metrics.active_sockets == 0 ? 0 : (double)metrics.sum_total_time_ns / 1000 / 1000 / metrics.active_sockets;

  http_.add(t, std::move(key), latency);
}

} // namespace reducer::aggregation
