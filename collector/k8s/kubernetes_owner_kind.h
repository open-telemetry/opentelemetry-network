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

#include "platform/types.h"
#include <string>

namespace collector {
enum class KubernetesOwnerKind : u8 {
  ReplicaSet = 0,
  Deployment = 1,
  // TODO: fill in more as we go.
  //
  NoOwner = 254,
  Other = 255
};

KubernetesOwnerKind KubernetesOwnerKindFromString(const std::string &str);
const char *KubernetesOwnerKindToString(const KubernetesOwnerKind kind);

bool KubernetesOwnerIsDeployment(const std::string &str);
bool KubernetesOwnerIsReplicaSet(const std::string &str);
bool KubernetesOwnerIsNoOwner(const std::string &str);

} // namespace collector
