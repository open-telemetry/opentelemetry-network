/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>

namespace reducer::logging {

struct ConnectionMetrics {
  u64 num_connections;

  ConnectionMetrics() : num_connections(0) {}

  ConnectionMetrics const &operator+=(ConnectionMetrics const &other)
  {
    num_connections += other.num_connections;
    return *this;
  }
};

} // namespace reducer::logging
