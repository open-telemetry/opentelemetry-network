/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <channel/callbacks.h>
#include <channel/file_channel.h>
#include <channel/upstream_connection.h>
#include <collector/kernel/bpf_handler.h>
#include <collector/kernel/kernel_collector_restarter.h>
#include <collector/kernel/probe_handler.h>
#include <common/host_info.h>
#include <config/intake_config.h>
#include <generated/ebpf_net/ingest/writer.h>
#include <platform/platform.h>
#include <scheduling/interval_scheduler.h>
#include <util/aws_instance_metadata.h>
#include <util/curl_engine.h>
#include <util/gcp_instance_metadata.h>
#include <util/logger.h>

#include <uv.h>

#include <map>

class KernelCollector {
  friend class KernelCollectorRestarter;
  friend class KernelCollectorTest;

public:
  /**
   * c'tor
   */
  KernelCollector(
      const BpfConfiguration &bpf_config,
      config::IntakeConfig const &intake_config,
      AwsMetadata const *aws_metadata,
      GcpInstanceMetadata const *gcp_metadata,
      std::map<std::string, std::string> config_labels,
      uv_loop_t &loop,
      CurlEngine &curl_engine,
      bool enable_http_metrics,
      u64 socket_stats_interval_sec,
      CgroupHandler::CgroupSettings cgroup_settings,
      std::string const &bpf_dump_file,
      HostInfo host_info);

  /**
   * d'tor
   */
  virtual ~KernelCollector();

  /**
   * called by the uv_timer callback for establishing server connections
   */
  void try_connecting(uv_timer_t *timer);

  /**
   * called by the uv_timer callback for timing out connection attempts
   */
  void connection_timeout(uv_timer_t *timer);

  /**
   * called by the uv_timer callback for starting bpf probes
   */
  void probe_holdoff_timeout(uv_timer_t *timer);

  /**
   * called by the uv_timer callback for polling processes
   */
  void polling_steady_state(uv_timer_t *timer);

  /**
   * called by the uv_timer callback for slow polling processes
   */
  void polling_steady_state_slow(uv_timer_t *timer);

  /**
   * called when upstream is connected, to complete connection
   */
  void on_upstream_connected();

  /* called from callbacks given to uv_close */
  void on_close();

#ifndef NDEBUG
  /* Debug code for internal development to simulate lost BPF samples (PERF_RECORD_LOST) in BufferedPoller. */
  void debug_bpf_lost_samples();
#endif

private:
  class Callbacks : public channel::Callbacks {
  public:
    Callbacks(KernelCollector &collector);
    virtual u32 received_data(const u8 *data, int data_len) override;
    virtual void on_error(int err) override;
    virtual void on_closed() override;
    virtual void on_connect() override;

  private:
    KernelCollector &collector_;
  };

  /* sends info from config (if present) and AWS state */
  void send_connection_metadata();

  /* cleans up shared pointers */
  void cleanup_pointers();

  // enter try_connecting state - will take `discount` time out of
  // the hold-off
  void enter_try_connecting(std::chrono::milliseconds discount = 0ms);

  /* connecting state, while Tcp is trying to connect and we can time out */
  void enter_connecting();

  /* probe holdoff. got upstream connection and waiting before adding probes */
  void enter_probe_holdoff();

  /* enter polling steady state */
  void enter_polling_state();

  /* stops all timers */
  void stop_all_timers();

  /* receive data from connection */
  void received_data(const u8 *data, int data_len);

  /* handles command received from the server */
  void handle_received_command(u64 command);

  /* sends a heartbeat message to the server */
  void send_heartbeat();

  void on_connected();
  void on_error(int error);

  /* called to restart the KernelCollector */
  void restart();

private:
  /* parameters for establishing connections */
  BpfConfiguration const bpf_config_;
  config::IntakeConfig const &intake_config_;
  AwsMetadata const *aws_metadata_;
  GcpInstanceMetadata const *gcp_metadata_;
  const std::map<std::string, std::string> config_labels_;
  HostInfo const host_info_;

  // Following 2 variables handle the command received from server.
  //
  // |received_command_| holds the paritial command received so far, of
  // |recieved_length_| bytes.
  // When |recieved_length_| == max_command_length, handle_recieved_command()
  // function is invoked.
  u64 received_command_ = 0;
  int recieved_length_ = 0;

  bool disabled_ = false;

  /* lib_uv objects */
  uv_loop_t &loop_;
  uv_timer_t try_connecting_timer_;
  uv_timer_t connection_timeout_;
  uv_timer_t probe_holdoff_timer_;
  uv_timer_t polling_timer_;
  uv_timer_t slow_timer_;

  u64 last_lost_count_;
  std::unique_ptr<::ebpf_net::ingest::Encoder> encoder_;
  std::optional<BPFHandler> bpf_handler_;
  Callbacks callbacks_;
  std::unique_ptr<channel::NetworkChannel> primary_channel_;
  channel::FileChannel secondary_channel_;
  channel::UpstreamConnection upstream_connection_;
  ::ebpf_net::ingest::Writer writer_;

  u64 last_probe_monotonic_time_ns_;

  // is the agent in healthy steady state -- so we can log when not healthy
  bool is_connected_;

  CurlEngine &curl_engine_;

  scheduling::IntervalScheduler heartbeat_sender_;

  /* enable/disable features */
  bool enable_http_metrics_;
  u64 socket_stats_interval_sec_;
  CgroupHandler::CgroupSettings const cgroup_settings_;

  FileDescriptor bpf_dump_file_;
  logging::Logger log_;

  KernelCollectorRestarter kernel_collector_restarter_;
};
