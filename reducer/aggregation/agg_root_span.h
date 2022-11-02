/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/aggregation/span_base.h>

namespace reducer::aggregation {

class AggRootSpan : public ::ebpf_net::aggregation::AggRootSpanBase {
public:
  AggRootSpan();
  ~AggRootSpan();

  void update_node(::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_node *msg);

  void update_tcp_metrics(
      ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_tcp_metrics *msg);

  void update_udp_metrics(
      ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_udp_metrics *msg);

  void update_http_metrics(
      ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_http_metrics *msg);

  void update_dns_metrics(
      ::ebpf_net::aggregation::weak_refs::agg_root span_ref, u64 timestamp, jsrv_aggregation__update_dns_metrics *msg);
};

} // namespace reducer::aggregation
