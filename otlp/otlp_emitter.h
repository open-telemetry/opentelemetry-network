/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>

#include "otlp_grpc_metrics_client.h"

namespace otlp_client {

// A small class that will send an ExportMetricsServiceRequest out along
// the identified uri. It is synchronous and is intended for ad-hoc
// metric sending when performance doesn't matter.  It shouldn't be
// used otherwise.
class OtlpEmitter {
public:
  explicit OtlpEmitter(const std::string &uri);

  bool operator()(const ExportMetricsServiceRequest &request);

private:
  OtlpGrpcMetricsClient client_;
};
} // namespace otlp_client