// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

void TsdbEncoder::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::agg_root &span, ::ebpf_net::metrics::METRICS &metrics, u64 interval)
{}

void TsdbEncoder::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::node_node &span, ::ebpf_net::metrics::METRICS &metrics, u64 interval)
{
  if (id_id_enabled_) {
    if (!metric_writers_.empty()) {
      auto writer_num = span.loc() % metric_writers_.size();
      auto &metric_writer = metric_writers_[writer_num];

      NodeLabels k[2] = {span.node1(), span.node2()};

      encode_and_write(metric_writer, "id_id", {k[reverse_], k[1 - reverse_]}, metrics);
    }

    if (otlp_metric_writer_) {
      NodeLabels k[2] = {span.node1(), span.node2()};

      encode_and_write_otlp_grpc(otlp_metric_writer_, "id_id", {k[reverse_], k[1 - reverse_]}, metrics);
    }
  }

  if (flow_logs_enabled_ && otlp_metric_writer_) {
    NodeLabels k[2] = {span.node1(), span.node2()};

    encode_and_write_otlp_grpc_flow_log({k[reverse_], k[1 - reverse_]}, metrics);
  }

  // If there was activity in this timeslot, start a new timeslot
  // so there is a zero report at the end.
  // Also this keeps handles for another interval so should reduce
  // handle churn.
  if (metrics.active_sockets > 0) {
    ::ebpf_net::metrics::METRICS zero_metrics = {};

    if (reverse_ == 0)
      span.A_B_UPDATE(t + interval, zero_metrics);
    else
      span.B_A_UPDATE(t + interval, zero_metrics);
  }
}

void TsdbEncoder::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::az_node &az_node, ::ebpf_net::metrics::METRICS &metrics, u64 interval)
{
  if (az_id_enabled_) {
    if (!metric_writers_.empty()) {
      auto writer_num = az_node.loc() % metric_writers_.size();
      auto &metric_writer = metric_writers_[writer_num];
      NodeLabels k[2] = {az_node.az(), az_node.node()};

      encode_and_write(metric_writer, (reverse_ == 0) ? "az_id" : "id_az", {k[reverse_], k[1 - reverse_]}, metrics);
    }

    if (otlp_metric_writer_) {
      NodeLabels k[2] = {az_node.az(), az_node.node()};

      encode_and_write_otlp_grpc(
          otlp_metric_writer_, (reverse_ == 0) ? "az_id" : "id_az", {k[reverse_], k[1 - reverse_]}, metrics);
    }
  }
}

void TsdbEncoder::operator()(
    u64 t, ::ebpf_net::aggregation::weak_refs::az_az &az_az, ::ebpf_net::metrics::METRICS &metrics, u64 interval)
{
  if (!metric_writers_.empty()) {
    auto writer_num = az_az.loc() % metric_writers_.size();

    auto &metric_writer = metric_writers_[writer_num];

    NodeLabels k[2] = {az_az.az1(), az_az.az2()};

    encode_and_write(metric_writer, "az_az", {k[0], k[1]}, metrics);
  }

  if (otlp_metric_writer_) {
    NodeLabels k[2] = {az_az.az1(), az_az.az2()};

    encode_and_write_otlp_grpc(otlp_metric_writer_, "az_az", {k[0], k[1]}, metrics);
  }
}
