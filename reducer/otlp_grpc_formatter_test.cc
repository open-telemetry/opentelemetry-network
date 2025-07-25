// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "otlp_grpc_formatter.h"
#include "publisher.h"

#include <otlp/otlp_util.h>
#include <util/common_test.h>
#include <util/json.h>
#include <util/overloaded_visitor.h>
#include <util/time.h>

#include <generated/ebpf_net/metrics.h>

#include <string>

namespace reducer {

class TestPublisherWriter : public Publisher::Writer {
public:
  TestPublisherWriter(
      ExportLogsServiceRequest &logs_request_to_validate, ExportMetricsServiceRequest &metrics_request_to_validate)
      : logs_request_to_validate_(logs_request_to_validate), metrics_request_to_validate_(metrics_request_to_validate){};

  TestPublisherWriter(TestPublisherWriter const &) = delete;
  TestPublisherWriter(TestPublisherWriter &&) = default;

  ~TestPublisherWriter(){};

  void write(ExportLogsServiceRequest &request) override { logs_request_to_validate_ = request; };
  void write(ExportMetricsServiceRequest &request) override { metrics_request_to_validate_ = request; };

  void flush() override{};

private:
  ExportLogsServiceRequest &logs_request_to_validate_;
  ExportMetricsServiceRequest &metrics_request_to_validate_;
};

class OtlpGrpcFormatterTest : public CommonTest {
protected:
  void SetUp() override
  {
    CommonTest::SetUp();

    writer_ = std::make_unique<TestPublisherWriter>(logs_request_to_validate_, metrics_request_to_validate_);
    formatter_ = TsdbFormatter::make(TsdbFormat::otlp_grpc, writer_);
  }

  void send_and_validate(
      ebpf_net::metrics::tcp_metrics const &tcp_metrics, TsdbFormatter::timestamp_t timestamp, TsdbFormatter::labels_t labels)
  {
    formatter_->set_labels(std::move(labels));
    formatter_->set_timestamp(timestamp);
    formatter_->write_flow_log(tcp_metrics);
    formatter_->flush();

    validate_logs_request(tcp_metrics, timestamp);
  }

  void validate_logs_request(ebpf_net::metrics::tcp_metrics const &tcp_metrics, TsdbFormatter::timestamp_t timestamp)
  {
    auto request_json_opt = get_request_json(logs_request_to_validate_);
    if (!request_json_opt.has_value()) {
      LOG::error("Failed to get JSON from logs request");
      FAIL();
    }
    auto request_json_str = request_json_opt.value();
    LOG::trace("JSON view of ExportLogsServiceRequest: {}", request_json_str);

    nlohmann::json request_json;
    try {
      request_json = nlohmann::json::parse(request_json_str);
    } catch (std::exception &ex) {
      LOG::error("Failed to parse JSON.  request_json_str={} ex.what()={}", request_json_str, ex.what());
      FAIL();
    }

    try {
      for (auto const &rl : request_json.at("resource_logs")) {
        for (auto const &sl : rl.at("scope_logs")) {
          for (auto const &log : sl.at("log_records")) {
            EXPECT_EQ(
                (unsigned long long)(integer_time<std::chrono::nanoseconds>(timestamp)),
                std::stoull(std::string(log.at("time_unix_nano"))));
            EXPECT_EQ("SEVERITY_NUMBER_INFO", log.at("severity_number"));
            EXPECT_EQ("INFO", log.at("severity_text"));
            double sum_srtt = double(tcp_metrics.sum_srtt) / 8 / 1'000'000; // RTTs are measured in units of 1/8 microseconds.
            std::string log_message(
                formatter_->labels_["source.ip"] + " " + formatter_->labels_["source.workload.name"] + " " +
                formatter_->labels_["dest.ip"] + " " + formatter_->labels_["dest.workload.name"] + " " +
                std::to_string(tcp_metrics.sum_bytes) + " " + std::to_string(tcp_metrics.active_rtts) + " " +
                std::to_string(tcp_metrics.active_sockets) + " " +
                fmt::format("{} ", tcp_metrics.active_rtts ? sum_srtt / tcp_metrics.active_rtts : 0.0) +
                std::to_string(tcp_metrics.sum_delivered) + " " + std::to_string(tcp_metrics.sum_retrans) + " " +
                std::to_string(tcp_metrics.syn_timeouts) + " " + std::to_string(tcp_metrics.new_sockets) + " " +
                std::to_string(tcp_metrics.tcp_resets));
            EXPECT_EQ(log_message, log.at("body").at("string_value"));
          }
        }
      }
    } catch (std::exception &ex) {
      LOG::error("Exception during validation.  request_json={} ex.what()={}", log_waive(request_json.dump()), ex.what());
      FAIL();
    }
  }

  void send_and_validate(
      std::string_view name, TsdbFormatter::value_t value, TsdbFormatter::timestamp_t timestamp, TsdbFormatter::labels_t labels)
  {
    formatter_->set_aggregation("az_az");
    formatter_->set_labels(std::move(labels));
    formatter_->set_timestamp(timestamp);
    formatter_->write(name, value, writer_);
    formatter_->flush();

    validate_metrics_request(name, value, timestamp);
  }

  void
  validate_metrics_request(std::string_view name, TsdbFormatter::value_t metric_value, TsdbFormatter::timestamp_t timestamp)
  {
    auto request_json_opt = get_request_json(metrics_request_to_validate_);
    if (!request_json_opt.has_value()) {
      LOG::error("Failed to get JSON from metrics request");
      FAIL();
    }
    auto request_json_str = request_json_opt.value();
    LOG::trace("JSON view of ExportMetricsServiceRequest: {}", request_json_str);

    auto labels_to_validate = formatter_->labels_;

    nlohmann::json request_json;
    try {
      request_json = nlohmann::json::parse(request_json_str);
    } catch (std::exception &ex) {
      LOG::error("Failed to parse JSON.  request_json_str={} ex.what()={}", request_json_str, ex.what());
      FAIL();
    }

    try {
      for (auto const &rm : request_json.at("resource_metrics")) {
        for (auto const &sm : rm.at("scope_metrics")) {
          for (auto const &metric : sm.at("metrics")) {
            EXPECT_EQ(name, metric.at("name"));
            auto const &sum = metric.at("sum");
            EXPECT_EQ("AGGREGATION_TEMPORALITY_DELTA", sum.at("aggregation_temporality"));
            EXPECT_TRUE(sum.at("is_monotonic"));
            for (auto const &data_point : sum.at("data_points")) {
              std::visit(
                  overloaded_visitor{
                      [&](u32 val) { EXPECT_EQ(val, stoul(std::string(data_point.at("as_int")))); },
                      [&](u64 val) { EXPECT_EQ(val, stoull(std::string(data_point.at("as_int")))); },
                      [&](double val) { EXPECT_EQ(val, data_point.at("as_double")); }},
                  metric_value);
              EXPECT_EQ(
                  (u64)integer_time<std::chrono::nanoseconds>(timestamp),
                  std::stoull(std::string(data_point.at("time_unix_nano"))));
              for (auto const &attribute : data_point.at("attributes")) {
                auto key = attribute.at("key");
                auto value = attribute.at("value").at("string_value");
                EXPECT_EQ(labels_to_validate.count(key), 1ull);
                EXPECT_EQ(labels_to_validate.at(key), value);
                labels_to_validate.erase(key);
              }
              EXPECT_EQ(labels_to_validate.size(), 0ull);
            }
          }
        }
      }
    } catch (std::exception &ex) {
      LOG::error("Exception during validation.  request_json={} ex.what()={}", log_waive(request_json.dump()), ex.what());
      FAIL();
    }
  }

  std::unique_ptr<TsdbFormatter> formatter_;
  std::unique_ptr<Publisher::Writer> writer_;
  ExportLogsServiceRequest logs_request_to_validate_;
  ExportMetricsServiceRequest metrics_request_to_validate_;
};

TEST_F(OtlpGrpcFormatterTest, ValidateLogsRequest)
{
  TsdbFormatter::labels_t labels{
      {"source.ip", "127.0.0.1"},
      {"source.workload.name", "frontend"},
      {"dest.ip", "192.168.0.1"},
      {"dest.workload.name", "cart-service"}};

  ebpf_net::metrics::tcp_metrics tcp_metrics{
      .active_sockets = 11,
      .sum_retrans = 2,
      .sum_bytes = 3333,
      .sum_srtt = 120'000'000ULL,
      .sum_delivered = 55,
      .active_rtts = 60,
      .syn_timeouts = 7,
      .new_sockets = 88,
      .tcp_resets = 9};

  send_and_validate(tcp_metrics, 1652901822111111111ns, std::move(labels));
}

TEST_F(OtlpGrpcFormatterTest, ValidateMetricsRequest)
{
  {
    TsdbFormatter::labels_t labels{
        {"az_equal", "false"},
        {"daz", "ubuntu-focal"},
        {"denv", "ubuntu-focal"},
        {"dns", "ubuntu-focal"},
        {"dprocess", "sshd"},
        {"drole", "sshd"},
        {"dtype", "PROCESS"},
        {"saz", "(unknown)"},
        {"senv", "(no agent)"},
        {"srole", "(unknown)"},
        {"stype", "IP"}};
    u32 value_u32 = 57643;

    send_and_validate("tcp.bytes", value_u32, 1652901824222222222ns, std::move(labels));
  }

  {
    TsdbFormatter::labels_t labels{
        {"az_equal", "false"},
        {"daz", "us-east-1"},
        {"denv", "ubuntu-focal"},
        {"dns", "ubuntu-focal"},
        {"dprocess", "sshd"},
        {"drole", "sshd"},
        {"dtype", "PROCESS"},
        {"saz", "(unknown)"},
        {"senv", "(no agent)"},
        {"srole", "(unknown)"},
        {"stype", "IP"}};
    u64 value_u64 = 34982;

    send_and_validate("tcp.other_metric", value_u64, 1652901827333333333ns, std::move(labels));
  }

  {
    TsdbFormatter::labels_t labels{
        {"az_equal", "false"},
        {"daz", "us-west-1"},
        {"denv", "ubuntu-focal"},
        {"dns", "ubuntu-focal"},
        {"dprocess", "sshd"},
        {"drole", "sshd"},
        {"dtype", "PROCESS"},
        {"saz", "(unknown)"},
        {"senv", "(no agent)"},
        {"srole", "(unknown)"},
        {"stype", "IP"}};
    double value_double = 3.1415921234567889;

    send_and_validate("tcp.rtt_average", value_double, 1652901830444444444ns, std::move(labels));
  }
}

} // namespace reducer