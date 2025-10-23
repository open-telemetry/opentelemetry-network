// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "process_span.h"

#include <reducer/ingest/component.h>
#include <reducer/ingest/shared_state.h>

#include <generated/ebpf_net/ingest/modifiers.h>

#include <util/log.h>

#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>

namespace reducer::ingest {

namespace {

constexpr std::string_view RUNIT_SERVICE_PROG_NAME = "runsv";

} // namespace

ProcessSpan::ProcessSpan() {}

ProcessSpan::~ProcessSpan() {}

void ProcessSpan::pid_info(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_info *msg)
{
  std::string comm{(char const *)msg->comm, strnlen((char const *)msg->comm, sizeof(msg->comm))};

  LOG::trace_in(Component::process, "ProcessSpan::pid_info pid:{} comm:{}", msg->pid, comm);

  create_refs(span_ref, msg->pid, std::nullopt, std::move(comm), std::nullopt);
}

void ProcessSpan::pid_info_create_deprecated(
    ::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_info_create_deprecated *msg)
{
  std::string comm{(char const *)msg->comm, strnlen((char const *)msg->comm, sizeof(msg->comm))};

  LOG::trace_in(
      Component::process, "ProcessSpan::pid_info_create_deprecated pid:{} cgroup:{} comm:{}", msg->pid, msg->cgroup, comm);

  create_refs(span_ref, msg->pid, msg->cgroup, std::move(comm), std::nullopt);
}

void ProcessSpan::pid_info_create(
    ::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_info_create *msg)
{
  std::string comm{(char const *)msg->comm, strnlen((char const *)msg->comm, sizeof(msg->comm))};

  LOG::trace_in(
      Component::process,
      "ProcessSpan::pid_info_create pid:{} cgroup:{} comm:{} cmdline:{}",
      msg->pid,
      msg->cgroup,
      comm,
      msg->cmdline);

  create_refs(span_ref, msg->pid, msg->cgroup, std::move(comm), msg->parent_pid);

  parse_cmdline(span_ref, msg->cmdline.string_view());
}

void ProcessSpan::parse_cmdline(::ebpf_net::ingest::weak_refs::process span_ref, std::string_view cmdline)
{
  if (cmdline.size() == 0) {
    return;
  }

  if (auto sep = cmdline.find('\0'); sep != std::string_view::npos) {
    auto prog = cmdline.substr(0, sep);
    auto args = cmdline.substr(sep + 1);

    std::string_view arg1;

    if (auto sep2 = args.find('\0'); sep2 != std::string_view::npos) {
      arg1 = args.substr(0, sep2);
    } else {
      arg1 = args;
    }

    if ((prog == RUNIT_SERVICE_PROG_NAME) && !arg1.empty()) {
      auto service_name = arg1;

      auto service = span_ref.index().service.by_key({{short_string_behavior::truncate, service_name}});

      if (!service.valid()) {
        LOG::error("ProcessSpan: could not allocate a service span");
        return;
      }

      span_ref.modify().service(std::move(service));
    }
  }
}

void ProcessSpan::create_refs(
    ::ebpf_net::ingest::weak_refs::process span_ref,
    u32 pid,
    const std::optional<u64> &cgroup,
    std::string comm,
    const std::optional<s32> &parent_pid)
{
  auto *conn = local_connection()->ingest_connection();

  // TODO: make sure comm length is bounded
  span_ref.modify().comm({comm.c_str(), comm.size()});

  if (cgroup) {
    if (auto cgroup_span = conn->get_cgroup(*cgroup); cgroup_span.valid()) {
      span_ref.modify().cgroup(cgroup_span.get());
    }
  }

  if (parent_pid.has_value() && (parent_pid.value() >= 0)) {
    if (auto parent_span = conn->get_process(*parent_pid); parent_span.valid()) {
      if (parent_span.service().valid()) {
        // Inherit parent's service.
        span_ref.modify().service(parent_span.service().get());
      }
    }
  }
}

void ProcessSpan::pid_cgroup_move(
    ::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_cgroup_move *msg)
{
  auto *conn = local_connection()->ingest_connection();

  LOG::trace_in(Component::process, "ProcessSpan::pid_cgroup_move pid:{} cgroup:{}", msg->pid, msg->cgroup);

  auto cgroup_span = conn->get_cgroup(msg->cgroup);
  if (!cgroup_span.valid()) {
    local_logger().cgroup_not_found(msg->cgroup);
    return;
  }

  span_ref.modify().cgroup(cgroup_span.get());
}

void ProcessSpan::pid_set_comm(::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_set_comm *msg)
{
  std::string comm{(char const *)msg->comm, strnlen((char const *)msg->comm, sizeof(msg->comm))};

  LOG::trace_in(Component::process, "ProcessSpan::pid_set_comm pid:{}, comm:{}", msg->pid, comm);

  span_ref.modify().comm({comm.c_str(), comm.size()});
}

void ProcessSpan::pid_set_cmdline(
    ::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_set_cmdline *msg)
{
  LOG::trace_in(Component::process, "ProcessSpan::pid_set_cmdline pid:{}", msg->pid);

  parse_cmdline(span_ref, msg->cmdline.string_view());
}

void ProcessSpan::pid_close_info(
    ::ebpf_net::ingest::weak_refs::process span_ref, u64 timestamp, jsrv_ingest__pid_close_info *msg)
{
  LOG::trace_in(Component::process, "ProcessSpan::pid_close_info pid:{}", msg->pid);
}

} // namespace reducer::ingest
