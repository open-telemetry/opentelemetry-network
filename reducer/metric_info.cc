// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "metric_info.h"

namespace {

static constexpr std::string_view UNIT_BYTES = "By";
static constexpr std::string_view UNIT_MICROSECONDS = "us";
static constexpr std::string_view UNIT_DIMENSIONLESS = "1";

} // namespace

namespace reducer {

////////////////////////////////////////////////////////////////////////////////
// TCP
//

TcpMetricInfo TcpMetricInfo::bytes{
    TcpMetrics::bytes,
    "The total number of TCP bytes between the source and destination measured for the prior thirty seconds.",
    UNIT_BYTES,
    MetricTypeSum};

TcpMetricInfo TcpMetricInfo::rtt_num_measurements{
    TcpMetrics::rtt_num_measurements,
    "The number of measurements made in calculating the current RTT average value.",
    UNIT_DIMENSIONLESS,
    MetricTypeGauge};

TcpMetricInfo TcpMetricInfo::active{
    TcpMetrics::active,
    "The number of TCP connections considered to be open and alive between the source and destination at the point the measurement was taken.",
    UNIT_DIMENSIONLESS,
    MetricTypeGauge};

TcpMetricInfo TcpMetricInfo::rtt_average{
    TcpMetrics::rtt_average,
    "The computed average round trip time between the source and destination as measured in microseconds.",
    UNIT_MICROSECONDS,
    MetricTypeGauge};

TcpMetricInfo TcpMetricInfo::packets{
    TcpMetrics::packets,
    "The total number of TCP packets between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

TcpMetricInfo TcpMetricInfo::retrans{
    TcpMetrics::retrans,
    "The total number of TCP retransmission requests between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

TcpMetricInfo TcpMetricInfo::syn_timeouts{
    TcpMetrics::syn_timeouts,
    "The total number of TCP SYN timeouts between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

TcpMetricInfo TcpMetricInfo::new_sockets{
    TcpMetrics::new_sockets,
    "The total number of new TCP sockets opened between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

TcpMetricInfo TcpMetricInfo::resets{
    TcpMetrics::resets,
    "The total number of TCP resets sent between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

////////////////////////////////////////////////////////////////////////////////
// UDP
//

UdpMetricInfo UdpMetricInfo::bytes{
    UdpMetrics::bytes,
    "The total number of UDP bytes between the source and destination measured for the prior thirty seconds.",
    UNIT_BYTES,
    MetricTypeSum};

UdpMetricInfo UdpMetricInfo::packets{
    UdpMetrics::packets,
    "The total number of UDP packets between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

UdpMetricInfo UdpMetricInfo::active{
    UdpMetrics::active,
    "The number of UDP connections considered to be open and alive between the source and destination at the point the measurement was taken.",
    UNIT_DIMENSIONLESS,
    MetricTypeGauge};

UdpMetricInfo UdpMetricInfo::drops{
    UdpMetrics::drops,
    "The total number of UDP connections dropped between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

////////////////////////////////////////////////////////////////////////////////
// DNS
//

DnsMetricInfo DnsMetricInfo::client_duration_average{
    DnsMetrics::client_duration_average,
    "This metric is the average duration in microseconds from when the client sends a DNS request, until the response is received back from the server."
    " As such, it includes the communication round-trip times, plus the server processing latency."
    " Computed by the summation of all times, divided by dns.responses.",
    UNIT_MICROSECONDS,
    MetricTypeGauge};

DnsMetricInfo DnsMetricInfo::server_duration_average{
    DnsMetrics::server_duration_average,
    "This metric is the average duration in microseconds for the server to respond to a request received locally. "
    " Thus, it does not include the network latency from or to the client."
    " Computed by the summation of all times, divided by dns.responses.",
    UNIT_MICROSECONDS,
    MetricTypeGauge};

DnsMetricInfo DnsMetricInfo::active_sockets{
    DnsMetrics::active_sockets,
    "The number of DNS connections for which measurements were taken in the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeGauge};

DnsMetricInfo DnsMetricInfo::responses{
    DnsMetrics::responses,
    "The total number of DNS responses sent between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

DnsMetricInfo DnsMetricInfo::timeouts{
    DnsMetrics::timeouts,
    "The total number of DNS timeouts between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};

////////////////////////////////////////////////////////////////////////////////
// HTTP
//

HttpMetricInfo HttpMetricInfo::client_duration_average{
    HttpMetrics::client_duration_average,
    "This metric is the average duration in microseconds from when the client sends an HTTP request, until the response is received back from the server."
    " As such, it includes the communication round-trip times, plus the server processing latency."
    " Computed by summation of all times, divided by http.active_sockets.",
    UNIT_MICROSECONDS,
    MetricTypeGauge};

HttpMetricInfo HttpMetricInfo::server_duration_average{
    HttpMetrics::server_duration_average,
    "This metric is the average duration in microseconds for the server to respond to a request received locally."
    " Thus, it does not include the network latency from or to the client."
    " Computed by summation of all times, divided by http.active_sockets.",
    UNIT_MICROSECONDS,
    MetricTypeGauge};

HttpMetricInfo HttpMetricInfo::active_sockets{
    HttpMetrics::active_sockets,
    "The number of unencrypted HTTPv1 connections for which measurements were taken in the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeGauge};

HttpMetricInfo HttpMetricInfo::status_code{
    HttpMetrics::status_code,
    "For a given class of response code (see 'response_code' dimension), the number of times an unencrypted server sent an"
    " HTTPv1 status code between the source and destination measured for the prior thirty seconds.",
    UNIT_DIMENSIONLESS,
    MetricTypeSum};
} // namespace reducer
