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

#include <linux/bpf.h>

#include <bcc/BPFTable.h>
#include <bcc/bpf_module.h>
#include <collector/kernel/perf_reader.h>
#include <string>
#include <vector>

/**
 * Handles the creation of probes and provides info for cleanup to signal
 * handler
 */
class ProbeHandler {
public:
  /**
   * c'tor
   */
  ProbeHandler();

  int start_bpf_module(std::string full_program, ebpf::BPFModule &bpf_module, PerfContainer &perf);

  /**
   * BPF table helpers
   **/
  ebpf::BPFHashTable<u32, u32> get_hash_table(ebpf::BPFModule &bpf_module, const std::string &name);
  ebpf::BPFProgTable get_prog_table(ebpf::BPFModule &bpf_module, const std::string &name);
  ebpf::BPFStackTable get_stack_table(ebpf::BPFModule &bpf_module, const std::string &name);

  /**
   * Register tail call in table
   */
  int register_tail_call(
      ebpf::BPFModule &bpf_module, const std::string &prog_array_name, int index, const std::string &func_name);

  /**
   * Starts a kprobe
   * @returns 0 on success, negative value on failure
   */
  int start_probe(
      ebpf::BPFModule &bpf_module,
      const std::string &func_name,
      const std::string &k_func_name,
      const std::string &event_id_suffix = std::string());

  /**
   * Starts a kretprobe
   * @returns 0 on success, negative value on failure
   */
  int start_kretprobe(
      ebpf::BPFModule &bpf_module,
      const std::string &func_name,
      const std::string &k_func_name,
      const std::string &event_id_suffix = std::string());

  /**
   * Handles the cleanup of probes on exit
   */
  void cleanup_probes();

  /**
   * Cleans up a single probe
   */
  void cleanup_probe(std::string func_name);

#if DEBUG_ENABLE_STACKTRACE
  /**
   * Gets a stack trace and removes it from the list
   */
  std::string get_stack_trace(ebpf::BPFModule &bpf_module, s32 kernel_stack_id, s32 user_stack_id, u32 tgid);
#endif

protected:
  /**
   * Returns the file descriptor for a table declared in bpf
   */
  int get_bpf_table_descriptor(ebpf::BPFModule &bpf_module, const char *table_name);

  /**
   * Sets up memory mapping for perf rings
   */
  int setup_mmap(int cpu, int events_fd, PerfContainer &perf, bool is_data, u32 n_bytes, u32 n_watermark_bytes);

private:
  std::vector<int> fds_;
  std::vector<int> probes_;
  std::vector<std::string> k_func_names_;
  size_t stack_trace_count_;
};
