/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/counter_to_rate.h>
#include <util/logger.h>
#include <util/stop_watch.h>

#include <generated/flowmill/kernel_collector/handles.h>
#include <generated/flowmill/kernel_collector/index.h>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Turn this on to enable tgid table debugging feature
// via: kill -USR1 <agent_pid>
#ifndef NDEBUG
//#define DEBUG_TGID 1
#endif // NDEBUG

class ProcessHandler {
public:
  ProcessHandler(
      ::flowmill::ingest::Writer &writer, ::flowmill::kernel_collector::Index &collector_index, logging::Logger &log);

  ~ProcessHandler();

  void on_new_process(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_info const &msg);

  void on_process_end(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_close const &msg);

  void on_cgroup_move(std::chrono::nanoseconds timestamp, struct jb_agent_internal__cgroup_attach_task const &msg);

  void set_process_command(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_set_comm const &msg);

  void pid_exit(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_exit const &msg);

#ifdef DEBUG_TGID
  void debug_tgid_dump();
#endif // DEBUG_TGID

private:
  struct ThreadGroupData {
    flowmill::kernel_collector::handles::tracked_process handle;
#ifdef DEBUG_TGID
    std::chrono::nanoseconds timestamp;
    u64 cgroup;
    std::string command;
#endif // DEBUG_TGID
  };

  // rpc components
  ::flowmill::ingest::Writer &writer_;
  ::flowmill::kernel_collector::Index &collector_index_;

  // process data
  absl::flat_hash_map<u32, ThreadGroupData> processes_;

  // logging
  logging::Logger &log_;

  std::size_t memory_page_bytes_;

  struct InternalStats {
    std::size_t duplicate_tgid = 0;
    std::size_t missing_tgid = 0;
  } stats_;
};
