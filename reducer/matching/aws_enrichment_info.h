/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

struct AwsEnrichmentInfo {
  std::string role;
  std::string az;
  std::string id;
};
