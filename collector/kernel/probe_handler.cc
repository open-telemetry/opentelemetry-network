// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <iostream>

#include <config.h>
#include <linux/bpf.h>
#include <util/log.h>

#include <bcc/BPF.h>
#include <bcc/libbpf.h>
#include <bcc/perf_reader.h>
#include <bcc/table_storage.h>

#include <collector/agent_log.h>
#include <collector/kernel/bpf_src/render_bpf.h>
#include <collector/kernel/probe_handler.h>

#include <memory>
#include <vector>

#define EVENTS_PERF_RING_N_BYTES (1024 * 4096)
#define EVENTS_PERF_RING_N_WATERMARK_BYTES (512 * 4096)
#define DATA_CHANNEL_PERF_RING_N_BYTES (256 * 4096)
#define DATA_CHANNEL_PERF_RING_N_WATERMARK_BYTES (1)

namespace ebpf {
// This function is declared in BCC's common.h, which is private (doesn't get installed).
// TODO: remove this when bcc/common.h is made public.
std::vector<int> get_online_cpus();
}

ProbeHandler::ProbeHandler(logging::Logger &log) : log_(log), num_failed_probes_(0), stack_trace_count_(0){};

int ProbeHandler::setup_mmap(int cpu, int perf_fd, PerfContainer &perf, bool is_data, u32 n_bytes, u32 n_watermark_bytes)
{
  /* get mmap'd memory */
  auto s = std::make_unique<MmapPerfRingStorage>(cpu, n_bytes, n_watermark_bytes);
  int mmap_fd = s->fd();

  /* add to perf container */
  PerfRing ring(std::move(s));
  if (is_data) {
    perf.add_data_ring(ring);
  } else {
    perf.add_ring(ring);
  }

  /* set the file descriptor in the kernel */
  int res = bpf_update_elem(perf_fd, static_cast<void *>(&cpu), static_cast<void *>(&mmap_fd), 0);
  if (res < 0) {
    LOG::error("cannot set perf fd {} for cpu {} to point to ring fd {}", perf_fd, cpu, mmap_fd);
    return -4;
  }

  return 0;
}

int ProbeHandler::get_bpf_table_descriptor(ebpf::BPFModule &bpf_module, const char *table_name)
{
  ebpf::TableStorage::iterator it;
  ebpf::Path path({bpf_module.id(), table_name});
  int res = bpf_module.table_storage().Find(path, it);
  if (res == 0) {
    LOG::error("Cannot get '{}' table", table_name);
    return -2;
  }
  int events_fd = it->second.fd;
  if (events_fd < 0) { /* sanity check */
    LOG::error("'{}' table's fd<0: fd={}", table_name, events_fd);
    return -3;
  }

  return events_fd;
}

int ProbeHandler::start_bpf_module(std::string full_program, ebpf::BPFModule &bpf_module, PerfContainer &perf)
{
  int res = bpf_module.load_string(full_program, nullptr, 0);
  if (res != 0) {
    LOG::error("Cannot initialize BPF program, res={}", res);
    return -1;
  }
  LOG::info("eBPF program successfully compiled");

  /* get events table descriptor */
  int events_fd = get_bpf_table_descriptor(bpf_module, "events");
  if (events_fd < 0) {
    return events_fd;
  }

  /* get data_channel table descriptor */
  int data_channel_fd = get_bpf_table_descriptor(bpf_module, "data_channel");
  if (data_channel_fd < 0) {
    return data_channel_fd;
  }

  /* get online cpus */
  auto online_cpus = ebpf::get_online_cpus();

  /* open mmaps */
  for (auto cpu : online_cpus) {
    res = setup_mmap(cpu, events_fd, perf, false, EVENTS_PERF_RING_N_BYTES, EVENTS_PERF_RING_N_WATERMARK_BYTES);
    if (res < 0)
      return res;
    res =
        setup_mmap(cpu, data_channel_fd, perf, true, DATA_CHANNEL_PERF_RING_N_BYTES, DATA_CHANNEL_PERF_RING_N_WATERMARK_BYTES);
    if (res < 0)
      return res;
  }

  return 0;
}

// template <class KeyType, class ValueType>
ebpf::BPFHashTable<u32, u32> ProbeHandler::get_hash_table(ebpf::BPFModule &bpf_module, const std::string &name)
{
  ebpf::TableStorage::iterator it;
  ebpf::Path path({bpf_module.id(), name});
  if (bpf_module.table_storage().Find(path, it)) {
    return ebpf::BPFHashTable<u32, u32>(it->second);
  }
  throw std::runtime_error("ProbeHandler: hash table not found");
}

ebpf::BPFProgTable ProbeHandler::get_prog_table(ebpf::BPFModule &bpf_module, const std::string &name)
{
  ebpf::TableStorage::iterator it;
  ebpf::Path path({bpf_module.id(), name});
  if (bpf_module.table_storage().Find(path, it)) {
    return ebpf::BPFProgTable(it->second);
  }
  throw std::runtime_error("ProbeHandler: prog table not found");
}

ebpf::BPFStackTable ProbeHandler::get_stack_table(ebpf::BPFModule &bpf_module, const std::string &name)
{
  ebpf::TableStorage::iterator it;
  ebpf::Path path({bpf_module.id(), name});
  if (bpf_module.table_storage().Find(path, it)) {
    return ebpf::BPFStackTable(it->second, true, true);
  }
  throw std::runtime_error("ProbeHandler: stack table not found");
}

int ProbeHandler::register_tail_call(
    ebpf::BPFModule &bpf_module, const std::string &prog_array_name, int index, const std::string &func_name)
{
  ebpf::BPFProgTable prog_array = get_prog_table(bpf_module, prog_array_name);

  uint8_t *func_start = bpf_module.function_start(func_name);
  if (func_start == nullptr) {
    log_.error("Failed to register tail call for {}, could not get function start", func_name);
    ++num_failed_probes_;
    return -1;
  }
  size_t func_size = bpf_module.function_size(func_name);
  int prog_fd = bcc_prog_load(
      BPF_PROG_TYPE_KPROBE,
      nullptr,
      reinterpret_cast<struct bpf_insn *>(func_start),
      func_size,
      bpf_module.license(),
      bpf_module.kern_version(),
      0,
      nullptr,
      0);
  if (prog_fd < 0) {
    log_.error("Failed to register tail call for {}, could not load program, errno {}", func_name, errno);
    ++num_failed_probes_;
    return -2;
  }

  tail_calls_.emplace_back(prog_array_name, func_name, prog_fd, index);
  return prog_array.update_value(index, prog_fd).code();
}

#if DEBUG_ENABLE_STACKTRACE

std::string ProbeHandler::get_stack_trace(ebpf::BPFModule &bpf_module, s32 kernel_stack_id, s32 user_stack_id, u32 tgid)
{
  std::string out;
  ebpf::BPFStackTable stack_traces = get_stack_table(bpf_module, "stack_traces");

  out += "Kernel Stack:\n";
  if (kernel_stack_id >= 0) {
    auto syms = stack_traces.get_stack_symbol(kernel_stack_id, -1);
    for (auto sym : syms)
      out += "  " + sym + "\n";
  } else if (kernel_stack_id == -EFAULT) {
    out += "  [UNAVAILABLE]";
  } else {
    out += "  [LOST]";
  }
  out += "User Stack:\n";
  if (user_stack_id >= 0) {
    auto syms = stack_traces.get_stack_symbol(user_stack_id, tgid);
    for (auto sym : syms)
      out += "  " + sym + "\n";
  } else if (user_stack_id == -EFAULT) {
    out += "  [UNAVAILABLE]";
  } else {
    out += "  [LOST]";
  }

  stack_trace_count_++;
  if (stack_trace_count_ >= WATERMARK_STACK_TRACES) {
    stack_traces.clear_table_non_atomic();
    stack_trace_count_ = 0;
  }

  return out;
}

#endif

int ProbeHandler::start_probe_common(
    ebpf::BPFModule &bpf_module,
    bool is_kretprobe,
    const std::string &func_name,
    const std::string &k_func_name,
    const std::string &event_id_suffix)
{
  uint8_t *func_start = bpf_module.function_start(func_name);
  if (func_start == nullptr) {
    LOG::debug_in(AgentLogKind::BPF, "Could not get function start. func_name:{} k_func_name:{}", func_name, k_func_name);
    return -1;
  }
  size_t func_size = bpf_module.function_size(func_name);
  int prog_fd = bcc_prog_load(
      BPF_PROG_TYPE_KPROBE,
      nullptr,
      reinterpret_cast<struct bpf_insn *>(func_start),
      func_size,
      bpf_module.license(),
      bpf_module.kern_version(),
      0,
      nullptr,
      0);
  if (prog_fd < 0) {
    LOG::debug_in(
        AgentLogKind::BPF, "Failed to load prog. func_name:{} k_func_name:{} error:{}", func_name, k_func_name, prog_fd);
    return -2;
  }

  /* attach the probe */
  std::string probe_name = (is_kretprobe ? kretprobe_prefix_ : probe_prefix_) + k_func_name + event_id_suffix;
  int attach_res = bpf_attach_kprobe(
      prog_fd, (is_kretprobe ? BPF_PROBE_RETURN : BPF_PROBE_ENTRY), probe_name.c_str(), k_func_name.c_str(), 0, 0);
  if (attach_res == -1) {
    // we expect this to be triggered depending on kernel version
    close(prog_fd);
    LOG::debug_in(
        AgentLogKind::BPF,
        "Unable to attach {}. probe_name:{} func_name:{} k_func_name:{} errno:{}",
        is_kretprobe ? "kretprobe" : "kprobe",
        probe_name,
        func_name,
        k_func_name,
        errno);
    return -3;
  }

  fds_.push_back(prog_fd);
  probes_.push_back(attach_res);
  probe_names_.push_back(probe_name);
  return 0;
}

int ProbeHandler::start_probe(
    ebpf::BPFModule &bpf_module,
    const std::string &func_name,
    const std::string &k_func_name,
    const std::string &event_id_suffix)
{
  auto ret = start_probe_common(bpf_module, false, func_name, k_func_name, event_id_suffix);
  if (ret != 0) {
    log_.error("Failed to attach {} kprobe, error {}", k_func_name, ret);
    ++num_failed_probes_;
  }
  return ret;
}

int ProbeHandler::start_kretprobe(
    ebpf::BPFModule &bpf_module,
    const std::string &func_name,
    const std::string &k_func_name,
    const std::string &event_id_suffix)
{
  auto ret = start_probe_common(bpf_module, true, func_name, k_func_name, event_id_suffix);
  if (ret != 0) {
    log_.error("Failed to attach {} kretprobe, error {}", k_func_name, ret);
    ++num_failed_probes_;
  }
  return ret;
}

std::string ProbeHandler::start_probe_common(
    ebpf::BPFModule &bpf_module,
    bool is_kretprobe,
    const ProbeAlternatives &probe_alternatives,
    const std::string &event_id_suffix)
{
  size_t probe_num = 1;
  size_t num_alternatives = probe_alternatives.func_names.size();
  if (num_alternatives == 0) {
    throw std::runtime_error("ProbeHandler:start_probe_common() no alternatives provided");
  }
  for (const auto &func_and_kfunc : probe_alternatives.func_names) {
    int ret =
        start_probe_common(bpf_module, is_kretprobe, func_and_kfunc.func_name, func_and_kfunc.k_func_name, event_id_suffix);
    if (ret == 0) {
      LOG::debug_in(
          AgentLogKind::BPF,
          "Successfully attached {} {}, alternative {} of {}, func_name={}, k_func_name={}",
          probe_alternatives.desc,
          is_kretprobe ? "kretprobe" : "kprobe",
          probe_num,
          num_alternatives,
          func_and_kfunc.func_name,
          func_and_kfunc.k_func_name);
      return func_and_kfunc.k_func_name;
      break;
    }
    ++probe_num;
  }
  log_.error(
      "Failed to attach any {} {}, attempted {} alternatives",
      probe_alternatives.desc,
      is_kretprobe ? "kretprobe" : "kprobe",
      num_alternatives);
  ++num_failed_probes_;
  return std::string();
}

std::string ProbeHandler::start_probe(
    ebpf::BPFModule &bpf_module, const ProbeAlternatives &probe_alternatives, const std::string &event_id_suffix)
{
  return start_probe_common(bpf_module, false, probe_alternatives, event_id_suffix);
}

std::string ProbeHandler::start_kretprobe(
    ebpf::BPFModule &bpf_module, const ProbeAlternatives &probe_alternatives, const std::string &event_id_suffix)
{
  return start_probe_common(bpf_module, true, probe_alternatives, event_id_suffix);
}

void ProbeHandler::cleanup_probes()
{
  while (!fds_.empty()) {
    int fd = fds_.back();
    fds_.pop_back();
    auto probe = probes_.back();
    probes_.pop_back();
    std::string probe_name = probe_names_.back();
    probe_names_.pop_back();

    LOG::debug_in(AgentLogKind::BPF, "cleanup probe for {}", probe_name);

    bpf_close_perf_event_fd(probe);

    int res = close(fd);
    if (res != 0) {
      LOG::debug_in(AgentLogKind::BPF, "Error when unloading prog, res={}", res);
    }

    res = bpf_detach_kprobe(probe_name.c_str());
    if (res != 0) {
      LOG::debug_in(AgentLogKind::BPF, "Error when detaching, res={}", res);
    }
  }

  LOG::debug_in(AgentLogKind::BPF, "Done cleaning up probes");
}

void ProbeHandler::cleanup_tail_calls(ebpf::BPFModule &bpf_module)
{
  while (!tail_calls_.empty()) {
    const auto &tc = tail_calls_.back();

    LOG::debug_in(AgentLogKind::BPF, "cleanup tail call for {} from table {}", tc.func_, tc.table_);

    ebpf::BPFProgTable prog_array = get_prog_table(bpf_module, tc.table_);
    prog_array.remove_value(tc.index_);

    int res = close(tc.fd_);
    if (res != 0) {
      LOG::debug_in(AgentLogKind::BPF, "Error when unloading prog, res={}", res);
    }

    tail_calls_.pop_back();
  }
}

void ProbeHandler::cleanup_probe_common(const std::string &probe_name)
{
  int i = 0;
  for (std::vector<std::string>::iterator it = probe_names_.begin(); it != probe_names_.end(); ++it) {
    if (!probe_names_[i].compare(probe_name)) {
      int fd = fds_[i];
      auto probe = probes_[i];
      std::string probe_name = probe_names_[i];

      fds_.erase(fds_.begin() + i);
      probes_.erase(probes_.begin() + i);
      probe_names_.erase(probe_names_.begin() + i);

      LOG::debug_in(AgentLogKind::BPF, "cleanup probe for {}", probe_name);

      bpf_close_perf_event_fd(probe);

      int res = close(fd);
      if (res != 0) {
        LOG::debug_in(AgentLogKind::BPF, "Error when unloading prog, res={}", res);
      }

      res = bpf_detach_kprobe(probe_name.c_str());
      if (res != 0) {
        LOG::debug_in(AgentLogKind::BPF, "Error when detaching, res={}", res);
      }
      return;
    }
    ++i;
  }

  log_.error("Error removing probe. {} was not found.", probe_name);
}

void ProbeHandler::cleanup_probe(const std::string &k_func_name)
{
  if (k_func_name.empty())
    return;

  cleanup_probe_common(probe_prefix_ + k_func_name);
}

void ProbeHandler::cleanup_kretprobe(const std::string &k_func_name)
{
  if (k_func_name.empty())
    return;

  cleanup_probe_common(kretprobe_prefix_ + k_func_name);
}
