// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/matching/k8s_container_span.h>

#include <reducer/ingest/agent_span.h>
#include <reducer/matching/matching_core.h>
#include <reducer/util/docker_image.h>
#include <util/log.h>

#include <generated/ebpf_net/matching/containers.inl>
#include <generated/ebpf_net/matching/index.h>
#include <generated/ebpf_net/matching/keys.h>
#include <generated/ebpf_net/matching/modifiers.h>
#include <generated/ebpf_net/matching/spans.h>
#include <generated/ebpf_net/matching/weak_refs.h>
#include <generated/ebpf_net/matching/weak_refs.inl>

#include <string>

namespace reducer::matching {

K8sContainerSpan::K8sContainerSpan() {}

K8sContainerSpan::~K8sContainerSpan() {}

void K8sContainerSpan::set_container_pod(
    ::ebpf_net::matching::weak_refs::k8s_container span_ref, u64 timestamp, jsrv_matching__set_container_pod *msg)
{
  auto &index = span_ref.index();

  ::ebpf_net::matching::keys::k8s_pod k8s_pod_key;
  k8s_pod_key.uid_hash = msg->pod_uid_hash;
  static_assert(sizeof(k8s_pod_key.uid_suffix) == sizeof(msg->pod_uid_suffix));
  std::copy_n(std::begin(msg->pod_uid_suffix), k8s_pod_key.uid_suffix.max_size(), std::begin(k8s_pod_key.uid_suffix));

  LOG::trace_in(
      NodeResolutionType::K8S_CONTAINER,
      "matching::K8sContainerSpan::set_container_pod: looking up pod_u64={} "
      "pod_id_suffix='{}' container_u64={} container_id_suffix='{}'",
      k8s_pod_key.uid_hash,
      std::string_view((char *)k8s_pod_key.uid_suffix.data(), k8s_pod_key.uid_suffix.size()),
      span_ref.uid_hash(),
      std::string_view((char *)span_ref.uid_suffix().data(), span_ref.uid_suffix().size()));

  // get the owning k8s_pod
  auto k8s_pod = index.k8s_pod.by_key(k8s_pod_key);
  if (!k8s_pod.valid()) {
    LOG::trace_in(
        NodeResolutionType::K8S_CONTAINER,
        "matching::K8sContainerSpan::set_container_pod: failed to"
        " reference a pod: uid_suffix='{}'",
        std::string_view((char *)k8s_pod_key.uid_suffix.data(), k8s_pod_key.uid_suffix.size()));
    local_core<MatchingCore>().logger().k8s_container_pod_not_found(msg->pod_uid_suffix, msg->pod_uid_hash);
    return;
  }

  using name_t = ::ebpf_net::matching::modifiers::k8s_container::name_t;
  using version_t = ::ebpf_net::matching::modifiers::k8s_container::version_t;

  name_t const name{msg->name.buf, std::min((size_t)msg->name.len, name_t::max_len)};
  DockerImageMetadata const image_metadata{msg->image.string_view()};
  version_t const version{image_metadata.version().data(), std::min(image_metadata.version().size(), version_t::max_len)};

  span_ref.modify().pod(std::move(k8s_pod)).name(name).version(version);
}

} // namespace reducer::matching
