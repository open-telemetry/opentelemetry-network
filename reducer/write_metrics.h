/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config.h>

#include "disabled_metrics.h"
#include "metric_info.h"
#include "outbound_metrics.h"
#include "tsdb_formatter.h"

#include <generated/ebpf_net/metrics.h>

namespace reducer {

#define WRITE(metric_info, value)                                                                                              \
  if (!disabled_metrics.is_metric_disabled(metric_info.metric)) {                                                              \
    formatter.write(metric_info, value, writer);                                                                               \
  }

inline double divide(double dividend, u32 divisor)
{
  return divisor ? dividend / divisor : 0.0;
}

inline void write_metrics(
    ebpf_net::metrics::tcp_metrics const &m,
    Publisher::WriterPtr const &writer,
    TsdbFormatter &formatter,
    const DisabledMetrics &disabled_metrics)
{
  if (disabled_metrics.is_metric_group_disabled<TcpMetrics>())
    return;

  double sum_srtt = double(m.sum_srtt) / 8 / 1'000'000; // RTTs are measured in units of 1/8 microseconds.

  WRITE(TcpMetricInfo::bytes, m.sum_bytes);
  WRITE(TcpMetricInfo::rtt_num_measurements, m.active_rtts);
  WRITE(TcpMetricInfo::active, m.active_sockets);
  WRITE(TcpMetricInfo::rtt_average, divide(sum_srtt, m.active_rtts));
  WRITE(TcpMetricInfo::packets, m.sum_delivered);
  WRITE(TcpMetricInfo::retrans, m.sum_retrans);
  WRITE(TcpMetricInfo::syn_timeouts, m.syn_timeouts);
  WRITE(TcpMetricInfo::new_sockets, m.new_sockets);
  WRITE(TcpMetricInfo::resets, m.tcp_resets);
}

inline void write_metrics(
    ebpf_net::metrics::udp_metrics const &m,
    Publisher::WriterPtr const &writer,
    TsdbFormatter &formatter,
    const DisabledMetrics &disabled_metrics)
{
  if (disabled_metrics.is_metric_group_disabled<UdpMetrics>())
    return;

  WRITE(UdpMetricInfo::bytes, m.bytes);
  WRITE(UdpMetricInfo::packets, m.packets);
  WRITE(UdpMetricInfo::active, m.active_sockets);
  WRITE(UdpMetricInfo::drops, m.drops);
}

inline void write_metrics(
    ebpf_net::metrics::dns_metrics const &m,
    Publisher::WriterPtr const &writer,
    TsdbFormatter &formatter,
    const DisabledMetrics &disabled_metrics)
{
  if (disabled_metrics.is_metric_group_disabled<DnsMetrics>())
    return;

  double dns_sum_total_time = double(m.sum_total_time_ns) / 1'000'000'000;
  double dns_sum_processing_time = double(m.sum_processing_time_ns) / 1'000'000'000;

  WRITE(DnsMetricInfo::client_duration_average, divide(dns_sum_total_time, m.responses));
  WRITE(DnsMetricInfo::server_duration_average, divide(dns_sum_processing_time, m.responses));
  WRITE(DnsMetricInfo::active_sockets, m.active_sockets);
  WRITE(DnsMetricInfo::responses, m.responses);
  WRITE(DnsMetricInfo::timeouts, m.timeouts);
}

inline void write_metrics(
    ebpf_net::metrics::http_metrics const &m,
    Publisher::WriterPtr const &writer,
    TsdbFormatter &formatter,
    const DisabledMetrics &disabled_metrics)
{
  if (disabled_metrics.is_metric_group_disabled<HttpMetrics>())
    return;

  double http_sum_total_time = double(m.sum_total_time_ns) / 1'000'000'000;
  double http_sum_processing_time = double(m.sum_processing_time_ns) / 1'000'000'000;

  WRITE(HttpMetricInfo::client_duration_average, divide(http_sum_total_time, m.active_sockets));
  WRITE(HttpMetricInfo::server_duration_average, divide(http_sum_processing_time, m.active_sockets));
  WRITE(HttpMetricInfo::active_sockets, m.active_sockets);

  static constexpr std::string_view status_code_label = "status_code";

  formatter.assign_label(status_code_label, "200");
  WRITE(HttpMetricInfo::status_code, m.sum_code_200);

  formatter.assign_label(status_code_label, "400");
  WRITE(HttpMetricInfo::status_code, m.sum_code_400);

  formatter.assign_label(status_code_label, "500");
  WRITE(HttpMetricInfo::status_code, m.sum_code_500);

  formatter.assign_label(status_code_label, "other");
  WRITE(HttpMetricInfo::status_code, m.sum_code_other);

  formatter.remove_label(status_code_label);
}

#undef WRITE

// The following functions are used to write metrics as flow logs.
inline void
write_flow_log(ebpf_net::metrics::tcp_metrics const &metrics, TsdbFormatter &formatter, const DisabledMetrics &disabled_metrics)
{
  if (disabled_metrics.is_metric_group_disabled<TcpMetrics>())
    return;

  formatter.write_flow_log(metrics);
}

inline void
write_flow_log(ebpf_net::metrics::udp_metrics const &metrics, TsdbFormatter &formatter, const DisabledMetrics &disabled_metrics)
{}

inline void
write_flow_log(ebpf_net::metrics::dns_metrics const &metrics, TsdbFormatter &formatter, const DisabledMetrics &disabled_metrics)
{}

inline void write_flow_log(
    ebpf_net::metrics::http_metrics const &metrics, TsdbFormatter &formatter, const DisabledMetrics &disabled_metrics)
{}

} // namespace reducer
