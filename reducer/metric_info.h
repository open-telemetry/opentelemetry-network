// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "outbound_metrics.h"

#include <string>
#include <string_view>

namespace reducer {
// Metric Types
//
enum MetricType {
  // OTLP Metric Data Sum
  MetricTypeSum,
  // OTLP Metric Data Gauge
  MetricTypeGauge,
};

// Information associated with a metric.
//
struct MetricInfo {
  // Canonical metric name (e.g. `tcp.bytes`).
  const std::string name;
  // Description of the metric.
  const std::string description;
  // Unit in which the metric value is reported (format described in http://unitsofmeasure.org/ucum.html).
  const std::string unit;
  // Metric Type
  MetricType type;

  explicit MetricInfo(
      std::string_view name_, std::string_view description_ = "", std::string_view unit_ = "", MetricType type_ = MetricTypeSum)
      : name(name_), description(description_), unit(unit_), type(type_)
  {}
};

// Used for information on outbound metrics.
// The template parameter is one of enums defined in `outbound_metrics.h`.
//
template <typename T> struct OutboundMetricInfo : public MetricInfo {
  typedef T metric_group_t;
  const metric_group_t metric;

  OutboundMetricInfo(T metric_, std::string_view description_, std::string_view unit_, MetricType type_ = MetricTypeSum)
      : MetricInfo(to_string(metric_), description_, unit_, type_), metric(metric_)
  {}
};

// Information on TCP outbound metrics.
//
struct TcpMetricInfo : public OutboundMetricInfo<TcpMetrics> {
  using OutboundMetricInfo<TcpMetrics>::OutboundMetricInfo;

  static TcpMetricInfo bytes;
  static TcpMetricInfo rtt_num_measurements;
  static TcpMetricInfo active;
  static TcpMetricInfo rtt_average;
  static TcpMetricInfo packets;
  static TcpMetricInfo retrans;
  static TcpMetricInfo syn_timeouts;
  static TcpMetricInfo new_sockets;
  static TcpMetricInfo resets;
};

// Information on UDP outbound metrics.
//
struct UdpMetricInfo : public OutboundMetricInfo<UdpMetrics> {
  using OutboundMetricInfo<UdpMetrics>::OutboundMetricInfo;

  static UdpMetricInfo bytes;
  static UdpMetricInfo packets;
  static UdpMetricInfo active;
  static UdpMetricInfo drops;
};

// Information on DNS outbound metrics.
//
struct DnsMetricInfo : public OutboundMetricInfo<DnsMetrics> {
  using OutboundMetricInfo<DnsMetrics>::OutboundMetricInfo;

  static DnsMetricInfo client_duration_average;
  static DnsMetricInfo server_duration_average;
  static DnsMetricInfo active_sockets;
  static DnsMetricInfo responses;
  static DnsMetricInfo timeouts;
};

// Information on HTTP outbound metrics.
//
struct HttpMetricInfo : public OutboundMetricInfo<HttpMetrics> {
  using OutboundMetricInfo<HttpMetrics>::OutboundMetricInfo;

  static HttpMetricInfo client_duration_average;
  static HttpMetricInfo server_duration_average;
  static HttpMetricInfo active_sockets;
  static HttpMetricInfo status_code;
};

} // namespace reducer
