// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <cstdarg>
#include <iostream>

#include <config.h>
#include <linux/bpf.h>
#include <util/log.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>

#include <collector/agent_log.h>
#include <collector/kernel/bpf_src/render_bpf.h>
#include <collector/kernel/probe_handler.h>

// Include the generated skeleton
extern "C" {
#include "generated/render_bpf.skel.h"
}

#include <memory>
#include <vector>

#define EVENTS_PERF_RING_N_BYTES (1024 * 4096)
#define EVENTS_PERF_RING_N_WATERMARK_BYTES (512 * 4096)
#define DATA_CHANNEL_PERF_RING_N_BYTES (256 * 4096)
#define DATA_CHANNEL_PERF_RING_N_WATERMARK_BYTES (1)

// Helper function to get online CPUs
std::vector<int> get_online_cpus()
{
  std::vector<int> cpus;
  int num_cpus = libbpf_num_possible_cpus();
  for (int i = 0; i < num_cpus; i++) {
    cpus.push_back(i);
  }
  return cpus;
}

ProbeHandler::ProbeHandler(logging::Logger &log) : log_(log), num_failed_probes_(0), stack_trace_count_(0) {};

void ProbeHandler::load_kernel_symbols()
{
  KernelSymbols ks;
  try {
    ks = read_proc_kallsyms();
  } catch (std::exception &exc) {
    log_.error("Failed to load kernel symbols: {}", exc.what());
    return;
  }

  if (!ks.empty()) {
    LOG::info("Kernel symbols list loaded");
    kernel_symbols_ = std::move(ks);
  }
}

void ProbeHandler::clear_kernel_symbols()
{
  kernel_symbols_.reset();
}

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
  int res = bpf_map_update_elem(perf_fd, static_cast<void *>(&cpu), static_cast<void *>(&mmap_fd), 0);
  if (res < 0) {
    LOG::error("cannot set perf fd {} for cpu {} to point to ring fd {}", perf_fd, cpu, mmap_fd);
    return -4;
  }

  return 0;
}

int ProbeHandler::get_bpf_map_fd(struct render_bpf_bpf *skel, const char *map_name)
{
  struct bpf_map *map = get_bpf_map(skel, map_name);
  if (!map) {
    LOG::error("Cannot get '{}' map", map_name);
    return -2;
  }
  int map_fd = bpf_map__fd(map);
  if (map_fd < 0) {
    LOG::error("'{}' map's fd<0: fd={}", map_name, map_fd);
    return -3;
  }
  return map_fd;
}

// Callback to route libbpf messages through our logging system
static int libbpf_print_messages(enum libbpf_print_level level, const char *format, va_list args)
{
  char buffer[16 * 1024];
  int len = vsnprintf(buffer, sizeof(buffer), format, args);

  // Remove trailing newline if present
  if (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
    len--;
  }

  std::string message(buffer);
  if (level <= LIBBPF_INFO)
    LOG::debug("libbpf: {}", message);
  else
    LOG::trace("libbpf: {}", message);

  return len;
}

struct render_bpf_bpf *ProbeHandler::open_bpf_skeleton()
{
  // Set a custom print callback to route libbpf messages through our logging system
  libbpf_set_print(libbpf_print_messages);

  struct render_bpf_bpf *skel = render_bpf_bpf__open();
  if (!skel) {
    LOG::error("Cannot open BPF skeleton");
    return nullptr;
  }
  return skel;
}

void ProbeHandler::configure_bpf_skeleton(struct render_bpf_bpf *skel, const BpfConfiguration &config)
{
  if (!skel) {
    LOG::error("Cannot configure BPF skeleton: null skeleton");
    return;
  }

  // Configure global variables before loading
  skel->rodata->boot_time_adjustment = config.boot_time_adjustment;
  skel->rodata->filter_ns = config.filter_ns;
  skel->rodata->enable_tcp_data_stream = config.enable_tcp_data_stream ? 1 : 0;

  LOG::info(
      "BPF configuration: boot_time_adjustment={}, filter_ns={}, tcp_data_stream={}",
      config.boot_time_adjustment,
      config.filter_ns,
      config.enable_tcp_data_stream ? "enabled" : "disabled");
}

void ProbeHandler::destroy_bpf_skeleton(struct render_bpf_bpf *skel)
{
  if (skel) {
    render_bpf_bpf__destroy(skel);
  }
}

int ProbeHandler::load_bpf_skeleton(struct render_bpf_bpf *skel, PerfContainer &perf)
{
  int res = render_bpf_bpf__load(skel);
  if (res != 0) {
    LOG::error("Cannot load BPF skeleton, res={}", res);
    return res;
  }
  LOG::info("eBPF program successfully loaded");

  /* get events map descriptor */
  int events_fd = get_bpf_map_fd(skel, "events");
  if (events_fd < 0) {
    return events_fd;
  }

  /* get data_channel map descriptor */
  int data_channel_fd = get_bpf_map_fd(skel, "data_channel");
  if (data_channel_fd < 0) {
    return data_channel_fd;
  }

  /* get online cpus */
  auto online_cpus = get_online_cpus();

  /* open mmaps */
  for (auto cpu : online_cpus) {
    res = setup_mmap(cpu, events_fd, perf, false, EVENTS_PERF_RING_N_BYTES, EVENTS_PERF_RING_N_WATERMARK_BYTES);
    if (res < 0) {
      return res;
    }
    res =
        setup_mmap(cpu, data_channel_fd, perf, true, DATA_CHANNEL_PERF_RING_N_BYTES, DATA_CHANNEL_PERF_RING_N_WATERMARK_BYTES);
    if (res < 0) {
      return res;
    }
  }

  return 0;
}

struct bpf_map *ProbeHandler::get_bpf_map(struct render_bpf_bpf *skel, const std::string &name)
{
  if (name == "cgroup_exit_active")
    return skel->maps.cgroup_exit_active;
  if (name == "tcp_open_sockets")
    return skel->maps.tcp_open_sockets;
  if (name == "on_inet_csk_accept_active")
    return skel->maps.on_inet_csk_accept_active;
  if (name == "seen_inodes")
    return skel->maps.seen_inodes;
  if (name == "udp_get_port_hash")
    return skel->maps.udp_get_port_hash;
  if (name == "inet_release_active")
    return skel->maps.inet_release_active;
  if (name == "tail_calls")
    return skel->maps.tail_calls;
  if (name == "cgroup_control_active")
    return skel->maps.cgroup_control_active;
  if (name == "seen_conntracks")
    return skel->maps.seen_conntracks;
  if (name == "inet_csk_accept_active")
    return skel->maps.inet_csk_accept_active;
  if (name == "tcp_sendmsg_active")
    return skel->maps.tcp_sendmsg_active;
  if (name == "tcp_recvmsg_active")
    return skel->maps.tcp_recvmsg_active;
  if (name == "events")
    return skel->maps.events;
  if (name == "bpf_log_globals_per_cpu")
    return skel->maps.bpf_log_globals_per_cpu;
  if (name == "tgid_info_table")
    return skel->maps.tgid_info_table;
  if (name == "dead_group_tasks")
    return skel->maps.dead_group_tasks;
  if (name == "udp_open_sockets")
    return skel->maps.udp_open_sockets;
  if (name == "dns_message_array")
    return skel->maps.dns_message_array;
  if (name == "_tcp_connections")
    return skel->maps._tcp_connections;
  if (name == "_tcp_control")
    return skel->maps._tcp_control;
  if (name == "data_channel")
    return skel->maps.data_channel;
  return nullptr;
}

int ProbeHandler::register_tail_call(
    struct render_bpf_bpf *skel, const std::string &prog_array_name, int index, const std::string &func_name)
{
  struct bpf_map *prog_array = get_bpf_map(skel, prog_array_name);
  if (!prog_array) {
    log_.error("Failed to find prog array map {}", prog_array_name);
    ++num_failed_probes_;
    return -1;
  }

  // TCP processor programs need to be looked up by function name
  struct bpf_program *prog = bpf_object__find_program_by_name(skel->obj, func_name.c_str());
  if (!prog) {
    log_.error("Failed to register tail call for {}, could not find program", func_name);
    ++num_failed_probes_;
    return -2;
  }

  int prog_fd = bpf_program__fd(prog);
  if (prog_fd < 0) {
    log_.error("Failed to register tail call for {}, could not get program fd", func_name);
    ++num_failed_probes_;
    return -3;
  }

  int map_fd = bpf_map__fd(prog_array);
  int ret = bpf_map_update_elem(map_fd, &index, &prog_fd, 0);
  if (ret < 0) {
    log_.error("Failed to update prog array for tail call {}, errno {}", func_name, errno);
    ++num_failed_probes_;
    return -4;
  }

  tail_calls_.emplace_back(prog_array_name, func_name, prog_fd, index);
  return 0;
}

#if DEBUG_ENABLE_STACKTRACE

std::string ProbeHandler::get_stack_trace(struct render_bpf_bpf *skel, s32 kernel_stack_id, s32 user_stack_id, u32 tgid)
{
  std::string out;

  // TODO: Implement stack trace functionality for libbpf
  // This requires additional implementation for stack trace handling
  out += "Stack trace functionality not yet implemented for libbpf\n";

  stack_trace_count_++;
  return out;
}

#endif

int ProbeHandler::start_probe_common(
    struct render_bpf_bpf *skel,
    bool is_kretprobe,
    const std::string &func_name,
    const std::string &k_func_name,
    const std::string &event_id_suffix)
{
  auto bpf_program = bpf_object__find_program_by_name(skel->obj, func_name.c_str());
  if (!bpf_program) {
    LOG::error("Could not get find program. func_name:{} k_func_name:{}", func_name, k_func_name);
    return -1;
  }

  /* attach the probe */
  std::string probe_name = (is_kretprobe ? kretprobe_prefix_ : probe_prefix_) + k_func_name + event_id_suffix;

  struct bpf_link *link;
  link = bpf_program__attach_kprobe(
      bpf_program,
      is_kretprobe, /* retprobe */
      k_func_name.c_str());

  if (!link) {
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

  // Store the link pointer for cleanup
  probes_.push_back(link);
  probe_names_.push_back(probe_name);
  return 0;
}

int ProbeHandler::start_probe(
    struct render_bpf_bpf *skel,
    const std::string &func_name,
    const std::string &k_func_name,
    const std::string &event_id_suffix)
{
  auto ret = start_probe_common(skel, false, func_name, k_func_name, event_id_suffix);
  if (ret != 0) {
    log_.error("Failed to attach {} kprobe, error {}", k_func_name, ret);
    ++num_failed_probes_;
  }
  return ret;
}

int ProbeHandler::start_kretprobe(
    struct render_bpf_bpf *skel,
    const std::string &func_name,
    const std::string &k_func_name,
    const std::string &event_id_suffix)
{
  auto ret = start_probe_common(skel, true, func_name, k_func_name, event_id_suffix);
  if (ret != 0) {
    log_.error("Failed to attach {} kretprobe, error {}", k_func_name, ret);
    ++num_failed_probes_;
  }
  return ret;
}

std::string ProbeHandler::start_probe_common(
    struct render_bpf_bpf *skel,
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
    int ret = start_probe_common(skel, is_kretprobe, func_and_kfunc.func_name, func_and_kfunc.k_func_name, event_id_suffix);
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
    struct render_bpf_bpf *skel, const ProbeAlternatives &probe_alternatives, const std::string &event_id_suffix)
{
  return start_probe_common(skel, false, probe_alternatives, event_id_suffix);
}

std::string ProbeHandler::start_kretprobe(
    struct render_bpf_bpf *skel, const ProbeAlternatives &probe_alternatives, const std::string &event_id_suffix)
{
  return start_probe_common(skel, true, probe_alternatives, event_id_suffix);
}

void ProbeHandler::cleanup_probes()
{
  while (!probes_.empty()) {
    auto probe = probes_.back();
    probes_.pop_back();
    std::string probe_name = probe_names_.back();
    probe_names_.pop_back();

    LOG::debug_in(AgentLogKind::BPF, "cleanup probe for {}", probe_name);

    // Clean up the bpf_link
    if (probe) {
      bpf_link__destroy(probe);
    }
  }

  LOG::debug_in(AgentLogKind::BPF, "Done cleaning up probes");
}

void ProbeHandler::cleanup_tail_calls(struct render_bpf_bpf *skel)
{
  while (!tail_calls_.empty()) {
    const auto &tc = tail_calls_.back();

    LOG::debug_in(AgentLogKind::BPF, "cleanup tail call for {} from table {}", tc.func_, tc.table_);

    struct bpf_map *prog_array = get_bpf_map(skel, tc.table_);
    if (prog_array) {
      int map_fd = bpf_map__fd(prog_array);
      bpf_map_delete_elem(map_fd, &tc.index_);
    }

    tail_calls_.pop_back();
  }
}

void ProbeHandler::cleanup_probe_common(const std::string &probe_name)
{
  int i = 0;
  for (std::vector<std::string>::iterator it = probe_names_.begin(); it != probe_names_.end(); ++it) {
    if (!probe_names_[i].compare(probe_name)) {
      auto probe = probes_[i];
      std::string probe_name = probe_names_[i];

      probes_.erase(probes_.begin() + i);
      probe_names_.erase(probe_names_.begin() + i);

      LOG::debug_in(AgentLogKind::BPF, "cleanup probe for {}", probe_name);

      // Clean up the bpf_link
      if (probe) {
        bpf_link__destroy(probe);
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
