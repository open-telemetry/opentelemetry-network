//
// Copyright 2021 Splunk Inc.
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

#include <util/enum.h>

#define ENUM_NAME ClientType
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(kernel, 1)                                                                                                                 \
  X(aws, 2)                                                                                                                    \
  X(k8s, 3)                                                                                                                    \
  X(ingest, 4)                                                                                                                 \
  X(matching, 5)                                                                                                               \
  X(aggregation, 6)                                                                                                            \
  X(liveness_probe, 7)                                                                                                         \
  X(readiness_probe, 8)
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
