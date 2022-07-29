/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
