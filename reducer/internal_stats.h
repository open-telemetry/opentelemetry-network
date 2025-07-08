/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "metric_info.h"

// The intention of these macros is to create a struct of related metrics (collectively called "Stats").  They
// are characterized by sharing the same set of labels.
// The BEGIN_LABELS/ LABEL / END_LABELS macros create a struct called "labels", with a public member
// for each LABEL macro invocation.  It also creates an iterative function called "foreach_label()" that can
// be called to yield all label names and values.
//
// The BEGIN_METRICS / METRIC / END_METRICS macros do the same, but the struct is called "metrics".
//
// To use these macros, create a containing struct, such as "SpanUtilizationStats".
// Inside the struct, begin defining the set of labels.
// You must first begin the label list with the "BEGIN_LABELS" macro
// Next, list each LABEL(), giving the label its name: e.g. LABEL(span).
// The name is both the field that will contain the label's value, as well as the "display" name.
// Finally, end the label section with "END_LABELS"
//
// Similarly, define the metrics.
// Begin the metric list with BEGIN_METRICS
// Add each metric with METRIC()  There are two arguments: the metric info, defined in metric_info.h
// and the name of the field that will store the metric value.
// End the metric section with END_METRICS.  For example:
//
// struct SomeStat {
//  BEGIN_LABELS
//  LABEL(field)
//  LABEL(field2)
//  END_LABELS
//
//  BEGIN_METRICS
//  METRIC(EbpfNetMetricInfo::Metric1, metric1)
//  METRIC(EbpfNetMetricInfo::Metric2, metric2)
//  END_METRICS
// };
//
// Lots of concrete examples follow after the macro declarations.
//
// The labels can be iterated through the "foreach_label()" function, which will callback
// with the name and the field value of each label.
// The metrics can be iterated through the "foreach_metric()" function in the same fashion.
// An example of how this is done can be found in internal_stats.h.
//
// Finally, access to the labels or metrics can be done directly via the labels or metrics structs.
//
// Since internal metrics are a cross-cutting concern, you can see examples of how to fill out a "Stat" in many places, such as
// agg_core.cc
//
// The common pattern is this:
//  SomeStat stats;
//  // fill the labels
//  stats.labels.field = value;
//  stats.labels.field2 = value2;
//  // fill the metrics
//  stats.metrics.value = metric_value1;
//  stats.metrics.value2 = metric_value2;
//  // write it out along the encoder
//  encoder.write_internal_stats(

namespace reducer {

#define BEGIN_LABELS                                                                                                           \
  struct Labels {                                                                                                              \
    using label_fn_t = std::function<void(std::string_view, std::string)>;                                                     \
    void foreach_label(label_fn_t func)                                                                                        \
    {                                                                                                                          \
      __foreach_label(_FirstLabelType(), func, *this);                                                                         \
    }                                                                                                                          \
    struct _FirstLabelType {};                                                                                                 \
    typedef _FirstLabelType

#define LABEL(LABEL_NAME)                                                                                                      \
  _LabelType_##LABEL_NAME;                                                                                                     \
  std::string LABEL_NAME;                                                                                                      \
  struct _NextLabelType_##LABEL_NAME {};                                                                                       \
  static void __foreach_label(_LabelType_##LABEL_NAME, label_fn_t func, Labels &this_struct)                                   \
  {                                                                                                                            \
    func(#LABEL_NAME, this_struct.LABEL_NAME);                                                                                 \
    __foreach_label(_NextLabelType_##LABEL_NAME(), func, this_struct);                                                         \
  }                                                                                                                            \
  typedef _NextLabelType_##LABEL_NAME

#define END_LABELS                                                                                                             \
  _LastLabelType;                                                                                                              \
  static void __foreach_label(_LastLabelType, label_fn_t func, Labels &this_struct) {}                                         \
  }                                                                                                                            \
  labels;

#define BEGIN_METRICS                                                                                                          \
  struct Metrics {                                                                                                             \
    using value_t = std::variant<u32, u64, double>;                                                                            \
    using metric_fn_t = std::function<void(const EbpfNetMetricInfo &, value_t)>;                                               \
                                                                                                                               \
    void foreach_metric(metric_fn_t func)                                                                                      \
    {                                                                                                                          \
      __foreach_metric(_FirstMetricType(), func, *this);                                                                       \
    }                                                                                                                          \
    struct _FirstMetricType {};                                                                                                \
    typedef _FirstMetricType

#define METRIC(METRIC_INFO, METRIC_NAME)                                                                                       \
  _MetricType_##METRIC_NAME;                                                                                                   \
  value_t METRIC_NAME;                                                                                                         \
  struct _NextMetricType_##METRIC_NAME {};                                                                                     \
  static void __foreach_metric(_MetricType_##METRIC_NAME, metric_fn_t func, Metrics &this_struct)                              \
  {                                                                                                                            \
    func(METRIC_INFO, this_struct.METRIC_NAME);                                                                                \
    __foreach_metric(_NextMetricType_##METRIC_NAME(), func, this_struct);                                                      \
  }                                                                                                                            \
  typedef _NextMetricType_##METRIC_NAME

#define END_METRICS                                                                                                            \
  _LastMetricType;                                                                                                             \
  static void __foreach_metric(_LastMetricType, metric_fn_t func, Metrics &this_struct) {}                                     \
  }                                                                                                                            \
  metrics;

#define COMMON_LABELS                                                                                                          \
  LABEL(module)                                                                                                                \
  LABEL(shard)

///////////////////////////////////////////////////////////////////////////////
// CoreBase
///////////////////////////////////////////////////////////////////////////////
struct SpanUtilizationStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(span)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::span_utilization, utilization)
  METRIC(EbpfNetMetricInfo::span_utilization_fraction, utilization_fraction)
  METRIC(EbpfNetMetricInfo::span_utilization_max, utilization_max)
  END_METRICS
};

struct ConnectionMessageStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(connection)
  LABEL(message)
  LABEL(severity)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::message, count)
  END_METRICS
};

struct ConnectionMessageErrorStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(connection)
  LABEL(message)
  LABEL(error)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::pipeline_message_error, count)
  END_METRICS
};

struct StatusStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(program)
  LABEL(version)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::up, status)
  END_METRICS
};

///////////////////////////////////////////////////////////////////////////////
// AggCore
///////////////////////////////////////////////////////////////////////////////

struct AggRootTruncationStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(field)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::agg_root_truncation, count)
  END_METRICS
};

struct AggPrometheusBytesStats {
  BEGIN_LABELS
  COMMON_LABELS
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::prometheus_bytes_written, prometheus_bytes_written)
  METRIC(EbpfNetMetricInfo::prometheus_bytes_discarded, prometheus_bytes_discarded)
  END_METRICS
};

struct CodeTimingStats {
  BEGIN_LABELS
  LABEL(name)
  LABEL(filename)
  LABEL(line)
  LABEL(index)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::codetiming_count, count)
  METRIC(EbpfNetMetricInfo::codetiming_avg_ns, avg_ns)
  METRIC(EbpfNetMetricInfo::codetiming_min_ns, min_ns)
  METRIC(EbpfNetMetricInfo::codetiming_max_ns, max_ns)
  METRIC(EbpfNetMetricInfo::codetiming_sum_ns, sum_ns)
  END_METRICS
};

///////////////////////////////////////////////////////////////////////////////
// AgentSpan
///////////////////////////////////////////////////////////////////////////////
#define COMMON_AGENT_SPAN_LABELS                                                                                               \
  COMMON_LABELS                                                                                                                \
  LABEL(version)                                                                                                               \
  LABEL(cloud)                                                                                                                 \
  LABEL(env)                                                                                                                   \
  LABEL(role)                                                                                                                  \
  LABEL(az)                                                                                                                    \
  LABEL(id)                                                                                                                    \
  LABEL(kernel)                                                                                                                \
  LABEL(c_type)                                                                                                                \
  LABEL(c_host)                                                                                                                \
  LABEL(os)                                                                                                                    \
  LABEL(os_version)

struct CollectorHealthStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  LABEL(status)
  LABEL(detail)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::collector_health, health)
  END_METRICS
};

struct BpfLogStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  LABEL(file)
  LABEL(line)
  LABEL(code)
  LABEL(arg0)
  LABEL(arg1)
  LABEL(arg2)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::bpf_log, bpf_log)
  END_METRICS
};

///////////////////////////////////////////////////////////////////////////////
// IngestCore
///////////////////////////////////////////////////////////////////////////////

struct ClientHandlePoolStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  LABEL(span)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::client_handle_pool, client_handle_pool)
  METRIC(EbpfNetMetricInfo::client_handle_pool_fraction, client_handle_pool_fraction)
  END_METRICS
};

struct ConnectionStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::time_since_last_message_ns, time_since_last_message_ns)
  METRIC(EbpfNetMetricInfo::clock_offset_ns, clock_offset_ns)
  END_METRICS
};

struct AgentConnectionMessageStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  LABEL(message)
  LABEL(severity)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::message, count)
  END_METRICS
};

struct AgentConnectionMessageErrorStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  LABEL(message)
  LABEL(error)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::pipeline_message_error, count)
  END_METRICS
};

struct CollectorLogStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  LABEL(severity)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::collector_log_count, collector_log_count)
  END_METRICS
};

struct EntrypointStats {
  BEGIN_LABELS
  COMMON_AGENT_SPAN_LABELS
  LABEL(kernel_headers_source)
  LABEL(error)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::entrypoint_info, entrypoint_info)
  END_METRICS
};

struct ServerStats {
  BEGIN_LABELS
  LABEL(module)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::connections, connections)
  METRIC(EbpfNetMetricInfo::disconnects, disconnects)
  END_METRICS
};

///////////////////////////////////////////////////////////////////////////////
// LoggingCore
///////////////////////////////////////////////////////////////////////////////
struct PipelineAgentStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(hostname)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::pipeline_agent_connections, connections)
  END_METRICS
};

///////////////////////////////////////////////////////////////////////////////
// OtlpGrpcPublisher
///////////////////////////////////////////////////////////////////////////////
struct OtlpGrpcStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(client_type)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::otlp_grpc_bytes_failed, bytes_failed)
  METRIC(EbpfNetMetricInfo::otlp_grpc_bytes_sent, bytes_sent)
  METRIC(EbpfNetMetricInfo::otlp_grpc_metrics_failed, metrics_failed)
  METRIC(EbpfNetMetricInfo::otlp_grpc_metrics_sent, metrics_sent)
  METRIC(EbpfNetMetricInfo::otlp_grpc_requests_failed, requests_failed)
  METRIC(EbpfNetMetricInfo::otlp_grpc_requests_sent, requests_sent)
  METRIC(EbpfNetMetricInfo::otlp_grpc_unknown_response_tags, unknown_response_tags)
  END_METRICS
};

///////////////////////////////////////////////////////////////////////////////
// PrometheusPublisher
///////////////////////////////////////////////////////////////////////////////
struct PromStats {
  BEGIN_LABELS
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::prometheus_bytes_ingested, bytes_ingested)
  METRIC(EbpfNetMetricInfo::prometheus_failed_scrapes, failed_scrapes)
  END_METRICS
};

struct BigItemsDroppedStats {
  BEGIN_LABELS
  COMMON_LABELS
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::prometheus_big_items_dropped, prometheus_big_items_dropped)
  END_METRICS
};

///////////////////////////////////////////////////////////////////////////////
// RpcStats
///////////////////////////////////////////////////////////////////////////////
struct RpcWriteStallsStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(peer)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::rpc_write_stalls, stalls)
  END_METRICS
};

struct RpcQueueUtilizationStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(peer)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::rpc_queue_buf_utilization, max_buf_used)
  METRIC(EbpfNetMetricInfo::rpc_queue_buf_utilization_fraction, max_buf_util)
  METRIC(EbpfNetMetricInfo::rpc_queue_elem_utilization_fraction, max_elem_util)
  END_METRICS
};

struct RpcLatencyStats {
  BEGIN_LABELS
  COMMON_LABELS
  LABEL(peer)
  END_LABELS

  BEGIN_METRICS
  METRIC(EbpfNetMetricInfo::rpc_latency_ns, max_latency_ns)
  END_METRICS
};

#undef BEGIN_LABELS
#undef END_LABELS
#undef LABEL

#undef BEGIN_METRICS
#undef END_METRICS
#undef METRIC

#undef INTERNAL_METRIC_PREFIX

#undef COMMON_LABELS
#undef COMMON_AGENT_SPAN_LABELS
} // namespace reducer
