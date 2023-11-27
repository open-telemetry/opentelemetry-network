// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "labels.h"

#include <reducer/constants.h>

#define FOREACH_NODE_LABEL(FUNC)                                                                                               \
  FUNC("workload.name", role)                                                                                                  \
  FUNC("workload.uid", role_uid)                                                                                               \
  FUNC("availability_zone", az)                                                                                                \
  FUNC("id", id)                                                                                                               \
  FUNC("ip", ip)                                                                                                               \
  FUNC("resolution_type", type)                                                                                                \
  FUNC("image_version", version)                                                                                               \
  FUNC("environment", env)                                                                                                     \
  FUNC("namespace.name", ns)                                                                                                   \
  FUNC("process.name", process)                                                                                                \
  FUNC("container.name", container)                                                                                            \
  FUNC("pod", pod)

namespace reducer::aggregation {

inline NodeLabels::NodeLabels() {}

inline NodeLabels::NodeLabels(::ebpf_net::aggregation::weak_refs::node node_ref) : NodeLabels(node_ref.az())
{
  id = node_ref.id().to_string();
  ip = node_ref.ip().to_string();
  pod = node_ref.pod_name().to_string();
}

inline NodeLabels::NodeLabels(::ebpf_net::aggregation::weak_refs::az az_ref) : NodeLabels(az_ref.role())
{
  az = az_ref.s().to_string();
}

inline NodeLabels::NodeLabels(::ebpf_net::aggregation::weak_refs::role role_ref)
{
  role = role_ref.s().to_string();
  role_uid = role_ref.uid().to_string();
  version = role_ref.version().to_string();
  env = role_ref.env().to_string();
  ns = role_ref.ns().to_string();
  type = to_string(static_cast<NodeResolutionType>(role_ref.node_type()), "");
  process = role_ref.process().to_string();
  container = role_ref.container().to_string();
}

inline void NodeLabels::foreach (std::function<void(std::string_view, std::string_view)> func) const
{
#define CALL_FUNC(NAME, VALUE) func(NAME, VALUE);
  FOREACH_NODE_LABEL(CALL_FUNC);
#undef CALL_FUNC
}

inline void FlowLabels::foreach (std::function<void(std::string_view, std::string_view)> func) const
{
#define CALL_FUNC_SRC(NAME, VALUE) func("source." NAME, src.VALUE);
#define CALL_FUNC_DST(NAME, VALUE) func("dest." NAME, dst.VALUE);

  FOREACH_NODE_LABEL(CALL_FUNC_SRC);
  FOREACH_NODE_LABEL(CALL_FUNC_DST);

#undef CALL_FUNC_SRC
#undef CALL_FUNC_DST

  // Equality label for the az field.
  // Needed for queriying cross-zone traffic.
  //
  if (!src.az.empty() && !dst.az.empty()) {
    func("az_equal", (src.az == dst.az) ? "true" : "false");
  }
}

inline bool operator==(NodeLabels const &lhs, NodeLabels const &rhs)
{
#define EQL(NAME, VALUE) &&(lhs.VALUE == rhs.VALUE)
  return true FOREACH_NODE_LABEL(EQL);
#undef EQL
}

template <typename Hash> Hash AbslHashValue(Hash hash, NodeLabels const &labels)
{
#define LVAL(NAME, VALUE) , labels.VALUE
  return Hash::combine(std::move(hash) FOREACH_NODE_LABEL(LVAL));
#undef LVAL
}

inline bool operator==(FlowLabels const &lhs, FlowLabels const &rhs)
{
  return (lhs.src == rhs.src) && (lhs.dst == rhs.dst);
}

template <typename Hash> Hash AbslHashValue(Hash hash, FlowLabels const &labels)
{
  return Hash::combine(std::move(hash), labels.src, labels.dst);
}

} // namespace reducer::aggregation

#undef FOREACH_NODE_LABEL
