/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <reducer/constants.h>

#include <jitbuf/jb.h>

#include <generated/ebpf_net/ingest/weak_refs.h>

namespace reducer::ingest {

// Utility for sending container info updates to the matching core.
// Updates will be sent only when container information changes.
//
class ContainerUpdater {
public:
  // Send container info update if the container information has changed since
  // the last time this function was called.
  // flow -- reference to the maching core's flow span
  // container -- span that holds the container information
  void
  send_container_info(ebpf_net::ingest::weak_refs::flow flow, ebpf_net::ingest::weak_refs::container container, FlowSide side)
  {
    if (!container.valid()) {
      return;
    }

    if ((last_container_loc_ == container.loc()) && (last_update_count_ == container.update_count())) {
      // No change.
      return;
    }

    last_container_loc_ = container.loc();
    last_update_count_ = container.update_count();

    // NOTE: cgroup->container reference is created unconditionally, even when a
    //       cgroup is not functioning as a container. We treat a container span
    //       as initialized after it has been updated at least once.
    if (container.update_count() == 0) {
      return;
    }

    flow.container_info(
        (u8)side,
        jb_blob(container.name()),
        jb_blob(container.pod_name()),
        jb_blob(container.role()),
        jb_blob(container.version()),
        jb_blob(container.ns()),
        container.node_type());
  }

private:
  using loc_t = ebpf_net::ingest::weak_refs::container::location_type;
  using count_t = ebpf_net::ingest::weak_refs::container::update_count_t;

  loc_t last_container_loc_ = ebpf_net::ingest::weak_refs::container::invalid;
  count_t last_update_count_ = 0;
};

} // namespace reducer::ingest
