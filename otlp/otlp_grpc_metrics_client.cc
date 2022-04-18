//
// Copyright 2022 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <opentelemetry/proto/metrics/v1/metrics.pb.h>

#include <otlp/otlp_grpc_metrics_client.h>
#include <util/json.h>
#include <util/log.h>

#include <ctime>

static const std::set<std::string> attr_keys = {
    "npm_daz",
    "npm_dcontainer",
    "npm_denv",
    "npm_dns",
    "npm_dprocess",
    "npm_drole",
    "npm_dtype",
    "npm_dver",
    "npm_saz",
    "npm_scontainer",
    "npm_senv",
    "npm_sns",
    "npm_sprocess",
    "npm_srole",
    "npm_stype",
    "npm_sver",
    "timestamp"};

namespace otlp {

OtlpGrpcMetricsClient::OtlpGrpcMetricsClient(std::shared_ptr<grpc::Channel> channel) : stub_(MetricsService::NewStub(channel))
{}

void OtlpGrpcMetricsClient::FormatAndExport(std::string const &metric_string)
{
  LOG::debug("OtlpGrpcMetricsClient:FormatAndExport() metric_string={}", metric_string);

  nlohmann::json metric_json;
  try {
    metric_json = nlohmann::json::parse(metric_string);
  } catch (std::exception &ex) {
    LOG::error("Failed to parse JSON.  metric_string={} ex.what()={}", metric_string, ex.what());
    return;
  }

  ExportMetricsServiceRequest request;
  try {
    auto resource_metrics = request.add_resource_metrics();

    auto scope_metrics = resource_metrics->add_scope_metrics();

    opentelemetry::proto::metrics::v1::Metric metric;

    std::string name(metric_json["name"]);
    std::string aggregation(metric_json["aggregation"]);
    metric.set_name(name + "_" + aggregation);
    metric.set_unit("number");

    opentelemetry::proto::metrics::v1::Sum sum;

    sum.set_aggregation_temporality(opentelemetry::proto::metrics::v1::AggregationTemporality::AGGREGATION_TEMPORALITY_DELTA);
    sum.set_is_monotonic(true);

    opentelemetry::proto::metrics::v1::NumberDataPoint data_point;

    for (auto const &key : attr_keys) {
      if (metric_json.contains(key)) {
        std::string value(metric_json[key]);
        auto attribute = data_point.add_attributes();
        attribute->set_key(key.data(), key.size());
        attribute->mutable_value()->set_string_value(value.data(), value.size());
      }
    }

    data_point.set_time_unix_nano(std::time(nullptr) * 1000000000);
    data_point.set_as_int(metric_json[name]);

    *sum.add_data_points() = std::move(data_point);

    *metric.mutable_sum() = std::move(sum);

    *scope_metrics->add_metrics() = std::move(metric);
  } catch (std::exception &ex) {
    LOG::error(
        "Failed to generate ExportMetricsServiceRequest from metric_json - skipping metric.  metric_string={} ex.what()={}",
        metric_string,
        ex.what());
    return;
  }

  std::string request_json_str;
  google::protobuf::util::JsonPrintOptions json_print_options;
  json_print_options.add_whitespace = true;
  json_print_options.always_print_primitive_fields = true;
  google::protobuf::util::MessageToJsonString(request, &request_json_str, json_print_options);
  LOG::trace("JSON view of ExportMetricsServiceRequest being sent: {}", request_json_str);

  auto status = Export(request);
  if (status.ok()) {
    LOG::debug("RPC succeeded.");
  } else {
    LOG::error("RPC failed: {}: {}", status.error_code(), log_waive(status.error_message()));
  }
}

grpc::Status OtlpGrpcMetricsClient::Export(ExportMetricsServiceRequest const &request)
{
  ExportMetricsServiceResponse response;
  grpc::ClientContext context;

  return stub_->Export(&context, request, &response);
}

} /* namespace otlp */
