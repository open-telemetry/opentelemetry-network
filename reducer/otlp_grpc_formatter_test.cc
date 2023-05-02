// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "otlp_grpc_formatter.h"
#include "publisher.h"

#include <otlp/otlp_util.h>
#include <util/common_test.h>
#include <util/json.h>
#include <util/overloaded_visitor.h>
#include <util/time.h>

#include <string>

namespace reducer {

class TestPublisherWriter : public Publisher::Writer {
public:
  TestPublisherWriter(ExportMetricsServiceRequest &request_to_validate) : request_to_validate_(request_to_validate){};

  TestPublisherWriter(TestPublisherWriter const &) = delete;
  TestPublisherWriter(TestPublisherWriter &&) = default;

  ~TestPublisherWriter(){};

  void write(ExportLogsServiceRequest &request) override{};
  void write(ExportMetricsServiceRequest &request) override { request_to_validate_ = request; };

  void flush() override{};

private:
  ExportMetricsServiceRequest &request_to_validate_;
};

class OtlpGrpcFormatterTest : public CommonTest {
protected:
  void SetUp() override { CommonTest::SetUp(); }

  void validate_request(std::string_view name, TsdbFormatter::value_t metric_value, TsdbFormatter::timestamp_t timestamp)
  {
    auto request_json_str = get_request_json(request_to_validate_);
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
      for (auto const &rm : request_json.at("resourceMetrics")) {
        for (auto const &sm : rm.at("scopeMetrics")) {
          for (auto const &metric : sm.at("metrics")) {
            EXPECT_EQ(name, metric.at("name"));
            auto const &sum = metric.at("sum");
            EXPECT_EQ("AGGREGATION_TEMPORALITY_DELTA", sum.at("aggregationTemporality"));
            EXPECT_TRUE(sum.at("isMonotonic"));
            for (auto const &data_point : sum.at("dataPoints")) {
              std::visit(
                  overloaded_visitor{
                      [&](u32 val) { EXPECT_EQ(val, stoul(std::string(data_point.at("asInt")))); },
                      [&](u64 val) { EXPECT_EQ(val, stoull(std::string(data_point.at("asInt")))); },
                      [&](double val) { EXPECT_EQ(val, data_point.at("asDouble")); }},
                  metric_value);
              EXPECT_EQ(
                  integer_time<std::chrono::nanoseconds>(timestamp), std::stoull(std::string(data_point.at("timeUnixNano"))));
              for (auto const &attribute : data_point.at("attributes")) {
                auto key = attribute.at("key");
                auto value = attribute.at("value").at("stringValue");
                EXPECT_EQ(labels_to_validate.count(key), 1);
                EXPECT_EQ(labels_to_validate.at(key), value);
                labels_to_validate.erase(key);
              }
              EXPECT_EQ(labels_to_validate.size(), 0);
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
  ExportMetricsServiceRequest request_to_validate_;
};

TEST_F(OtlpGrpcFormatterTest, ValidateRequest)
{
  writer_ = std::make_unique<TestPublisherWriter>(request_to_validate_);
  formatter_ = TsdbFormatter::make(TsdbFormat::otlp_grpc, writer_);

  auto send_and_validate = [&](std::string_view name,
                               TsdbFormatter::value_t value,
                               TsdbFormatter::timestamp_t timestamp,
                               TsdbFormatter::labels_t labels) {
    formatter_->set_aggregation("az_az");
    formatter_->set_labels(std::move(labels));
    formatter_->set_timestamp(timestamp);
    formatter_->write(name, value, writer_);
    formatter_->flush();

    validate_request(name, value, timestamp);
  };

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
    send_and_validate("tcp.bytes", value_u32, 1652901824111111111ns, std::move(labels));
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
    send_and_validate("tcp.other_metric", value_u64, 1652901827222222222ns, std::move(labels));
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
    send_and_validate("tcp.rtt_average", value_double, 1652901830333333333ns, std::move(labels));
  }
}

} // namespace reducer
