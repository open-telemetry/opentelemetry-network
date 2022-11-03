/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <generated/ebpf_net/ingest/handles.h>
#include <generated/ebpf_net/ingest/span_base.h>
#include <generated/ebpf_net/ingest/weak_refs.h>

#include <optional>
#include <string>

namespace reducer::ingest {

class ProcessSpan : public ::ebpf_net::ingest::ProcessSpanBase {
public:
  ProcessSpan();
  ~ProcessSpan();

  /* Deprecated handlers.
   */
  void pid_info(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_info *msg);
  void pid_info_create_deprecated(
      ::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_info_create_deprecated *msg);

  /* Handlers.
   */
  void pid_info_create(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_info_create *msg);
  void pid_cgroup_move(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_cgroup_move *msg);
  void pid_set_comm(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_set_comm *msg);
  void pid_set_cmdline(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_set_cmdline *msg);
  void pid_close_info(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_close_info *msg);

private:
  void create_refs(
      ::ebpf_net::ingest::weak_refs::process span_ref,
      u32 pid,
      const std::optional<u64> &cgroup,
      std::string comm,
      const std::optional<s32> &parent_pid);

  void parse_cmdline(::ebpf_net::ingest::weak_refs::process span_ref, std::string_view cmdline);
};

} // namespace reducer::ingest
