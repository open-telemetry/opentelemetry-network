//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <collector/kernel/process_handler.h>

#include <collector/agent_log.h>
#include <common/collected_blob_type.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/system_ops.h>

#include <generated/flowmill/agent_internal.wire_message.h>

#include <cassert>

ProcessHandler::ProcessHandler(
    ::flowmill::ingest::Writer &writer, ::flowmill::kernel_collector::Index &collector_index, logging::Logger &log)
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
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} msg={}", __func__, timestamp, msg);

  if (processes_.find(msg.pid) != processes_.end()) {
    ++stats_.duplicate_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "DUPLICATE TGID {}!", msg);
#endif // DEBUG_TGID
  }

  auto weak_handle = collector_index_.tracked_process.by_key({.cgroup = msg.cgroup, .tgid = msg.pid});
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
      .command = render_array_to_string(msg.comm)
#endif // DEBUG_TGID
  };
}

void ProcessHandler::on_process_end(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_close const &msg)
{
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} msg={}", __func__, timestamp, msg);

  if (auto i = processes_.find(msg.pid); i != processes_.end()) {
    i->second.handle.put(collector_index_);
    processes_.erase(i);
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID {}", msg);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::on_cgroup_move(std::chrono::nanoseconds timestamp, struct jb_agent_internal__cgroup_attach_task const &msg)
{
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} msg={}", __func__, timestamp, msg);

  if (auto i = processes_.find(msg.pid); i != processes_.end()) {
    i->second.handle.access(collector_index_).set_cgroup(msg.cgroup);
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID {}", msg);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::set_process_command(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_set_comm const &msg)
{
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} msg={}", __func__, timestamp, msg);

  if (auto i = processes_.find(msg.pid); i != processes_.end()) {
    i->second.handle.access(collector_index_).set_command(to_jb_blob(msg.comm));
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID {}", msg);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::pid_exit(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_exit const &msg)
{
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} msg={}", __func__, timestamp, msg);

  if (auto i = processes_.find(msg.tgid); i != processes_.end()) {
    i->second.handle.access(collector_index_)
        .pid_exit_tstamp(integer_time<std::chrono::nanoseconds>(timestamp), msg.tgid, msg.pid, msg.exit_code);
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::TRACKED_PROCESS, "MISSING TGID {}", msg);
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
