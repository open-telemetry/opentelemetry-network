/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/constants.h>

#include <generated/ebpf_net/matching/span_base.h>

namespace reducer::matching {

class K8sContainerSpan : public ::ebpf_net::matching::K8sContainerSpanBase {
public:
  K8sContainerSpan();
  ~K8sContainerSpan();

  void set_container_pod(
      ::ebpf_net::matching::weak_refs::k8s_container span_ref, u64 timestamp, jsrv_matching__set_container_pod *msg);
};

} // namespace reducer::matching
