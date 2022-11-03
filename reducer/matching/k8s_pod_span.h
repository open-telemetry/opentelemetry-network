/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/constants.h>

#include <generated/ebpf_net/matching/modifiers.h>
#include <generated/ebpf_net/matching/span_base.h>

#include <util/ip_address.h>

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <tuple>

namespace reducer::matching {

class K8sPodSpan : public ::ebpf_net::matching::K8sPodSpanBase {
public:
  K8sPodSpan();
  ~K8sPodSpan();

  // store pod detail into the span
  void set_pod_detail(::ebpf_net::matching::weak_refs::k8s_pod span_ref, u64 timestamp, jsrv_matching__set_pod_detail *msg);
};

} // namespace reducer::matching
