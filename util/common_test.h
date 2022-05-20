#pragma once

#include <util/log.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

static constexpr char GTEST_ENABLE_TRACE_LOGGING_VAR[] = "GTEST_ENABLE_TRACE_LOGGING";

class CommonTest : public ::testing::Test {
public:
  virtual void SetUp() override
  {
    LOG::init(true); // log to console

    std::string enable_trace_logging("false");
    if (auto val = std::getenv(GTEST_ENABLE_TRACE_LOGGING_VAR); (val != nullptr) && (strlen(val) > 0)) {
      enable_trace_logging = std::string(val);
    }
    if (enable_trace_logging == "true") {
      spdlog::set_level(spdlog::level::trace);
    } else {
      spdlog::set_level(spdlog::level::debug);
    }
  }
};
