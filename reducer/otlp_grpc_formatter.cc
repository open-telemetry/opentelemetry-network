// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>

#include "otlp_grpc_formatter.h"

#include <generated/ebpf_net/metrics.h>
#include <util/overloaded_visitor.h>

namespace reducer {

bool OtlpGrpcFormatter::metric_description_field_enabled_ = false;

void OtlpGrpcFormatter::set_metric_description_field_enabled(bool enabled)
{
  metric_description_field_enabled_ = enabled;
}

bool OtlpGrpcFormatter::metric_description_field_enabled()
{
  return metric_description_field_enabled_;
}

OtlpGrpcFormatter::OtlpGrpcFormatter(Publisher::WriterPtr const &writer)
{
  writer_ = dynamic_cast<OtlpGrpcPublisher::Writer *>(writer.get());
}

OtlpGrpcFormatter::~OtlpGrpcFormatter()
{
  flush();
}

void OtlpGrpcFormatter::flush()
{
  if (writer_) {
    writer_->flush();
  }
}

void OtlpGrpcFormatter::rebuild_labels(labels_t const &labels)
{
  labels_cache_.clear();
  for (auto const &kv : labels) {
    ::Label l;
    l.key = ::rust::String(kv.first);
    l.value = ::rust::String(kv.second);
    labels_cache_.push_back(l);
  }
}

void OtlpGrpcFormatter::format(
    MetricInfo const &metric_info,
    value_t val,
    std::string_view /*aggregation*/, // included in labels
    bool /*aggregation_changed*/,
    rollup_t /*rollup*/,
    bool /*rollup_changed*/,
    labels_t labels,
    bool labels_changed,
    timestamp_t timestamp,
    bool /*timestamp_changed*/,
    Publisher::WriterPtr const & /*unused_writer*/)
{
  if (!writer_)
    return;

  if (labels_changed) {
    rebuild_labels(labels);
  }

  auto &rp = writer_->rust_publisher();

  ::MetricKind kind = metric_info.type == MetricTypeSum ? ::MetricKind::Sum : ::MetricKind::Gauge;
  ::rust::Str name(metric_info.name);
  ::rust::Str unit(metric_info.unit);
  ::rust::Str description(OtlpGrpcFormatter::metric_description_field_enabled() ? metric_info.description : std::string(""));
  auto ts_ns = static_cast<int64_t>(timestamp.count());

  std::visit(
      overloaded_visitor{
          [&](u32 v) { rp.publish_metric_u64(name, unit, description, kind, labels_cache_, ts_ns, static_cast<uint64_t>(v)); },
          [&](u64 v) { rp.publish_metric_u64(name, unit, description, kind, labels_cache_, ts_ns, v); },
          [&](double v) { rp.publish_metric_f64(name, unit, description, kind, labels_cache_, ts_ns, v); },
      },
      val);
}

void OtlpGrpcFormatter::format_flow_log(
    ebpf_net::metrics::tcp_metrics const &tcp_metrics,
    labels_t labels,
    bool labels_changed,
    timestamp_t timestamp,
    bool /*timestamp_changed*/)
{
  if (!writer_)
    return;
  if (labels_changed) {
    rebuild_labels(labels);
  }
  auto &rp = writer_->rust_publisher();
  auto ts_ns = static_cast<int64_t>(timestamp.count());

  double sum_srtt = double(tcp_metrics.sum_srtt) / 8 / 1'000'000; // RTTs are measured in units of 1/8 microseconds.

  rp.publish_flow_log(
      labels_cache_,
      ts_ns,
      tcp_metrics.sum_bytes,
      tcp_metrics.active_rtts,
      tcp_metrics.active_sockets,
      tcp_metrics.active_rtts ? sum_srtt / tcp_metrics.active_rtts : 0.0,
      tcp_metrics.sum_delivered,
      tcp_metrics.sum_retrans,
      tcp_metrics.syn_timeouts,
      tcp_metrics.new_sockets,
      tcp_metrics.tcp_resets);
}

} // namespace reducer
