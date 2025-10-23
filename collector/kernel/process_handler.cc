// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/process_handler.h>

#include <collector/agent_log.h>
#include <common/collected_blob_type.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/system_ops.h>

#include <generated/ebpf_net/agent_internal/wire_message.h>

#include <cstring>
#include <string_view>

#include <cassert>

namespace {

std::string_view comm_to_string(std::uint8_t const (&comm)[16])
{
  auto length = strnlen(reinterpret_cast<char const *>(comm), sizeof(comm));
  return std::string_view(reinterpret_cast<char const *>(comm), length);
}

} // namespace

ProcessHandler::ProcessHandler(
    ::ebpf_net::ingest::Writer &writer, ::ebpf_net::kernel_collector::Index &collector_index, logging::Logger &log)
    : writer_(writer), collector_index_(collector_index), log_(log), memory_page_bytes_(memory_page_size().try_raise().value())
{
  LOG::trace_in(
      AgentLogKind::TRACKED_PROCESS,
      "ProcessHandler: memory_page_size={} clock_ticks_per_second={}",
      memory_page_bytes_,
      clock_ticks_per_second);

  writer_.system_wide_process_settings(clock_ticks_per_second, memory_page_bytes_);
}

ProcessHandler::~ProcessHandler()
{
  for (auto &process : processes_) {
    process.second.handle.put(collector_index_);
  }

  processes_.clear();
}

void ProcessHandler::on_new_process(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_info const &msg)
{
  const std::string_view comm = comm_to_string(msg.comm);
  LOG::trace_in(
      AgentLogKind::PID,
      "ProcessHandler::{}: timestamp={} pid={} parent={} cgroup=0x{:x} comm='{}'",
      __func__,
      timestamp,
      msg.pid,
      msg.parent_pid,
      msg.cgroup,
      comm);

  if (processes_.find(msg.pid) != processes_.end()) {
    ++stats_.duplicate_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(
        AgentLogKind::TRACKED_PROCESS,
        "DUPLICATE TGID pid={} parent={} cgroup=0x{:x} comm='{}'!",
        msg.pid,
        msg.parent_pid,
        msg.cgroup,
        comm);
#endif // DEBUG_TGID
  }

  auto weak_handle = collector_index_.tracked_process.by_key({msg.cgroup, msg.pid});
  assert(msg.pid == weak_handle.tgid());
  assert(msg.cgroup == weak_handle.cgroup());

  weak_handle.set_tgid(msg.pid);
  weak_handle.set_command(to_jb_blob(msg.comm));
  weak_handle.set_cgroup(msg.cgroup);

  processes_[msg.pid] = {
      .handle = weak_handle.to_handle()
#ifdef DEBUG_TGID
          ,
      .timestamp = timestamp,
      .cgroup = msg.cgroup,
      .command = std::string(comm)
#endif // DEBUG_TGID
  };
}

void ProcessHandler::on_process_end(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_close const &msg)
{
  const std::string_view comm = comm_to_string(msg.comm);
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} pid={} comm='{}'", __func__, timestamp, msg.pid, comm);

  if (auto i = processes_.find(msg.pid); i != processes_.end()) {
    i->second.handle.put(collector_index_);
    processes_.erase(i);
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID pid={} comm='{}'", msg.pid, comm);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::on_cgroup_move(std::chrono::nanoseconds timestamp, struct jb_agent_internal__cgroup_attach_task const &msg)
{
  LOG::trace_in(
      AgentLogKind::PID, "ProcessHandler::{}: timestamp={} pid={} cgroup=0x{:x}", __func__, timestamp, msg.pid, msg.cgroup);

  if (auto i = processes_.find(msg.pid); i != processes_.end()) {
    i->second.handle.access(collector_index_).set_cgroup(msg.cgroup);
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID pid={} cgroup=0x{:x}", msg.pid, msg.cgroup);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::set_process_command(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_set_comm const &msg)
{
  const std::string_view comm = comm_to_string(msg.comm);
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} pid={} comm='{}'", __func__, timestamp, msg.pid, comm);

  if (auto i = processes_.find(msg.pid); i != processes_.end()) {
    i->second.handle.access(collector_index_).set_command(to_jb_blob(msg.comm));
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID pid={} comm='{}'", msg.pid, comm);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::pid_exit(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_exit const &msg)
{
  LOG::trace_in(
      AgentLogKind::PID,
      "ProcessHandler::{}: timestamp={} tgid={} pid={} exit_code={}",
      __func__,
      timestamp,
      msg.tgid,
      msg.pid,
      msg.exit_code);

  if (auto i = processes_.find(msg.tgid); i != processes_.end()) {
    i->second.handle.access(collector_index_)
        .pid_exit_tstamp(integer_time<std::chrono::nanoseconds>(timestamp), msg.tgid, msg.pid, msg.exit_code);
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID tgid={} pid={} exit_code={}", msg.tgid, msg.pid, msg.exit_code);
#endif // DEBUG_TGID
  }
}

#ifdef DEBUG_TGID
void ProcessHandler::debug_tgid_dump()
{
  auto const str = [this] {
    std::ostringstream dump;
    dump << "-- BEGIN TGID DUMP --\n";

    for (auto const &i : processes_) {
      auto &process = i.second;
      dump << fmt::format(
          "tstamp: {}, tgid: {}, cgroup: {}, comm: {}\n", process.timestamp, i.first, process.cgroup, process.command);
    }

    dump << "-- END TGID DUMP --\n";
    return dump.str();
  }();

  LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "{}", str);
}
#endif // DEBUG_TGID
