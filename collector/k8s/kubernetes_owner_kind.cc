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

#include "kubernetes_owner_kind.h"

namespace collector {

namespace {
static constexpr char REPLICA_SET_KIND[] = "ReplicaSet";
static constexpr char DEPLOYMENT_KIND[] = "Deployment";
static constexpr char NO_OWNER_KIND[] = "NoOwner";
static constexpr char OTHER_KIND[] = "Other";
} // namespace

KubernetesOwnerKind KubernetesOwnerKindFromString(const std::string &str)
{
  if (str == REPLICA_SET_KIND) {
    return KubernetesOwnerKind::ReplicaSet;
  }

  if (str == DEPLOYMENT_KIND) {
    return KubernetesOwnerKind::Deployment;
  }

  if (str == NO_OWNER_KIND) {
    return KubernetesOwnerKind::NoOwner;
  }

  return KubernetesOwnerKind::Other;
}

const char *KubernetesOwnerKindToString(const KubernetesOwnerKind kind)
{
  switch (kind) {
  case KubernetesOwnerKind::ReplicaSet:
    return REPLICA_SET_KIND;
  case KubernetesOwnerKind::Deployment:
    return DEPLOYMENT_KIND;
  case KubernetesOwnerKind::NoOwner:
    return NO_OWNER_KIND;

  default:
    return OTHER_KIND;
  }
}

bool KubernetesOwnerIsDeployment(const std::string &str)
{
  return str == DEPLOYMENT_KIND;
}

bool KubernetesOwnerIsReplicaSet(const std::string &str)
{
  return str == REPLICA_SET_KIND;
}

bool KubernetesOwnerIsNoOwner(const std::string &str)
{
  return str == NO_OWNER_KIND;
}

} // namespace collector
