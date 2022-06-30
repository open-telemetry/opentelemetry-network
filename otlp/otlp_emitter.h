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