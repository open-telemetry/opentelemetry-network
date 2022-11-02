// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "stat_info.h"

namespace {

static constexpr std::string_view UNIT_BYTES = "By";
static constexpr std::string_view UNIT_MICROSECONDS = "us";
static constexpr std::string_view UNIT_DIMENSIONLESS = "1";

} // namespace

namespace reducer {

////////////////////////////////////////////////////////////////////////////////
// EBPF_NET
//

EbpfNetMetricInfo EbpfNetMetricInfo::pipeline_metric_bytes_discarded{
    EbpfNetMetrics::pipeline_metric_bytes_discarded,
    "some description"
    "some description",
    UNIT_MICROSECONDS};

EbpfNetMetricInfo EbpfNetMetricInfo::codetiming_min_ns{
    EbpfNetMetrics::codetiming_min_ns,
    "some description "
    "some description ",
    UNIT_MICROSECONDS};

EbpfNetMetricInfo EbpfNetMetricInfo::entrypoint_info{
    EbpfNetMetrics::entrypoint_info,
    "description"
    "description ",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::otlp_grpc_requests_sent{
    EbpfNetMetrics::otlp_grpc_requests_sent,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::connections{
    EbpfNetMetrics::connections,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::rpc_queue_elem_utilization_fraction{
    EbpfNetMetrics::rpc_queue_elem_utilization_fraction,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::disconnects{
    EbpfNetMetrics::codetiming_avg_ns,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::codetiming_avg_ns{
    EbpfNetMetrics::codetiming_avg_ns,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::client_handle_pool{
    EbpfNetMetrics::client_handle_pool,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::otlp_grpc_successful_requests{
    EbpfNetMetrics::otlp_grpc_successful_requests,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::span_utilization{
    EbpfNetMetrics::span_utilization,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::up{
    EbpfNetMetrics::up,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::rpc_queue_buf_utilization_fraction{
    EbpfNetMetrics::rpc_queue_buf_utilization_fraction,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::collector_log_count{
    EbpfNetMetrics::collector_log_count,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::time_since_last_message_ns{
    EbpfNetMetrics::time_since_last_message_ns,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::bpf_log{
    EbpfNetMetrics::bpf_log,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::codetiming_count{
    EbpfNetMetrics::codetiming_count,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::message{
    EbpfNetMetrics::message,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::otlp_grpc_bytes_sent{
    EbpfNetMetrics::otlp_grpc_bytes_sent,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::pipeline_message_error{
    EbpfNetMetrics::pipeline_message_error,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::pipeline_metric_bytes_written{
    EbpfNetMetrics::pipeline_metric_bytes_written,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::codetiming_max_ns{
    EbpfNetMetrics::codetiming_max_ns,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::span_utilization_max{
    EbpfNetMetrics::span_utilization_max,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::client_handle_pool_fraction{
    EbpfNetMetrics::client_handle_pool_fraction,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::span_utilization_fraction{
    EbpfNetMetrics::span_utilization_fraction,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::rpc_latency_ns{
    EbpfNetMetrics::rpc_latency_ns,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::agg_root_truncation{
    EbpfNetMetrics::agg_root_truncation,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::clock_offset_ns{
    EbpfNetMetrics::clock_offset_ns,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::otlp_grpc_metrics_sent{
    EbpfNetMetrics::otlp_grpc_metrics_sent,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::otlp_grpc_unknown_response_tags{
    EbpfNetMetrics::otlp_grpc_unknown_response_tags,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::collector_health{
    EbpfNetMetrics::collector_health,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::codetiming_sum_ns{
    EbpfNetMetrics::codetiming_sum_ns,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::otlp_grpc_failed_requests{
    EbpfNetMetrics::otlp_grpc_failed_requests,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::rpc_queue_buf_utilization{
    EbpfNetMetrics::rpc_queue_buf_utilization,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::pipeline_agent_connections{
    EbpfNetMetrics::rpc_queue_buf_utilization,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::bytes_ingested_by_prometheus{
    EbpfNetMetrics::rpc_queue_buf_utilization,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::pipeline_failed_scrapes{
    EbpfNetMetrics::rpc_queue_buf_utilization,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::big_items_dropped{
    EbpfNetMetrics::rpc_queue_buf_utilization,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};

EbpfNetMetricInfo EbpfNetMetricInfo::rpc_write_stalls{
    EbpfNetMetrics::rpc_queue_buf_utilization,
    " some definitions"
    " some description.",
    UNIT_DIMENSIONLESS};
} // namespace reducer
