/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/platform.h>

#include <collector/kernel/buffered_poller.h>
#include <collector/kernel/probe_handler.h>
#include <generated/ebpf_net/ingest/encoder.h>
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
      ::ebpf_net::ingest::Encoder *encoder);

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
      u64 socket_stats_interval_sec,
      CgroupHandler::CgroupSettings const &cgroup_settings,
      KernelCollectorRestarter &kernel_collector_restarter);

  /**
   * Loads BPF probes. Takes writer to send out steady_state msgs
   * where necessary.
   */
  void load_probes(::ebpf_net::ingest::Writer &writer);

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
  ::ebpf_net::ingest::Encoder *encoder_;
  std::unique_ptr<BufferedPoller> buf_poller_;
  bool enable_http_metrics_;
  bool enable_userland_tcp_;
  FileDescriptor &bpf_dump_file_;
  logging::Logger &log_;
  u64 last_lost_count_;
};
