/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/log.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

class CommonTest : public ::testing::Test {
public:
  virtual void SetUp() override
  {
    LOG::init(true); // log to console

    // Default to debug logging
    spdlog::set_level(spdlog::level::debug);
    // Override level with SPDLOG_LEVEL env var if set
    spdlog::cfg::load_env_levels();
  }
};
