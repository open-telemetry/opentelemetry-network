// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <util/log.h>

#include "outbound_metrics.h"
#include "outbound_stats.h"

// This class manages disabling metrics at the group and individual metric
// level.
//
// If you need to create a new group of metrics, see outbound_metrics.h
//
// To add additional metric groups to disable, merely add another line to
// the METRIC_GROUPS macro below:
//    X(ebpf_net, EbpfNetMetrics).
// the macros and templates in the class take care of the rest.
// However, if you need to make a certain metric in the group (or the metric
// group itself) disabled by default, update the  disabled_defaults()
// method below.

namespace reducer {

#define METRIC_GROUPS(X)                                                                                                       \
  X(tcp, TcpMetrics)                                                                                                           \
  X(udp, UdpMetrics)                                                                                                           \
  X(dns, DnsMetrics)                                                                                                           \
  X(http, HttpMetrics)                                                                                                         \
  X(ebpf_net, EbpfNetMetrics)

#define METRIC_FLAG(METRIC) METRIC##_disabled_flags_

class DisabledMetrics {
public:
  // disable metrics by string.
  // metrics are separated by commas
  // to disable an entire group, specify <group>.all (e.g. tcp.all)
  DisabledMetrics(std::string_view disabled_metrics_arg, std::string_view enabled_metrics_arg = "");

  // determine if the group as a whole is disabled
  // specialized versions of this template are declared later in this file.
  template <typename T> bool is_metric_group_disabled() const
  {
    assert(false);
    return true;
  }

  // inspect the disabled flags directly
  // specialized versions of this template are declared later in this file.
  template <typename T> std::uint64_t disabled_flags() const
  {
    assert(false);
    return 0u;
  }

  // check if a particular metric is disabled
#define METRIC_MEMBER_FNS(METRIC, ENUM_TYPE)                                                                                   \
  bool is_metric_disabled(ENUM_TYPE metric) const { return static_cast<std::uint64_t>(metric) & METRIC_FLAG(METRIC); }

  METRIC_GROUPS(METRIC_MEMBER_FNS)
#undef METRIC_MEMBER_FNS

private:
  void disabled_defaults()
  {
    // FUTURE insert any additional default disabled metrics here
    disable_metric("tcp.rtt.num_measurements");

    // shut all the internal stats off by default
    disable_metric("ebpf_net.all");
    // except the following
    enable_metric("ebpf_net.collector_health");
    enable_metric("ebpf_net.bpf_log");
    enable_metric("ebpf_net.otlp_grpc.bytes_failed");
    enable_metric("ebpf_net.otlp_grpc.bytes_sent");
    enable_metric("ebpf_net.otlp_grpc.metrics_failed");
    enable_metric("ebpf_net.otlp_grpc.metrics_sent");
    enable_metric("ebpf_net.otlp_grpc.requests_failed");
    enable_metric("ebpf_net.otlp_grpc.requests_sent");
    enable_metric("ebpf_net.otlp_grpc.unknown_response_tags");
    enable_metric("ebpf_net.up");
  }

  // disable a metric by its full name.
  // a metric of <group>.all (e.g. tcp.all) will disable the entire group.
  void disable_metric(const std::string &metric)
  {
    if (metric.empty()) {
      return;
    }

    size_t dot_pos = metric.find('.');
    if (dot_pos == std::string::npos) {
      return;
    }

    std::string prefix = metric.substr(0, dot_pos);

    disable_metric_by_prefix(prefix, metric);
  }

  // enable a metric by its full name.
  // this is just a convenience function for the disabled_defaults() method.
  // it has no user visibility (at this time).
  void enable_metric(const std::string &metric)
  {
    if (metric.empty()) {
      return;
    }

    size_t dot_pos = metric.find('.');
    if (dot_pos == std::string::npos) {
      return;
    }

    std::string prefix = metric.substr(0, dot_pos);

    enable_metric_by_prefix(prefix, metric);
  }

  // helper for mapping the prefix (e.g. tcp) to its enum type and bitset.
  void disable_metric_by_prefix(const std::string &prefix, const std::string &metric)
  {
#define IF_METRICS(METRIC, ENUM_TYPE)                                                                                          \
  if (prefix == #METRIC) {                                                                                                     \
    disable_metric<ENUM_TYPE>(metric, METRIC_FLAG(METRIC));                                                                    \
    return;                                                                                                                    \
  }

    METRIC_GROUPS(IF_METRICS)
#undef IF_METRICS

    LOG::warn("Unable to disable unknown metric {}", metric);
  }

  // helper for mapping the prefix (e.g. tcp) to its enum type and bitset.
  void enable_metric_by_prefix(const std::string &prefix, const std::string &metric)
  {
#define IF_METRICS(METRIC, ENUM_TYPE)                                                                                          \
  if (prefix == #METRIC) {                                                                                                     \
    enable_metric<ENUM_TYPE>(metric, METRIC_FLAG(METRIC));                                                                     \
    return;                                                                                                                    \
  }

    METRIC_GROUPS(IF_METRICS)
#undef IF_METRICS

    LOG::warn("Unable to enable unknown metric {}", metric);
  }

  // disable the metric in the given bit field.
  template <typename T> void disable_metric(const std::string &metric, std::uint64_t &flags);

  // enable the metric in the given bit field.
  template <typename T> void enable_metric(const std::string &metric, std::uint64_t &flags);

  // declare the bit fields
#define METRIC_MEMBERS(METRIC, ENUM_TYPE) std::uint64_t METRIC_FLAG(METRIC) = 0u;
  METRIC_GROUPS(METRIC_MEMBERS)
#undef METRIC_MEMBERS
};

// implementation of the above.
template <typename T> void DisabledMetrics::disable_metric(const std::string &metric, std::uint64_t &flags)
{
  T metric_enum = try_enum_from_string(metric, T::unknown);
  if (metric_enum == T::unknown) {
    LOG::warn("Unable to disable unknown metric {}", metric);
  } else {
    LOG::info("Disabling metric {}", metric);
    flags |= (u64)metric_enum;
  }
}

// implementation of the above.
template <typename T> void DisabledMetrics::enable_metric(const std::string &metric, std::uint64_t &flags)
{
  T metric_enum = try_enum_from_string(metric, T::unknown);
  if (metric_enum == T::unknown) {
    LOG::warn("Unable to enable unknown metric {}", metric);
  } else {
    LOG::info("Enabling metric {}", metric);
    flags &= ~((u64)metric_enum);
  }
}

// template specializations of the above.
#define METRIC_MEMBER_SPECIALIZATIONS(METRIC, ENUM_TYPE)                                                                       \
  template <> inline bool DisabledMetrics::is_metric_group_disabled<ENUM_TYPE>() const                                         \
  {                                                                                                                            \
    return METRIC_FLAG(METRIC) == static_cast<std::uint64_t>(ENUM_TYPE::all);                                                  \
  }                                                                                                                            \
  template <> inline std::uint64_t DisabledMetrics::disabled_flags<ENUM_TYPE>() const                                          \
  {                                                                                                                            \
    return METRIC_FLAG(METRIC);                                                                                                \
  }

METRIC_GROUPS(METRIC_MEMBER_SPECIALIZATIONS)
#undef METRIC_MEMBER_SPECIALIZATIONS

#undef METRIC_GROUPS
#undef METRIC_FLAG

} // namespace reducer
