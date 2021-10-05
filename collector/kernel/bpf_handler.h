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

#include <platform/platform.h>

#include <collector/kernel/buffered_poller.h>
#include <collector/kernel/probe_handler.h>
#include <generated/flowmill/ingest/encoder.h>
#include <util/curl_engine.h>
#include <util/logger.h>
#include <uv.h>

#include <memory>

class BPFHandler {
public:
  /**
   * c'tor
   *
   * Will throw if:
   * 1. PerfContainer cannot be allocated
   * 2. ProbeHandler can't load BPFModule
   */
  BPFHandler(
      uv_loop_t &loop,
      std::string full_program,
      bool enable_http_metrics,
      bool enable_userland_tcp,
      FileDescriptor &bpf_dump_file,
      logging::Logger &log,
      ::flowmill::ingest::Encoder *encoder);

  /**
   * d'tor
   */
  virtual ~BPFHandler();

  /**
   * Loads the buffered poller
   */
  void load_buffered_poller(
      IBufferedWriter &buffered_writer,
      u64 boot_time_adjustment,
      CurlEngine &curl_engine,
      NicPoller &nic_poller,
      CgroupHandler::CgroupSettings const &cgroup_settings,
      ProcessHandler::CpuMemIoSettings const *cpu_mem_io_settings,
      KernelCollectorRestarter &kernel_collector_restarter);

  /**
   * Loads BPF probes. Takes writer to send out steady_state msgs
   * where necessary.
   */
  void load_probes(::flowmill::ingest::Writer &writer);

  /**
   * Calls start(interval_useconds, n_intervals) on buf_poller_
   */
  void start_poll(u64 interval_useconds, u64 n_intervals);

  /**
   * Calls less frequent cleanup operations on buf_poller_
   */
  void slow_poll();

  /**
   * Calls serv_lost_count() on buf_poller_
   */
  u64 serv_lost_count();

  /**
   * Callback passed to probers for checking lost count
   */
  void check_cb(std::string error_loc);

#ifndef NDEBUG
  /**
   * Debug code for internal development to simulate lost BPF samples (PERF_RECORD_LOST) in BufferedPoller.
   */
  void debug_bpf_lost_samples();
#endif

private:
  uv_loop_t &loop_;
  ProbeHandler probe_handler_;
  ebpf::BPFModule bpf_module_;
  PerfContainer perf_;
  ::flowmill::ingest::Encoder *encoder_;
  std::unique_ptr<BufferedPoller> buf_poller_;
  bool enable_http_metrics_;
  bool enable_userland_tcp_;
  FileDescriptor &bpf_dump_file_;
  logging::Logger &log_;
  u64 last_lost_count_;
};
