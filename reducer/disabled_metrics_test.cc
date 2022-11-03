// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "disabled_metrics.h"

#include <gtest/gtest.h>

namespace reducer {

TEST(DisabledMetricsTest, AllEbpfEnabled)
{
  DisabledMetrics disabled_metrics("http.all", "ebpf_net.all");

  EXPECT_EQ(disabled_metrics.disabled_flags<EbpfNetMetrics>(), 0u);
  EXPECT_EQ(disabled_metrics.disabled_flags<HttpMetrics>(), 0xFFFFFFFF);
}

TEST(DisabledMetricsTest, IndividualInternalStatsEnabled)
{
  DisabledMetrics disabled_metrics("", "ebpf_net.message,ebpf_net.span_utilization_fraction");

  EXPECT_FALSE(disabled_metrics.is_metric_disabled(EbpfNetMetrics::message));
  EXPECT_FALSE(disabled_metrics.is_metric_disabled(EbpfNetMetrics::span_utilization_fraction));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(EbpfNetMetrics::client_handle_pool));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(EbpfNetMetrics::rpc_write_stalls));
}

TEST(DisabledMetricsTest, NothingDisabledExceptDefaults)
{
  // arrange
  DisabledMetrics disabled_metrics("none");

  // assert
  EXPECT_EQ(disabled_metrics.disabled_flags<UdpMetrics>(), 0u);
  EXPECT_EQ(disabled_metrics.disabled_flags<DnsMetrics>(), 0u);
  EXPECT_EQ(disabled_metrics.disabled_flags<HttpMetrics>(), 0u);
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::rtt_num_measurements));
  EXPECT_FALSE(disabled_metrics.is_metric_disabled(TcpMetrics::new_sockets));
  EXPECT_FALSE(disabled_metrics.is_metric_disabled(TcpMetrics::rtt_average));
}

TEST(DisabledMetricsTest, Enabled)
{
  // arrange
  DisabledMetrics disabled_metrics("tcp.all,udp.bytes,dns.timeouts");

  // assert
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::all));
  EXPECT_TRUE(disabled_metrics.is_metric_group_disabled<TcpMetrics>());

  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(UdpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(DnsMetrics::timeouts));

  EXPECT_FALSE(disabled_metrics.is_metric_disabled(HttpMetrics::active_sockets));
}

TEST(DisabledMetricsTest, HappyPath)
{
  // arrange
  DisabledMetrics disabled_metrics("tcp.all,udp.bytes,dns.timeouts");

  // assert
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::all));
  EXPECT_TRUE(disabled_metrics.is_metric_group_disabled<TcpMetrics>());

  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(UdpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(DnsMetrics::timeouts));

  EXPECT_FALSE(disabled_metrics.is_metric_disabled(HttpMetrics::active_sockets));
}

TEST(DisabledMetricsTest, HappyPathWhiteSpace)
{
  // arrange
  DisabledMetrics disabled_metrics(" tcp.all,    udp.bytes,\tdns.timeouts\n  , http.status_code   ");

  // assert
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::all));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_group_disabled<TcpMetrics>());

  EXPECT_TRUE(disabled_metrics.is_metric_disabled(UdpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(DnsMetrics::timeouts));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(HttpMetrics::status_code));

  EXPECT_FALSE(disabled_metrics.is_metric_disabled(HttpMetrics::active_sockets));
}

TEST(DisabledMetricsTest, InternalStats)
{
  DisabledMetrics disabled_metrics("");

  EXPECT_TRUE(disabled_metrics.is_metric_disabled(EbpfNetMetrics::codetiming_avg_ns));
  EXPECT_FALSE(disabled_metrics.is_metric_disabled(EbpfNetMetrics::collector_health));
}

TEST(DisabledMetricsTest, Casing)
{
  // arrange
  DisabledMetrics disabled_metrics("TcP.All, UDP.BYTES,dns.timeouts");

  // assert
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::all));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_group_disabled<TcpMetrics>());

  EXPECT_TRUE(disabled_metrics.is_metric_disabled(UdpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(DnsMetrics::timeouts));

  EXPECT_FALSE(disabled_metrics.is_metric_disabled(HttpMetrics::active_sockets));
}

TEST(DisabledMetricsTest, JustOneGroup)
{
  // arrange
  DisabledMetrics disabled_metrics("tcp.all");

  // assert
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::active));
  EXPECT_TRUE(disabled_metrics.is_metric_group_disabled<TcpMetrics>());

  EXPECT_FALSE(disabled_metrics.is_metric_disabled(HttpMetrics::active_sockets));
}

TEST(DisabledMetricsTest, DanglingSemi)
{
  // arrange
  DisabledMetrics disabled_metrics("tcp.all,");

  // assert
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::bytes));
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::active));
  EXPECT_TRUE(disabled_metrics.is_metric_group_disabled<TcpMetrics>());

  EXPECT_FALSE(disabled_metrics.is_metric_disabled(HttpMetrics::active_sockets));
}

// TODO when we define what the default disabled_metrics set is, update this test
TEST(DisabledMetricsTest, EmptyCtor)
{
  // arrange
  DisabledMetrics disabled_metrics("");

  // assert
  EXPECT_TRUE(disabled_metrics.is_metric_disabled(TcpMetrics::rtt_num_measurements));

  EXPECT_EQ(disabled_metrics.disabled_flags<UdpMetrics>(), 0u);
  EXPECT_EQ(disabled_metrics.disabled_flags<DnsMetrics>(), 0u);
  EXPECT_EQ(disabled_metrics.disabled_flags<HttpMetrics>(), 0u);
}

TEST(MetricEnumToStringTest, TcpEnumToString)
{
  std::string_view metric_str = to_string(TcpMetrics::active);
  EXPECT_STREQ(metric_str.data(), "tcp.active");
  metric_str = to_string(TcpMetrics::all);
  EXPECT_STREQ(metric_str.data(), "tcp.all");
  metric_str = to_string(TcpMetrics::bytes);
  EXPECT_STREQ(metric_str.data(), "tcp.bytes");
  metric_str = to_string(TcpMetrics::new_sockets);
  EXPECT_STREQ(metric_str.data(), "tcp.new_sockets");
  metric_str = to_string(TcpMetrics::packets);
  EXPECT_STREQ(metric_str.data(), "tcp.packets");
  metric_str = to_string(TcpMetrics::resets);
  EXPECT_STREQ(metric_str.data(), "tcp.resets");
  metric_str = to_string(TcpMetrics::retrans);
  EXPECT_STREQ(metric_str.data(), "tcp.retrans");
  metric_str = to_string(TcpMetrics::rtt_average);
  EXPECT_STREQ(metric_str.data(), "tcp.rtt.average");
  metric_str = to_string(TcpMetrics::rtt_num_measurements);
  EXPECT_STREQ(metric_str.data(), "tcp.rtt.num_measurements");
  metric_str = to_string(TcpMetrics::syn_timeouts);
  EXPECT_STREQ(metric_str.data(), "tcp.syn_timeouts");
  metric_str = to_string(TcpMetrics::unknown);
  EXPECT_STREQ(metric_str.data(), "unknown");
}

TEST(MetricEnumToStringTest, UdpEnumToString)
{
  std::string_view metric_str = to_string(UdpMetrics::active);
  EXPECT_STREQ(metric_str.data(), "udp.active");
  metric_str = to_string(UdpMetrics::all);
  EXPECT_STREQ(metric_str.data(), "udp.all");
  metric_str = to_string(UdpMetrics::bytes);
  EXPECT_STREQ(metric_str.data(), "udp.bytes");
  metric_str = to_string(UdpMetrics::drops);
  EXPECT_STREQ(metric_str.data(), "udp.drops");
  metric_str = to_string(UdpMetrics::packets);
  EXPECT_STREQ(metric_str.data(), "udp.packets");
  metric_str = to_string(UdpMetrics::unknown);
  EXPECT_STREQ(metric_str.data(), "unknown");
}

TEST(MetricEnumToStringTest, DnsEnumToString)
{
  std::string_view metric_str = to_string(DnsMetrics::active_sockets);
  EXPECT_STREQ(metric_str.data(), "dns.active_sockets");
  metric_str = to_string(DnsMetrics::all);
  EXPECT_STREQ(metric_str.data(), "dns.all");
  metric_str = to_string(DnsMetrics::client_duration_average);
  EXPECT_STREQ(metric_str.data(), "dns.client.duration.average");
  metric_str = to_string(DnsMetrics::responses);
  EXPECT_STREQ(metric_str.data(), "dns.responses");
  metric_str = to_string(DnsMetrics::server_duration_average);
  EXPECT_STREQ(metric_str.data(), "dns.server.duration.average");
  metric_str = to_string(DnsMetrics::timeouts);
  EXPECT_STREQ(metric_str.data(), "dns.timeouts");
  metric_str = to_string(DnsMetrics::unknown);
  EXPECT_STREQ(metric_str.data(), "unknown");
}

TEST(MetricEnumToStringTest, HttpEnumToString)
{
  std::string_view metric_str = to_string(HttpMetrics::active_sockets);
  EXPECT_STREQ(metric_str.data(), "http.active_sockets");
  metric_str = to_string(HttpMetrics::all);
  EXPECT_STREQ(metric_str.data(), "http.all");
  metric_str = to_string(HttpMetrics::client_duration_average);
  EXPECT_STREQ(metric_str.data(), "http.client.duration.average");
  metric_str = to_string(HttpMetrics::server_duration_average);
  EXPECT_STREQ(metric_str.data(), "http.server.duration.average");
  metric_str = to_string(HttpMetrics::status_code);
  EXPECT_STREQ(metric_str.data(), "http.status_code");
  metric_str = to_string(HttpMetrics::unknown);
  EXPECT_STREQ(metric_str.data(), "unknown");
}

TEST(StringToMetricEnumTest, StringToTcpEnum)
{
  EXPECT_EQ(try_enum_from_string("tcp.active", TcpMetrics::unknown), TcpMetrics::active);
  EXPECT_EQ(try_enum_from_string("tcp.all", TcpMetrics::unknown), TcpMetrics::all);
  EXPECT_EQ(try_enum_from_string("tcp.bytes", TcpMetrics::unknown), TcpMetrics::bytes);
  EXPECT_EQ(try_enum_from_string("tcp.new_sockets", TcpMetrics::unknown), TcpMetrics::new_sockets);
  EXPECT_EQ(try_enum_from_string("tcp.packets", TcpMetrics::unknown), TcpMetrics::packets);
  EXPECT_EQ(try_enum_from_string("tcp.resets", TcpMetrics::unknown), TcpMetrics::resets);
  EXPECT_EQ(try_enum_from_string("tcp.retrans", TcpMetrics::unknown), TcpMetrics::retrans);
  EXPECT_EQ(try_enum_from_string("tcp.rtt.average", TcpMetrics::unknown), TcpMetrics::rtt_average);
  EXPECT_EQ(try_enum_from_string("tcp.rtt.num_measurements", TcpMetrics::unknown), TcpMetrics::rtt_num_measurements);
  EXPECT_EQ(try_enum_from_string("tcp.syn_timeouts", TcpMetrics::unknown), TcpMetrics::syn_timeouts);

  EXPECT_EQ(try_enum_from_string("G4RBAgE!?", TcpMetrics::unknown), TcpMetrics::unknown);
}

TEST(StringToMetricEnumTest, StringToUdpEnum)
{
  EXPECT_EQ(try_enum_from_string("udp.active", UdpMetrics::unknown), UdpMetrics::active);
  EXPECT_EQ(try_enum_from_string("udp.all", UdpMetrics::unknown), UdpMetrics::all);
  EXPECT_EQ(try_enum_from_string("udp.bytes", UdpMetrics::unknown), UdpMetrics::bytes);
  EXPECT_EQ(try_enum_from_string("udp.drops", UdpMetrics::unknown), UdpMetrics::drops);
  EXPECT_EQ(try_enum_from_string("udp.packets", UdpMetrics::unknown), UdpMetrics::packets);

  EXPECT_EQ(try_enum_from_string("G4RBAgE!?", UdpMetrics::unknown), UdpMetrics::unknown);
}

TEST(StringToMetricEnumTest, StringToDnsEnum)
{
  EXPECT_EQ(try_enum_from_string("dns.active_sockets", DnsMetrics::unknown), DnsMetrics::active_sockets);
  EXPECT_EQ(try_enum_from_string("dns.all", DnsMetrics::unknown), DnsMetrics::all);
  EXPECT_EQ(try_enum_from_string("dns.client.duration.average", DnsMetrics::unknown), DnsMetrics::client_duration_average);
  EXPECT_EQ(try_enum_from_string("dns.responses", DnsMetrics::unknown), DnsMetrics::responses);
  EXPECT_EQ(try_enum_from_string("dns.server.duration.average", DnsMetrics::unknown), DnsMetrics::server_duration_average);
  EXPECT_EQ(try_enum_from_string("dns.timeouts", DnsMetrics::unknown), DnsMetrics::timeouts);

  EXPECT_EQ(try_enum_from_string("G4RBAgE!?", DnsMetrics::unknown), DnsMetrics::unknown);
}

TEST(StringToMetricEnumTest, StringToHttpEnum)
{
  EXPECT_EQ(try_enum_from_string("http.active_sockets", HttpMetrics::unknown), HttpMetrics::active_sockets);
  EXPECT_EQ(try_enum_from_string("http.all", HttpMetrics::unknown), HttpMetrics::all);
  EXPECT_EQ(try_enum_from_string("http.client.duration.average", HttpMetrics::unknown), HttpMetrics::client_duration_average);
  EXPECT_EQ(try_enum_from_string("http.server.duration.average", HttpMetrics::unknown), HttpMetrics::server_duration_average);
  EXPECT_EQ(try_enum_from_string("http.status_code", HttpMetrics::unknown), HttpMetrics::status_code);

  EXPECT_EQ(try_enum_from_string("G4RBAgE!?", HttpMetrics::unknown), HttpMetrics::unknown);
}

}; // namespace reducer
