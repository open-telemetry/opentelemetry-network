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
#include <util/file_ops.h>
#include <util/log.h>
#include <util/log_formatters.h>
#include <util/proc_ops.h>
#include <util/system_ops.h>
#include <util/time.h>

#include <generated/flowmill/agent_internal.wire_message.h>

#include <cassert>

constexpr auto PROC_BLOB_SEND_COOLDOWN = 10s;

namespace {

// size in bytes to pre-allocate for the parsing buffer
// estimated based on maximum size of `/proc/[pid]/{stat|status|io}` files
constexpr std::size_t PRE_ALLOCATED_PARSING_BUFFER_CAPACITY = 1200;

} // namespace

ProcessHandler::ProcessHandler(
    ::flowmill::ingest::Writer &writer,
    ::flowmill::kernel_collector::Index &collector_index,
    ProbeHandler &probe_handler,
    ebpf::BPFModule &bpf_module,
    logging::Logger &log,
    CpuMemIoSettings const *cpu_mem_io_settings)
    : writer_(writer),
      collector_index_(collector_index),
      log_(log),
      cpu_mem_io_settings_(cpu_mem_io_settings),
      memory_page_bytes_(memory_page_size().try_raise().value())
{
  LOG::trace_in(
      AgentLogKind::CPU_MEM_IO,
      "ProcessHandler: enable_cpu_mem_io={} memory_page_size={} clock_ticks_per_second={}",
      static_cast<bool>(cpu_mem_io_settings),
      memory_page_bytes_,
      clock_ticks_per_second);

  writer_.system_wide_process_settings(clock_ticks_per_second, memory_page_bytes_);

  if (cpu_mem_io_settings_) {
    LOG::trace_in(
        AgentLogKind::CPU_MEM_IO,
        "ProcessHandler: CPU/Mem/IO settings"
        " min_batch_size={}"
        " max_batch_size={}"
        " poll_budget={}"
        " batch_poll_count_cooldown={}",
        cpu_mem_io_settings_->min_batch_size,
        cpu_mem_io_settings_->max_batch_size,
        cpu_mem_io_settings_->poll_budget,
        cpu_mem_io_settings_->batch_poll_count_cooldown);

    batch_tracker_.emplace(cpu_mem_io_settings_->max_batch_size);
    buffer_.reserve(PRE_ALLOCATED_PARSING_BUFFER_CAPACITY);

    probe_handler.start_probe(bpf_module, "on_finish_task_switch", "finish_task_switch");
  }
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
    LOG::debug_in(AgentLogKind::CPU_MEM_IO, "DUPLICATE TGID {}!", msg);
#endif // DEBUG_TGID
  }

  auto weak_handle = collector_index_.tracked_process.by_key({.cgroup = msg.cgroup, .tgid = msg.pid});
  assert(msg.pid == weak_handle.tgid());
  assert(msg.cgroup == weak_handle.cgroup());

  weak_handle.set_tgid(msg.pid);
  weak_handle.set_command(to_jb_blob(msg.comm));
  weak_handle.set_cgroup(msg.cgroup);

  processes_[msg.pid] = {
      .handle = weak_handle.to_handle(),
      .proc_path =
          {fmt::format("/proc/{}/stat", msg.pid), fmt::format("/proc/{}/io", msg.pid), fmt::format("/proc/{}/status", msg.pid)},
      .by_pid = {},
      .by_tgid = {},
      .last_check = monotonic_clock::now()
#ifdef DEBUG_TGID
          ,
      .timestamp = timestamp,
      .cgroup = msg.cgroup,
      .command = render_array_to_string(msg.comm)
#endif // DEBUG_TGID
  };

  if (cpu_mem_io_settings_) {
    batch_.push(msg.pid);
  }
}

void ProcessHandler::on_process_end(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_close const &msg)
{
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} msg={}", __func__, timestamp, msg);

  batch_.erase(msg.pid);

  if (auto i = processes_.find(msg.pid); i != processes_.end()) {
    i->second.handle.put(collector_index_);
    processes_.erase(i);
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::CPU_MEM_IO, "MISSING TGID {}", msg);
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
    LOG::debug_in(AgentLogKind::CPU_MEM_IO, "MISSING TGID {}", msg);
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
    LOG::debug_in(AgentLogKind::CPU_MEM_IO, "MISSING TGID {}", msg);
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
    LOG::debug_in(AgentLogKind::CPU_MEM_IO, "MISSING TGID {}", msg);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::report_task_status(
    std::chrono::nanoseconds timestamp, struct jb_agent_internal__report_task_status const &msg)
{
  LOG::trace_in(AgentLogKind::PID, "ProcessHandler::{}: timestamp={} msg={}", __func__, timestamp, msg);

  if (auto i = processes_.find(msg.tgid); i != processes_.end()) {
    auto &tgid_data = i->second;
    auto &by_pid = tgid_data.by_pid[msg.pid];
    by_pid += msg;
    // TODO: properly manage time slots (this lumps all data points since last
    //       submit_metrics in the same data point)
    tgid_data.by_tgid.update(timestamp, by_pid, static_cast<bool>(msg.on_exit));
  } else {
    ++stats_.missing_tgid;
#ifdef DEBUG_TGID
    LOG::debug_in(AgentLogKind::CPU_MEM_IO, "MISSING TGID {}", msg);
#endif // DEBUG_TGID
  }
}

void ProcessHandler::poll()
{
  LOG::trace_in(
      AgentLogKind::CPU_MEM_IO,
      "ProcessHandler::poll: enable_cpu_mem_io={} pid_count={}",
      static_cast<bool>(cpu_mem_io_settings_),
      processes_.size());

  if (cpu_mem_io_settings_) {
    poll_cpu_mem_io();
  }
}

void ProcessHandler::slow_poll(u64 timestamp)
{
  submit_metrics(timestamp);
}

void ProcessHandler::submit_metrics(u64 timestamp)
{
  for (auto &process : processes_) {
    auto &tgid_data = process.second;
    auto proxy = tgid_data.handle.access(collector_index_);
    auto &by_tgid = tgid_data.by_tgid;
    if (!by_tgid.pending()) {
      continue;
    }

    auto const timestamp = integer_time<std::chrono::nanoseconds>(by_tgid.timestamp);

    proxy.report_task_cpu_tstamp(timestamp, by_tgid.user_time_ns, by_tgid.system_time_ns, by_tgid.thread_count);

    proxy.report_task_rss_tstamp(
        timestamp,
        by_tgid.resident_pages_file_mapping * memory_page_bytes_,
        by_tgid.resident_pages_anonymous * memory_page_bytes_,
        by_tgid.resident_pages_shared_memory * memory_page_bytes_);

    proxy.report_task_page_faults_tstamp(timestamp, by_tgid.minor_page_faults, by_tgid.major_page_faults);

    proxy.report_task_io_tstamp(
        timestamp,
        by_tgid.read_syscalls,
        by_tgid.write_syscalls,
        by_tgid.bytes_logically_read,
        by_tgid.bytes_logically_written,
        by_tgid.bytes_physically_read,
        by_tgid.bytes_physically_written,
        by_tgid.cancelled_write_bytes);

    proxy.report_task_io_wait_tstamp(
        timestamp,
        by_tgid.block_io_delay_ns,
        by_tgid.block_io_delay_count,
        by_tgid.swap_in_delay_ns,
        by_tgid.swap_in_delay_count,
        by_tgid.free_pages_delay_ns,
        by_tgid.free_pages_delay_count,
        by_tgid.thrashing_page_delay_ns,
        by_tgid.thrashing_page_delay_count);

    proxy.report_task_context_switches_tstamp(
        timestamp, by_tgid.voluntary_context_switches, by_tgid.involuntary_context_switches);

    by_tgid.reset();
  }
}

template <typename View, ProcPath Which> View ProcessHandler::parse_file(u32 tgid, ThreadGroupData &process)
{
  buffer_.clear();

  auto const &path = process.proc_path[enum_index_of(Which)];
  if (auto const result = read_file(path.c_str(), buffer_)) {
    std::string_view const data{reinterpret_cast<char const *>(buffer_.data()), buffer_.size()};
    LOG::trace_in(AgentLogKind::CPU_MEM_IO, "ProcessHandler::parse_file - successfully read {}", path);

    // standard libraries number parsing routines require null terminated
    // strings, so let's ensure there's a null sentinel at the end of the buffer
    buffer_.push_back('\0');

    View view{data};

    if (view) {
      LOG::trace_in(AgentLogKind::CPU_MEM_IO, "ProcessHandler::parse_file - successfully parsed {}", path);
      ++successful_reads_[enum_index_of(Which)];
    } else {
      LOG::trace_in(AgentLogKind::CPU_MEM_IO, "ProcessHandler::parse_file - failed to parse {}", path);
      ++failed_parse_[enum_index_of(Which)];

      auto &cooldown = proc_blob_send_cooldown_[enum_index_of(Which)];
      if (cooldown.elapsed(PROC_BLOB_SEND_COOLDOWN)) {
        constexpr enum_traits<ProcPath>::array_map<CollectedBlobType> type_table{
            CollectedBlobType::proc_pid_stat, CollectedBlobType::proc_pid_io, CollectedBlobType::proc_pid_status};
        writer_.collect_blob(integer_value(type_table[enum_index_of(Which)]), tgid, jb_blob{path}, jb_blob{data});
        cooldown.reset();
      }
    }

    return view;
  } else {
    LOG::trace_in(AgentLogKind::CPU_MEM_IO, "ProcessHandler::parse_file - failed to read {}: {}", path, result.error());

    ++failed_reads_[enum_index_of(Which)];
    return {};
  }
}

void ProcessHandler::poll_cpu_mem_io()
{
  assert(batch_tracker_.has_value());

  if (batch_tracker_->off() && !batch_tracker_->tick(cpu_mem_io_settings_->max_batch_size)) {
    LOG::trace_in(
        AgentLogKind::CPU_MEM_IO,
        "ProcessHandler::poll_cpu_mem_io: cooling down for {} more polls",
        batch_tracker_->remaining());
    return;
  }

  StopWatch stop_watch;
  std::size_t count = 0;

  while (!batch_.empty()) {
    auto const tgid = batch_.pop();

    LOG::trace_in(AgentLogKind::CPU_MEM_IO, "ProcessHandler::poll_cpu_mem_io: processing tgid={}", tgid);
    auto process_ref = processes_.find(tgid);
    if (process_ref == processes_.end()) {
      LOG::trace_in(AgentLogKind::CPU_MEM_IO, "ProcessHandler::poll_cpu_mem_io: process not found tgid={}", tgid);
      continue;
    }

    auto &process_data = process_ref->second;

    auto proxy = process_data.handle.access(collector_index_);

    auto const sample_timestamp = monotonic_clock::now();

    if (auto view = parse_file<ProcStatView, ProcPath::stat>(tgid, process_data)) {
      proxy.report_process_status(
          integer_time<std::chrono::nanoseconds>(sample_timestamp),
          jb_blob{view.starttime.view()},
          jb_blob{view.utime.view()},
          jb_blob{view.stime.view()},
          jb_blob{view.cutime.view()},
          jb_blob{view.cstime.view()},
          integer_value(view.state),
          jb_blob{view.num_threads.view()},
          jb_blob{view.priority.view()},
          jb_blob{view.nice.view()},
          jb_blob{view.rss.view()},
          jb_blob{view.minflt.view()},
          jb_blob{view.majflt.view()},
          jb_blob{view.cminflt.view()},
          jb_blob{view.cmajflt.view()},
          jb_blob{view.delayacct_blkio_ticks.view()},
          jb_blob{view.guest_time.view()},
          jb_blob{view.cguest_time.view()});
    }

    if (auto view = parse_file<ProcIoView, ProcPath::io>(tgid, process_data)) {
      proxy.report_io(
          jb_blob{view.syscr.view()},
          jb_blob{view.syscw.view()},
          jb_blob{view.rchar.view()},
          jb_blob{view.wchar.view()},
          jb_blob{view.read_bytes.view()},
          jb_blob{view.write_bytes.view()},
          jb_blob{view.cancelled_write_bytes.view()});
    }

    if (auto view = parse_file<ProcStatusView, ProcPath::status>(tgid, process_data)) {
      proxy.report_context_switches(
          jb_blob{view.voluntary_ctxt_switches.view()}, jb_blob{view.nonvoluntary_ctxt_switches.view()});
    }

    ++count;

    if (batch_tracker_->tick(cpu_mem_io_settings_->batch_poll_count_cooldown) ||
        (count >= cpu_mem_io_settings_->min_batch_size && stop_watch.elapsed(cpu_mem_io_settings_->poll_budget))) {
      LOG::trace_in(
          AgentLogKind::CPU_MEM_IO,
          "ProcessHandler::poll_cpu_mem_io: ran out of budget after {} pids budget={} elapsed={}",
          count,
          cpu_mem_io_settings_->poll_budget,
          stop_watch);
      break;
    }
  }

  auto const elapsed = stop_watch.elapsed();
  batch_.update(elapsed, count);
  LOG::trace_in(
      AgentLogKind::CPU_MEM_IO,
      "ProcessHandler::poll_cpu_mem_io: remaining={} popped={} elapsed={}",
      batch_.size(),
      count,
      elapsed);

  if (batch_.empty()) {
    if (count) {
      LOG::trace_in(
          AgentLogKind::CPU_MEM_IO,
          "ProcessHandler::poll_cpu_mem_io: reporting batch stats"
          " budget={} elapsed={} count={} size={} min={} max={}",
          integer_time<std::chrono::nanoseconds>(cpu_mem_io_settings_->poll_budget),
          integer_time<std::chrono::nanoseconds>(batch_.elapsed()),
          batch_.batch_count(),
          batch_.total_batch_size(),
          batch_.min_batch_size(),
          batch_.max_batch_size());

      writer_.report_cpu_mem_io_polling_stats(
          integer_time<std::chrono::nanoseconds>(batch_.period()),
          integer_time<std::chrono::nanoseconds>(batch_.elapsed()),
          batch_.batch_count(),
          batch_.total_batch_size(),
          batch_.min_batch_size(),
          batch_.max_batch_size(),
          successful_reads_[enum_index_of(ProcPath::stat)],
          failed_reads_[enum_index_of(ProcPath::stat)],
          successful_reads_[enum_index_of(ProcPath::io)],
          failed_reads_[enum_index_of(ProcPath::io)],
          successful_reads_[enum_index_of(ProcPath::status)],
          failed_reads_[enum_index_of(ProcPath::status)]);

      if (failed_parse_[enum_index_of(ProcPath::stat)] || failed_parse_[enum_index_of(ProcPath::io)] ||
          failed_parse_[enum_index_of(ProcPath::status)]) {
        writer_.report_cpu_mem_io_parse_errors(
            failed_parse_[enum_index_of(ProcPath::stat)],
            failed_parse_[enum_index_of(ProcPath::io)],
            failed_parse_[enum_index_of(ProcPath::status)]);
      }
    }

    batch_.reset(processes_);
    LOG::trace_in(AgentLogKind::CPU_MEM_IO, "ProcessHandler::poll_cpu_mem_io: batch reset size={}", batch_.size());
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

  LOG::debug_in(AgentLogKind::CPU_MEM_IO, "{}", str);
}
#endif // DEBUG_TGID

//////////////
// PidBatch //
//////////////

void ProcessHandler::PidBatch::push(u32 tgid)
{
  if (empty()) {
    period_.reset();
  }

  batch_.insert(tgid);
}

u32 ProcessHandler::PidBatch::pop()
{
  assert(!batch_.empty());
  auto const tgid = *batch_.begin();
  batch_.erase(batch_.begin());
  return tgid;
}

void ProcessHandler::PidBatch::erase(u32 tgid)
{
  batch_.erase(tgid);
}

void ProcessHandler::PidBatch::update(StopWatch<>::duration elapsed, std::size_t count)
{
  if (!batch_count_ || count < min_batch_size_) {
    min_batch_size_ = count;
  }

  if (count > max_batch_size_) {
    max_batch_size_ = count;
  }

  total_batch_size_ += count;
  ++batch_count_;
  elapsed_ += elapsed;
}

void ProcessHandler::PidBatch::reset(absl::flat_hash_map<u32, ThreadGroupData> const &processes)
{
  assert(empty());

  min_batch_size_ = 0;
  max_batch_size_ = 0;
  total_batch_size_ = 0;
  batch_count_ = 0;
  elapsed_ = StopWatch<>::duration::zero();

  period_.reset();

  for (auto const &i : processes) {
    batch_.insert(i.first);
  }
}

//////////////////
// OnOffTracker //
//////////////////

bool ProcessHandler::OnOffTracker::tick(std::size_t tock_count)
{
  if (count_--) {
    return false;
  }

  on_ = !on_;
  count_ = tock_count;
  return true;
}
