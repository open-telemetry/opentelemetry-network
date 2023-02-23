// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "metrics_server_impl.h"

#include <cmath>
#include <fstream>

#include <util/time.h>

using namespace grpc;
using namespace opentelemetry::proto::collector::metrics::v1;
using namespace opentelemetry::proto::metrics::v1;
using namespace opentelemetry::proto::common::v1;

MetricsServiceImpl::MetricsServiceImpl(Sinks sinks) : sinks_(std::move(sinks)) {}

Status MetricsServiceImpl::Export(
    ServerContext *context, const ExportMetricsServiceRequest *request, ExportMetricsServiceResponse *response)
{
  for (const auto &rm : request->resource_metrics()) {
    write_resource_metrics(rm);
  }

  flush();
  return Status::OK;
}

void MetricsServiceImpl::write_resource_metrics(const ResourceMetrics &rm)
{
  for (const auto &sm : rm.scope_metrics()) {
    write_scope_metrics(sm);
  }
}

void MetricsServiceImpl::write_scope_metrics(const ScopeMetrics &sm)
{
  for (const auto &m : sm.metrics()) {
    write_metric(m);
  }
}

void MetricsServiceImpl::write_metric(const Metric &m)
{
  switch (m.data_case()) {
  case Metric::DataCase::kGauge:
    write_gauge(m.name(), m.gauge());
    break;
  case Metric::DataCase::kSum:
    write_sum(m.name(), m.sum());
    break;
  // Histogram, ExponentialHistogram, Summary not supported
  default:
    std::cerr << "Error writing metric - unhandled data type: " << m.data_case() << std::endl;
    break;
  }
}

void MetricsServiceImpl::write_sum(const std::string &name, const Sum &sum)
{
  for (const auto &dp : sum.data_points()) {
    write_number_data_point(name, dp);
  }
}

void MetricsServiceImpl::write_gauge(const std::string &name, const Gauge &gauge)
{
  for (const auto &dp : gauge.data_points()) {
    write_number_data_point(name, dp);
  }
}

void MetricsServiceImpl::write_number_data_point(const std::string &name, const NumberDataPoint &dp)
{
  // the metric name
  write(name);

  write("{");

  // labels
  bool once = false;
  for (const auto &a : dp.attributes()) {
    if (!once) {
      once = true;
    } else {
      write(",");
    }

    write_key_value(a);
  }

  write("} ");

  // the metric datapoint value
  switch (dp.value_case()) {
  case NumberDataPoint::ValueCase::kAsInt:
    write(dp.as_int());
    break;
  case NumberDataPoint::ValueCase::kAsDouble:
    write(dp.as_double());
    break;
  default:
    // shouldn't happen - only int or double is supported in the proto
    std::cerr << "Error writing NumberDataPoint - unhandled type: " << dp.value_case() << std::endl;
    break;
  }

  write(" ");

  // the metric datapoint timestamp
  // prometheus format is in ms
  std::chrono::nanoseconds timestamp(dp.time_unix_nano());
  write(integer_time<std::chrono::milliseconds>(timestamp));

  write("\n");
}

void MetricsServiceImpl::write_key_value(const KeyValue &kv)
{
  write(kv.key());
  write("=\"");
  write_any_value(kv.value());
  write("\"");
}

void MetricsServiceImpl::write_any_value(const AnyValue &v)
{
  switch (v.value_case()) {
  case AnyValue::ValueCase::kStringValue:
    write(v.string_value());
    break;
  case AnyValue::ValueCase::kBoolValue:
    write(v.bool_value());
    break;
  case AnyValue::ValueCase::kIntValue:
    write(v.int_value());
    break;
  case AnyValue::ValueCase::kDoubleValue:
    write(v.double_value());
    break;
  default:
    // shouldn't hit here, since I don't think we are ever going to send array value, kev value list
    // or bytes...but just in case
    std::cerr << "Error writing AnyValue - unhandled type: " << v.value_case() << std::endl;
    break;
  }
}

void MetricsServiceImpl::flush()
{
  for (auto sink : sinks_) {
    sink->flush();
  }
}
