// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/matching/k8s_pod_span.h>

#include <generated/ebpf_net/matching/modifiers.h>
#include <generated/ebpf_net/matching/spans.h>
#include <util/log.h>

#include <string>

namespace reducer::matching {

K8sPodSpan::K8sPodSpan() {}

K8sPodSpan::~K8sPodSpan() {}

void K8sPodSpan::set_pod_detail(
    ::ebpf_net::matching::weak_refs::k8s_pod span_ref, u64 timestamp, jsrv_matching__set_pod_detail *msg)
{
  span_ref.modify()
      .owner_name({short_string_behavior::truncate, msg->owner_name})
      .owner_uid({short_string_behavior::truncate, msg->owner_uid})
      .pod_name({short_string_behavior::truncate, msg->pod_name})
      .ns({short_string_behavior::truncate, msg->ns})
      .version({short_string_behavior::truncate, msg->version});
}

} // namespace reducer::matching
