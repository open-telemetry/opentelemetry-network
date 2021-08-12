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

#pragma once

#include <collector/kernel/probe_handler.h>
#include <collector/kernel/process_handler_per_pid_data.h>
#include <util/counter_to_rate.h>
#include <util/enum.h>
#include <util/gauge.h>
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

#define ENUM_NAME ProcPath
#define ENUM_TYPE std::uint8_t
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(stat, 0)                                                                                                                   \
  X(io, 1)                                                                                                                     \
  X(status, 2)
#include <util/enum_operators.inl>

class ProcessHandler {
public:
  struct CpuMemIoSettings {
    // minimum and maximum cpu/mem/io batch size
    std::size_t min_batch_size;
    std::size_t max_batch_size;
    // maximum time budget per cpu/mem/io micro batch
    StopWatch<>::duration poll_budget;
    // poll count between cpu/mem/io batch chunks
    std::size_t batch_poll_count_cooldown;
  };

  ProcessHandler(
      ::flowmill::ingest::Writer &writer,
      ::flowmill::kernel_collector::Index &collector_index,
      ProbeHandler &probe_handler,
      ebpf::BPFModule &bpf_module,
      logging::Logger &log,
      // if null, disables cpu/mem/io collection
      CpuMemIoSettings const *cpu_mem_io_settings);

  ~ProcessHandler();

  void poll();
  void slow_poll(u64 timestamp);

  void on_new_process(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_info const &msg);

  void on_process_end(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_close const &msg);

  void on_cgroup_move(std::chrono::nanoseconds timestamp, struct jb_agent_internal__cgroup_attach_task const &msg);

  void set_process_command(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_set_comm const &msg);

  void pid_exit(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_exit const &msg);

  void report_task_status(std::chrono::nanoseconds timestamp, struct jb_agent_internal__report_task_status const &msg);

#ifdef DEBUG_TGID
  void debug_tgid_dump();
#endif // DEBUG_TGID

private:
  struct PerPidData {
#define CPU_MEM_IO_DATA_IMPL(Name, Type, Aggregation, ...) Aggregation<Type> Name = {};
    PROCESS_HANDLER_CPU_MEM_IO_FIELDS(CPU_MEM_IO_DATA_IMPL)
#undef CPU_MEM_IO_DATA_IMPL

    template <typename RHS> PerPidData &operator+=(RHS &&rhs)
    {
#define CPU_MEM_IO_DATA_IMPL(Name, ...) Name += rhs.Name;
      PROCESS_HANDLER_CPU_MEM_IO_FIELDS(CPU_MEM_IO_DATA_IMPL)
#undef CPU_MEM_IO_DATA_IMPL
      return *this;
    }
  };

  struct PerTgidData {
#define CPU_MEM_IO_DATA_IMPL(Name, Type, ...) Type Name = {};
    PROCESS_HANDLER_CPU_MEM_IO_FIELDS(CPU_MEM_IO_DATA_IMPL)
#undef CPU_MEM_IO_DATA_IMPL

    std::chrono::nanoseconds timestamp = {};
    bool pending() const { return static_cast<bool>(timestamp.count()); }

    void reset()
    {
#define CPU_MEM_IO_DATA_IMPL(Name, Type, ...) Name = Type{0};
      PROCESS_HANDLER_CPU_MEM_IO_FIELDS(CPU_MEM_IO_DATA_IMPL)
#undef CPU_MEM_IO_DATA_IMPL

      timestamp = std::chrono::nanoseconds::zero();
    }

    void update(std::chrono::nanoseconds t, PerPidData &rhs, bool on_exit)
    {
#define CPU_MEM_IO_DATA_IMPL(Name, ...) update_field(Name, rhs.Name, on_exit);
      PROCESS_HANDLER_CPU_MEM_IO_FIELDS(CPU_MEM_IO_DATA_IMPL)
#undef CPU_MEM_IO_DATA_IMPL
      timestamp = t;
    }

    template <typename T> static void update_field(T &field, data::CounterToRate<T> &rhs, bool on_exit)
    {
      field += rhs.commit_rate(!on_exit);
    }

    template <typename T> static void update_field(T &field, data::Gauge<T> &rhs, bool)
    {
      if (rhs.max() > field) {
        field = rhs.max();
      }
      rhs.reset();
    }
  };

  struct ThreadGroupData {
    flowmill::kernel_collector::handles::tracked_process handle;
    enum_traits<ProcPath>::array_map<std::string> proc_path;
    absl::flat_hash_map<u32, PerPidData> by_pid;
    PerTgidData by_tgid;
    monotonic_clock::time_point last_check;
#ifdef DEBUG_TGID
    std::chrono::nanoseconds timestamp;
    u64 cgroup;
    std::string command;
#endif // DEBUG_TGID

    monotonic_clock::duration check_interval()
    {
      auto const now = monotonic_clock::now();
      auto const interval = now - last_check;
      last_check = now;
      return interval;
    }
  };

  class PidBatch {
  public:
    // pushes given tgid to the queue
    void push(u32 tgid);

    // removes and returns the tgid element in the queue
    u32 pop();

    // removes the given tgid from the queue
    void erase(u32 tgid);

    // updates internal counters for micro batch duration
    void update(StopWatch<>::duration elapsed, std::size_t count);

    // prepares the next batch to be sent
    // NIT: would be nice to not depend on the map used by the handler
    void reset(absl::flat_hash_map<u32, ThreadGroupData> const &processes);

    std::size_t min_batch_size() const { return min_batch_size_; }
    std::size_t max_batch_size() const { return max_batch_size_; }
    std::size_t total_batch_size() const { return total_batch_size_; }
    std::size_t batch_count() const { return batch_count_; }

    StopWatch<>::duration period() const { return period_.elapsed(); }
    StopWatch<>::duration elapsed() const { return elapsed_; }

    std::size_t size() const { return batch_.size(); }
    bool empty() const { return batch_.empty(); }

  private:
    absl::flat_hash_set<u32> batch_;
    std::size_t min_batch_size_ = 0;
    std::size_t max_batch_size_ = 0;
    std::size_t total_batch_size_ = 0;
    std::size_t batch_count_ = 0;
    StopWatch<>::duration elapsed_ = StopWatch<>::duration::zero();
    StopWatch<> period_;
  };

  // tracks cool down between batch chunks
  // will sit out `off` polls every `on` polls
  struct OnOffTracker {
    explicit OnOffTracker(std::size_t count) : count_(count) {}

    // decrements the remaining counter
    // if remaining counter is over, flips the on/off bit, sets counter to
    // `tock_counr` and returns true; otherwise, return false.
    bool tick(std::size_t tock_count);

    std::size_t remaining() const { return count_; }
    bool has_remaining() const { return count_ > 0; }

    bool on() const { return on_; }
    bool off() const { return !on_; }

  private:
    std::size_t count_;
    bool on_ = true;
  };

  void poll_cpu_mem_io();
  void submit_metrics(u64 timestamp);

  template <typename View, ProcPath Which> View parse_file(u32 tgid, ThreadGroupData &process);

  // rpc components
  ::flowmill::ingest::Writer &writer_;
  ::flowmill::kernel_collector::Index &collector_index_;

  // process data
  absl::flat_hash_map<u32, ThreadGroupData> processes_;
  PidBatch batch_;
  std::optional<OnOffTracker> batch_tracker_;

  // pre-allocated buffer used for `/proc` parsing
  std::vector<std::uint8_t> buffer_;

  // logging
  logging::Logger &log_;

  // feature flags
  CpuMemIoSettings const *cpu_mem_io_settings_;

  std::size_t memory_page_bytes_;

  enum_traits<ProcPath>::array_map<std::size_t> successful_reads_ = {0};
  enum_traits<ProcPath>::array_map<std::size_t> failed_reads_ = {0};
  enum_traits<ProcPath>::array_map<std::size_t> failed_parse_ = {0};

  enum_traits<ProcPath>::array_map<StopWatch<>> proc_blob_send_cooldown_ = {};

  struct InternalStats {
    std::size_t duplicate_tgid = 0;
    std::size_t missing_tgid = 0;
  } stats_;
};
