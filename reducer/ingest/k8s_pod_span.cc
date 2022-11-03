// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "k8s_pod_span.h"

#include <reducer/ingest/component.h>
#include <reducer/ingest/shared_state.h>

#include <reducer/uid_key.h>

#include <generated/ebpf_net/ingest/modifiers.h>

#include <util/log.h>

namespace reducer::ingest {

K8sPodSpan::~K8sPodSpan()
{
  for (auto &container_handle : containers_) {
    container_handle.put(*local_index());
  }
}

void K8sPodSpan::pod_container(
    ::ebpf_net::ingest::weak_refs::k8s_pod span,
    std::string_view container_id,
    std::string_view container_name,
    std::string_view container_image)
{
  static constexpr char const *sep = "://";

  LOG::trace_in(
      Component::k8s_pod,
      "K8sPodSpan::pod_container: pod_uid_suffix={} container_id={}",
      std::string_view((const char *)span.uid_suffix().data(), span.uid_suffix().size()),
      container_id);

  // Kubernetes prefixes container IDs with the container engine type,
  // e.g. "docker://" or "containerd://".
  auto sep_pos = container_id.find(sep);
  if (sep_pos != std::string::npos) {
    container_id = container_id.substr(sep_pos + strlen(sep));
  }

  if (auto [_, inserted] = seen_container_ids_.insert(std::string(container_id)); inserted == false) {
    LOG::trace_in(Component::k8s_pod, "K8sPodSpan::pod_container: container already reported: {}", container_id);
    return;
  }

  auto container_key = make_uid_key<ebpf_net::ingest::keys::k8s_container>(container_id);
  auto k8s_container = local_index()->k8s_container.by_key(container_key);
  if (!k8s_container.valid()) {
    LOG::trace_in(Component::k8s_pod, "K8sPodSpan::pod_container: failed to reference container {}", container_id);
    return;
  }

  k8s_container.set_container_pod(span.uid_suffix().data(), span.uid_hash(), jb_blob(container_name), jb_blob(container_image));

  containers_.push_back(k8s_container.to_handle());
}

} // namespace reducer::ingest
