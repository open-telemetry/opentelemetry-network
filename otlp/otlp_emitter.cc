// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "otlp_emitter.h"

namespace otlp_client {
OtlpEmitter::OtlpEmitter(const std::string &uri) : client_(grpc::CreateChannel(uri, grpc::InsecureChannelCredentials())) {}

bool OtlpEmitter::operator()(const ExportMetricsServiceRequest &request)
{
  grpc::Status status = client_.Export(request);
  if (!status.ok()) {
    std::cerr << "FAILED to emit metric. client export failed." << std::endl;
    return false;
  }

  return true;
}
} // namespace otlp_client