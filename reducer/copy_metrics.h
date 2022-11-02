/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace reducer {

template <typename Dst, typename Src> void copy_tcp_metrics(Dst &dst, Src const &src)
{
  dst.active_sockets = src.active_sockets;
  dst.sum_retrans = src.sum_retrans;
  dst.sum_bytes = src.sum_bytes;
  dst.sum_srtt = src.sum_srtt;
  dst.sum_delivered = src.sum_delivered;
  dst.active_rtts = src.active_rtts;
  dst.syn_timeouts = src.syn_timeouts;
  dst.new_sockets = src.new_sockets;
  dst.tcp_resets = src.tcp_resets;
}

template <typename Dst, typename Src> void copy_udp_metrics(Dst &dst, Src const &src)
{
  dst.active_sockets = src.active_sockets;
  dst.addr_changes = src.addr_changes;
  dst.packets = src.packets;
  dst.bytes = src.bytes;
  dst.drops = src.drops;
}

template <typename Dst, typename Src> void copy_dns_metrics(Dst &dst, Src const &src)
{
  dst.active_sockets = src.active_sockets;
  dst.requests_a = src.requests_a;
  dst.requests_aaaa = src.requests_aaaa;
  dst.responses = src.responses;
  dst.timeouts = src.timeouts;
  dst.sum_total_time_ns = src.sum_total_time_ns;
  dst.sum_processing_time_ns = src.sum_processing_time_ns;
}

template <typename Dst, typename Src> void copy_http_metrics(Dst &dst, Src const &src)
{
  dst.active_sockets = src.active_sockets;
  dst.sum_code_200 = src.sum_code_200;
  dst.sum_code_400 = src.sum_code_400;
  dst.sum_code_500 = src.sum_code_500;
  dst.sum_code_other = src.sum_code_other;
  dst.sum_total_time_ns = src.sum_total_time_ns;
  dst.sum_processing_time_ns = src.sum_processing_time_ns;
}

} // namespace reducer
