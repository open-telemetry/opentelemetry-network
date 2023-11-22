/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/aggregation/weak_refs.h>

#include <functional>
#include <string>
#include <string_view>

namespace reducer::aggregation {

// Set of labels (name/value pairs) that one side of a flow can have.
//
struct NodeLabels {
  std::string id;
  std::string ip;
  std::string az;
  std::string role;
  std::string role_uid;
  std::string version;
  std::string env;
  std::string ns;
  std::string type;
  std::string process;
  std::string container;
  std::string pod;

  // NOTE: if any label is added or removed, the FOREACH_NODE_LABEL macro needs to
  //       be updated.

  NodeLabels();
  NodeLabels(::ebpf_net::aggregation::weak_refs::node node_ref);
  NodeLabels(::ebpf_net::aggregation::weak_refs::az az_ref);
  NodeLabels(::ebpf_net::aggregation::weak_refs::role role_ref);

  void foreach (std::function<void(std::string_view, std::string_view)> func) const;
};

// Set of labels that a flow can have.
//
struct FlowLabels {
  NodeLabels src;
  NodeLabels dst;

  void foreach (std::function<void(std::string_view, std::string_view)> func) const;
};

} // namespace reducer::aggregation

#include "labels.inl"
