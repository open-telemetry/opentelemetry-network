/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/ingest/handles.h>
#include <generated/ebpf_net/ingest/span_base.h>
#include <generated/ebpf_net/ingest/weak_refs.h>

#include <absl/container/flat_hash_map.h>

#include <util/cgroup_parser.h>

#include <string>

namespace reducer::ingest {

class CgroupSpan : public ::ebpf_net::ingest::CgroupSpanBase {
public:
  CgroupSpan();
  ~CgroupSpan();
  /* deprecated handlers
   */
  void cgroup_create_deprecated(
      ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__cgroup_create_deprecated *msg);

  /* handlers
   */
  void cgroup_create(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__cgroup_create *msg);
  void cgroup_close(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__cgroup_close *msg);
  void container_metadata(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_metadata *msg);
  void
  container_annotation(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_annotation *msg);
  void pod_name(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__pod_name *msg);
  void container_resource_limits_deprecated(
      ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_resource_limits_deprecated *msg);
  void container_resource_limits(
      ::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__container_resource_limits *msg);
  void k8s_metadata(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__k8s_metadata *msg);
  void k8s_metadata_port(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__k8s_metadata_port *msg);
  void nomad_metadata(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 timestamp, jsrv_ingest__nomad_metadata *msg);

private:
  // Called whenever one or more container span fields have been modified.
  void on_container_updated(::ebpf_net::ingest::weak_refs::container container_ref);

  // Assigns the appropriate service span, if this cgroup is created by a
  // service manager.
  void set_service(::ebpf_net::ingest::weak_refs::cgroup span_ref, const CGroupParser &parser);
  // assigns a pod_uid and pod_uid_hash if applicable.
  void set_pod(::ebpf_net::ingest::weak_refs::cgroup span_ref, const CGroupParser &parser);
  // assigns a cgroup parent, if found
  void set_parent(::ebpf_net::ingest::weak_refs::cgroup span_ref, u64 cgroup_parent);
  // assigns a container, if found
  void set_container(::ebpf_net::ingest::weak_refs::cgroup span_ref, const CGroupParser &parser);

  absl::flat_hash_map<std::string, std::string> annotations_;
  std::string k8s_container_name_;
};

} // namespace reducer::ingest
