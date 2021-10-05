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

#include <util/curl_engine.h>
#include <util/fast_div.h>
#include <util/file_ops.h>
#include <util/logger.h>
#include <util/metric_store.h>

#include <channel/buffered_writer.h>
#include <collector/kernel/bpf_src/render_bpf.h>
#include <collector/kernel/cgroup_handler.h>
#include <collector/kernel/dns_requests.h>
#include <collector/kernel/nat_handler.h>
#include <collector/kernel/nic_poller.h>
#include <collector/kernel/perf_poller.h>
#include <collector/kernel/probe_handler.h>
#include <collector/kernel/process_handler.h>
#include <collector/kernel/socket_table.h>
#include <collector/kernel/tcp_data_handler.h>
#include <generated/flowmill/agent_internal/hash.h>
#include <generated/flowmill/ingest/encoder.h>
#include <generated/flowmill/ingest/writer.h>
#include <generated/flowmill/kernel_collector/index.h>

#include <memory>

class KernelCollectorRestarter;

/**
 * A BufferedPoller that empties the perf ring every poll
 * and sends the info to a specified channel
 */
class BufferedPoller : public PerfPoller {
public:
  // The hash map needs to be some 20% larger than the max number of elements.
  // Otherwise you'd start getting hash failures before all the elements are
  // exhausted
  static constexpr u32 tcp_socket_table_max_sockets = TABLE_SIZE__TCP_OPEN_SOCKETS;
  static constexpr u32 udp_socket_table_max_sockets = TABLE_SIZE__UDP_OPEN_SOCKETS;

  static constexpr u32 n_epochs = 2;

  /**
   * c'tor
   * throws if buff_ can't be malloc-ed
   * @param loop: the libuv event loop on which to receive perf events
   * @param container: the perf container to extract messages from
   * @param writer: the writer using which to send messages
   * @param time_adjustment: how much to add to CLOCK_MONOTONIC when comparing
   *   to ring timestamp
   */
  BufferedPoller(
      uv_loop_t &loop,
      PerfContainer &container,
      IBufferedWriter &writer,
      u64 time_adjustment,
      CurlEngine &curl_engine,
      FileDescriptor &bpf_dump_file,
      logging::Logger &log,
      ProbeHandler &probe_handler,
      ebpf::BPFModule &bpf_module,
      NicPoller &nic_poller,
      CgroupHandler::CgroupSettings const &cgroup_settings,
      ProcessHandler::CpuMemIoSettings const *cpu_mem_io_settings,
      ::flowmill::ingest::Encoder *encoder,
      KernelCollectorRestarter &kernel_collector_restarter);

  /**
   * batches as many entries into buffer as possible before calling
   * send_buffer(). always flushes the buffer at the end.
   *
   * there are four possible cases where it will throw:
   * 1) a record is larger than an empty buffer
   * 2) a record of unknown type was read
   * 3) call to send_buffer failed (there are two call locations)
   *
   */
  void process_samples(bool is_event);

  /**
   * entrypoint for event-driven polling when the perf container announces
   * it is half full or whatever notification limit is set.
   * calls through to process_samples()
   */
  void handle_event();

  /**
   * entrypoint for manual polling. calls through to process_samples()
   * @see PerfPoller::poll
   */

  virtual void poll();

  /**
   * Sends a bpf loss notification to backend, if a loss happened since
   *   the last notification call
   */
  void send_report_if_recent_loss();

  /**
   * accessor for lost_count_
   */
  u64 serv_lost_count();

  void slow_poll();

  /**
   * let us know when all the probes are loaded
   * and have achieved steady-state
   */
  void set_all_probes_loaded(void);

#ifndef NDEBUG
  /**
   * Debug code for internal development to simulate lost BPF samples (PERF_RECORD_LOST) in BufferedPoller.
   */
  void debug_bpf_lost_samples();
#endif

private:
  /**
   * polling point for dns timeout detection
   * called via slow poll
   */
  void process_dns_timeouts(u64 t);

  /**
   * A message handler is a member function of this class
   * @returns true if message should be copied, false otherwise
   */
  typedef void (BufferedPoller::*handler_fn)(PerfReader &reader, u16 length);

  struct message_metadata {
    u64 timestamp;
    std::size_t cpu_index;
    std::basic_string_view<u8> payload;
    std::basic_string_view<u8> padding;
  };

  template <typename MessageMetadata>
  using message_handler_fn =
      void (BufferedPoller::*)(message_metadata const &metadata, typename MessageMetadata::wire_message &);

  template <typename MessageMetadata, message_handler_fn<MessageMetadata>, std::size_t MaxPadding, typename Alignment>
  void message_handler_entrypoint(PerfReader &reader, u16 length);

  /**
   * Adds a handler to the hash. Throws on collision.
   */
  template <typename MessageMetadata, message_handler_fn<MessageMetadata>, std::size_t MaxPadding = 0, typename Alignment = u64>
  void add_handler();

  /**
   * Handler for DNS RPC messages
   */
  void handle_dns_message(message_metadata const &metadata, jb_agent_internal__dns_packet &msg);

  /**
   * Handler a new socket message
   */
  void handle_new_socket(message_metadata const &metadata, jb_agent_internal__new_sock_created &msg);

  /**
   * Handle a ipv4 address for a socket
   */
  void handle_set_state_ipv4(message_metadata const &metadata, jb_agent_internal__set_state_ipv4 &msg);

  /**
   * Handle ipv6 address for a socket
   */
  void handle_set_state_ipv6(message_metadata const &metadata, jb_agent_internal__set_state_ipv6 &msg);

  /**
   * Handler a socket close message
   */
  void handle_close_socket(message_metadata const &metadata, jb_agent_internal__close_sock_info &msg);

  /**
   * Handler a rtt_estimator telemetry message
   */
  void handle_rtt_estimator(message_metadata const &metadata, jb_agent_internal__rtt_estimator &msg);

  /**
   * Handler a rtt_estimator telemetry message
   */
  void handle_reset_tcp_counters(message_metadata const &metadata, jb_agent_internal__reset_tcp_counters &msg);

  /**
   * Handle a TCP SYN timeout message
   */
  void handle_tcp_syn_timeout(message_metadata const &metadata, jb_agent_internal__tcp_syn_timeout &msg);

  /**
   * Handle TCP RST
   */
  void handle_tcp_reset(message_metadata const &metadata, jb_agent_internal__tcp_reset &msg);

  /**
   * Handle a http_response message
   */
  void handle_http_response(message_metadata const &metadata, jb_agent_internal__http_response &msg);

  /**
   * Sends a message with statistics for the entry
   *
   * Also marks the entry as invalid.
   */
  void send_socket_stats(u64 t, u64 sk, tcp_statistics &stats);

  /**
   * Processes the current queue in socket_stats_, sending out messages and
   *   setting entries to stats.queued=false, stats.valid=false, then advances
   *   the queue.
   */
  void send_stats_from_queue(u64 t);

  /*** UDP ***/
  /**
   * Handler a new or existing udp socket message
   */
  void handle_udp_new_socket(message_metadata const &metadata, jb_agent_internal__udp_new_socket &msg);

  /**
   * Handler a UDP socket close message
   */
  void handle_udp_destroy_socket(message_metadata const &metadata, jb_agent_internal__udp_destroy_socket &msg);

  /**
   * Handler for udp TX notification
   */
  void handle_udp_stats(message_metadata const &metadata, jb_agent_internal__udp_stats &msg);

  /**
   * Handler for new process
   */
  void handle_pid_info(message_metadata const &metadata, jb_agent_internal__pid_info &msg);

  /**
   * Handler for close process
   */
  void handle_pid_close(message_metadata const &metadata, jb_agent_internal__pid_close &msg);

  /**
   * Handler for process comm change
   */
  void handle_pid_set_comm(message_metadata const &metadata, jb_agent_internal__pid_set_comm &msg);

  /**
   * Handler for process cpu time
   */
  void handle_report_task_status(message_metadata const &metadata, jb_agent_internal__report_task_status &msg);

  /**
   * Handler for process exit
   */
  void handle_pid_exit(message_metadata const &metadata, jb_agent_internal__pid_exit &msg);

  /**
   * Sends a udp socket message
   *
   * Only sends when lport is != 0
   */
  void udp_send_new_socket(u64 ts, udp_socket_entry *entry, u64 index);

  /**
   * Sends a message with statistics for the entry
   *
   * Also marks the entry as invalid.
   * @assumes entry is valid
   */
  void udp_send_stats(u64 t, u32 sk_id, u8 is_rx, udp_socket_entry &entry, udp_statistics &stats);

  /**
   * Processes the current queue in socket_stats_, sending out messages and
   *   setting entries to stats.queued=false, stats.valid=false, then advances
   *   the queue.
   */
  void udp_send_stats_from_queue(u64 t);

  /*** CONTAINERS ***/
  /**
   * Handler for a new cgroup dir
   */
  void handle_kill_css(message_metadata const &metadata, jb_agent_internal__kill_css &msg);

  void handle_css_populate_dir(message_metadata const &metadata, jb_agent_internal__css_populate_dir &msg);

  void handle_cgroup_clone_children_read(message_metadata const &metadata, jb_agent_internal__cgroup_clone_children_read &msg);

  void handle_cgroup_attach_task(message_metadata const &metadata, jb_agent_internal__cgroup_attach_task &msg);

  /*** NAT ***/
  void handle_nf_nat_cleanup_conntrack(message_metadata const &metadata, jb_agent_internal__nf_nat_cleanup_conntrack &msg);

  void handle_nf_conntrack_alter_reply(message_metadata const &metadata, jb_agent_internal__nf_conntrack_alter_reply &msg);

  void handle_existing_conntrack_tuple(message_metadata const &metadata, jb_agent_internal__existing_conntrack_tuple &msg);

  /*** DNS ***/
  void timeout_dns_request(u64 timestamp_ns, const DnsRequests::Request &req);

  /*** ERRORS ***/
  void handle_bpf_log(message_metadata const &metadata, jb_agent_internal__bpf_log &msg);
  void handle_stack_trace(message_metadata const &metadata, jb_agent_internal__stack_trace &msg);

  /*** TCP DATA ***/
  void handle_tcp_data(message_metadata const &metadata, jb_agent_internal__tcp_data &msg);

  void handle_nic_queue_state(message_metadata const &metadata, jb_agent_internal__nic_queue_state &msg);

  /////////////////////////////////////////////////////////////////////////

  uv_loop_t &loop_;

  u64 time_adjustment_;
  FileDescriptor &bpf_dump_file_;
  logging::Logger &log_;
  IBufferedWriter &buffered_writer_;
  ProbeHandler &probe_handler_;
  ebpf::BPFModule &bpf_module_;
  ::flowmill::ingest::Writer writer_;
  std::unique_ptr<TCPDataHandler> tcp_data_handler_;
  ::flowmill::kernel_collector::Index collector_index_;
  ProcessHandler process_handler_;
  u64 lost_count_ = 0;
  u32 pid_count_ = 0;

  /* the last lost count that a message was sent for */
  u64 notified_lost_count_ = 0;

  handler_fn handlers_[AGENT_INTERNAL_HASH_SIZE];

  /* u64 Hasher */
  struct u64_hasher {
    u32 operator()(u64 const &s) const noexcept;
  };

  CgroupHandler cgroup_handler_;
  NatHandler nat_handler_;
  NicPoller &nic_poller_;

  /* TCP */
  typedef FixedHash<u64, tcp_socket_entry, tcp_socket_table_max_sockets, u64_hasher> TcpSocketTable;
  typedef MetricStore<struct tcp_statistics, tcp_socket_table_max_sockets, n_epochs> TcpSocketStatistics;

  TcpSocketTable tcp_socket_table_;
  bool tcp_socket_table_ever_full_;
  fast_div tslot_;
  TcpSocketStatistics tcp_socket_stats_;
  u64 tcp_index_to_sk_[tcp_socket_table_max_sockets];

  /* UDP */
  typedef FixedHash<u64, udp_socket_entry, udp_socket_table_max_sockets, u64_hasher> UdpSocketTable;
  typedef MetricStore<struct udp_statistics, udp_socket_table_max_sockets, n_epochs> UdpSocketStatistics;

  UdpSocketTable udp_socket_table_;
  bool udp_socket_table_ever_full_;
  std::array<UdpSocketStatistics, 2> udp_socket_stats_; /* 0: TX, 1: RX */

  /* DNS */
  DnsRequests dns_requests_;

  bool all_probes_loaded_;

  KernelCollectorRestarter &kernel_collector_restarter_;

#ifndef NDEBUG
  bool debug_bpf_lost_samples_ = false;
#endif
};
