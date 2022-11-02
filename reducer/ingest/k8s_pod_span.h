/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/ingest/handles.h>
#include <generated/ebpf_net/ingest/span_base.h>
#include <generated/ebpf_net/ingest/weak_refs.h>

#include <set>
#include <vector>

namespace reducer::ingest {

class K8sPodSpan : public ::ebpf_net::ingest::K8sPodSpanBase {
public:
  K8sPodSpan() = default;
  ~K8sPodSpan();

  void pod_container(
      ::ebpf_net::ingest::weak_refs::k8s_pod span,
      std::string_view container_id,
      std::string_view container_name,
      std::string_view container_tag);

private:
  using k8s_container_handle_t = ::ebpf_net::ingest::handles::k8s_container;

  // Set of container IDs that were reported through |pod_container|.
  std::set<std::string> seen_container_ids_;

  // Container spans that this pod owns.
  std::vector<k8s_container_handle_t> containers_;
};

} // namespace reducer::ingest
