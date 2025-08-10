// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <config.h>
#include <util/log.h>

#include <collector/kernel/bpf_handler.h>
#include <collector/kernel/cgroup_prober.h>
#include <collector/kernel/nat_prober.h>
#include <collector/kernel/process_prober.h>
#include <collector/kernel/socket_prober.h>
#include <common/host_info.h>

BPFHandler::BPFHandler(
    uv_loop_t &loop,
    const BpfConfiguration &bpf_config,
    bool enable_http_metrics,
    FileDescriptor &bpf_dump_file,
    logging::Logger &log,
    ::ebpf_net::ingest::Encoder *encoder,
    HostInfo const &host_info)
    : loop_(loop),
      probe_handler_(log),
      bpf_skel_(nullptr),
      perf_(),
      encoder_(encoder),
      buf_poller_(nullptr),
      enable_http_metrics_(enable_http_metrics),
      bpf_dump_file_(bpf_dump_file),
      log_(log),
      last_lost_count_(0),
      host_info_(host_info)
{
  // Open the BPF skeleton (doesn't load yet)
  bpf_skel_ = probe_handler_.open_bpf_skeleton();
  if (!bpf_skel_) {
    throw std::system_error(errno, std::generic_category(), "ProbeHandler couldn't open BPF skeleton");
  }

  // Configure global variables before loading
  probe_handler_.configure_bpf_skeleton(bpf_skel_, bpf_config);

  // Now load the skeleton and set up perf rings
  int res = probe_handler_.load_bpf_skeleton(bpf_skel_, perf_);
  if (res != 0) {
    probe_handler_.destroy_bpf_skeleton(bpf_skel_);
    bpf_skel_ = nullptr;
    throw std::system_error(errno, std::generic_category(), "ProbeHandler couldn't load BPF skeleton");
  }
}

BPFHandler::~BPFHandler()
{
  probe_handler_.cleanup_probes();
  probe_handler_.cleanup_tail_calls(bpf_skel_);
  buf_poller_.reset();
  if (bpf_skel_) {
    probe_handler_.destroy_bpf_skeleton(bpf_skel_);
    bpf_skel_ = nullptr;
  }
}

void BPFHandler::load_buffered_poller(
    IBufferedWriter &buffered_writer,
    u64 boot_time_adjustment,
    CurlEngine &curl_engine,
    u64 socket_stats_interval_sec,
    CgroupHandler::CgroupSettings const &cgroup_settings,
    KernelCollectorRestarter &kernel_collector_restarter)
{
  LOG::trace("--- Starting BufferedPoller ---");
  buf_poller_ = std::make_unique<BufferedPoller>(
      loop_,
      perf_,
      buffered_writer,
      boot_time_adjustment,
      curl_engine,
      bpf_dump_file_,
      log_,
      probe_handler_,
      bpf_skel_,
      socket_stats_interval_sec,
      cgroup_settings,
      encoder_,
      kernel_collector_restarter);
  last_lost_count_ = serv_lost_count();
}

void BPFHandler::load_probes(::ebpf_net::ingest::Writer &writer)
{
  probe_handler_.load_kernel_symbols();

  CgroupProber cgroup_prober(
      probe_handler_,
      bpf_skel_,
      host_info_,
      [this]() { buf_poller_->start(1, 1); },
      [this](std::string error_loc) { check_cb(error_loc); });

  if (cgroup_prober.error_count() > 0) {
    log_.warn("load_probes could not close {} directories", cgroup_prober.error_count());
  }

  /* Start instrumentation for processes */
  ProcessProber process_prober(
      probe_handler_,
      bpf_skel_,
      [this]() { buf_poller_->start(1, 1); },
      [this](std::string error_loc) { check_cb(error_loc); });

  NatProber nat_prober(probe_handler_, bpf_skel_, [this]() { buf_poller_->start(1, 1); });

  // one more poll to make sure the perf rings are clear
  buf_poller_->start(1, 1);

  // send a process_steady_state message
  writer.process_steady_state(0);

  // UDP (v4+v6) DNS Replies
  // And receive udp statistics
  ProbeAlternatives dns_probe_alternatives{
      "DNS",
      {
          // skb_consume_udp was introduced in f970bd9e3a06f, i.e.,
          // v4.10-rc1~202^2~423^2~1
          {"on_skb_consume_udp", "skb_consume_udp"},
          // __skb_free_datagram_locked was introduced in 627d2d6b55009, i.e.,
          // v4.7-rc1~154^2~349^2
          {"on___skb_free_datagram_locked", "__skb_free_datagram_locked"},
          // skb_free_datagram_locked exists in udp_recvmsg since 9d410c7960676, i.e.,
          // v2.6.32-rc6~9^2~3
          {"on_skb_free_datagram_locked", "skb_free_datagram_locked"},
      }};
  probe_handler_.start_probe(bpf_skel_, dns_probe_alternatives);

  // Start instrumentation for sockets
  SocketProber socket_prober(
      probe_handler_,
      bpf_skel_,
      [this]() { buf_poller_->start(1, 1); },
      [this](std::string error_loc) { check_cb(error_loc); },
      log_);

  // one more poll to make sure the perf rings are clear
  buf_poller_->start(1, 1);

  // send a socket_steady_state message
  writer.socket_steady_state(0);

  // probe for steady-state data
  ProbeAlternatives probe_alternatives{
      "tcp rtt estimator",
      {
          {"on_tcp_rtt_estimator", "tcp_rtt_estimator"},
          {"on_tcp_rtt_estimator", "tcp_update_pacing_rate"},
          {"on_tcp_rtt_estimator", "tcp_ack"},
      }};
  probe_handler_.start_probe(bpf_skel_, probe_alternatives);

  // SYN timeouts
  probe_handler_.start_probe(bpf_skel_, "on_tcp_retransmit_timer", "tcp_retransmit_timer");

  // SYN-ACK timeouts
  probe_handler_.start_probe(bpf_skel_, "on_tcp_syn_ack_timeout", "tcp_syn_ack_timeout");

  // TCP resets
  probe_handler_.start_probe(bpf_skel_, "on_tcp_reset", "tcp_reset");
  probe_handler_.start_probe(bpf_skel_, "on_tcp_send_active_reset", "tcp_send_active_reset");

  buf_poller_->start(1, 1);
  check_cb("loading rtt probes");

  /**
   *  TODO: is it possible we're missing events from the new sock notification
   *   to the point we start the instrumentation below? we read some socket
   *   recv state when opening the socket
   */
  probe_handler_.start_probe(bpf_skel_, "on_tcp_event_data_recv", "tcp_event_data_recv");
  probe_handler_.start_probe(bpf_skel_, "on_tcp_rcv_established", "tcp_rcv_established");
  buf_poller_->start(1, 1);
  check_cb("loading tcp steady-state probes");

  // set up tail calls table
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_ON_UDP_SEND_SKB__2, "on_udp_send_skb__2");
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_ON_UDP_V6_SEND_SKB__2, "on_udp_v6_send_skb__2");
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_ON_IP_SEND_SKB__2, "on_ip_send_skb__2");
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_ON_IP6_SEND_SKB__2, "on_ip6_send_skb__2");
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_HANDLE_RECEIVE_UDP_SKB, "handle_receive_udp_skb");
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_HANDLE_RECEIVE_UDP_SKB__2, "handle_receive_udp_skb__2");
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_CONTINUE_TCP_SENDMSG, "continue_tcp_sendmsg");
  probe_handler_.register_tail_call(bpf_skel_, "tail_calls", TAIL_CALL_CONTINUE_TCP_RECVMSG, "continue_tcp_recvmsg");

  // udp v4 send statistics and dns requests
  ProbeAlternatives udp_v4_alternatives{
      "udp v4 send skb",
      {
          {"on_udp_send_skb", "udp_send_skb"},
          {"on_ip_send_skb", "ip_send_skb"},
      }};
  probe_handler_.start_probe(bpf_skel_, udp_v4_alternatives);

  // udp v6 send statistics and dns requests
  ProbeAlternatives udp_v6_alternatives{
      "udp v6 send skb",
      {
          {"on_udp_v6_send_skb", "udp_v6_send_skb"},
          {"on_ip6_send_skb", "ip6_send_skb"},
      }};
  probe_handler_.start_probe(bpf_skel_, udp_v6_alternatives);

  // start the udp probes
  buf_poller_->start(1, 1);
  check_cb("loading udp steady-state probes");

  // Probes for tcp-processor
  if (enable_http_metrics_ /*|| any other tcp-processor flags eventually */) {

    // Using _tcpproc as the event suffix to ensure we don't collide our kprobes
    // with anything in 'render_bpf'
    //
    // Keep kretprobes registered before kprobes to the same function to avoid
    // races

    LOG::debug("Adding TCP processor probes");

    probe_handler_.start_probe(bpf_skel_, "handle_kprobe__tcp_init_sock", "tcp_init_sock", "_tcpproc");
    probe_handler_.start_probe(bpf_skel_, "handle_kprobe__security_sk_free", "security_sk_free", "_tcpproc");
    probe_handler_.start_kretprobe(bpf_skel_, "handle_kretprobe__inet_csk_accept", "inet_csk_accept", "_tcpproc");
    probe_handler_.start_probe(bpf_skel_, "handle_kprobe__inet_csk_accept", "inet_csk_accept", "_tcpproc");
    probe_handler_.start_kretprobe(bpf_skel_, "handle_kretprobe__tcp_sendmsg", "tcp_sendmsg", "_tcpproc");
    probe_handler_.start_probe(bpf_skel_, "handle_kprobe__tcp_sendmsg", "tcp_sendmsg", "_tcpproc");
    probe_handler_.start_kretprobe(bpf_skel_, "handle_kretprobe__tcp_recvmsg", "tcp_recvmsg", "_tcpproc");
    probe_handler_.start_probe(bpf_skel_, "handle_kprobe__tcp_recvmsg", "tcp_recvmsg", "_tcpproc");

    buf_poller_->start(1, 1);
    check_cb("tcp processor probes");
  }

  buf_poller_->start(1, 1);
  check_cb("end of load_probes()");

  buf_poller_->set_all_probes_loaded();

  probe_handler_.clear_kernel_symbols();
}

void BPFHandler::start_poll(u64 interval_useconds, u64 n_intervals)
{
  buf_poller_->start(interval_useconds, n_intervals);
}

void BPFHandler::slow_poll()
{
  buf_poller_->slow_poll();
}

u64 BPFHandler::serv_lost_count()
{
  return buf_poller_->serv_lost_count();
}

void BPFHandler::check_cb(std::string error_loc)
{
  u64 lost_count = serv_lost_count();
  if (lost_count > last_lost_count_) {
    u64 diff_lost_count = lost_count - last_lost_count_;
    last_lost_count_ = lost_count;
    log_.warn("after {}, cumulative lost count non-zero: {} new, {} total", error_loc, diff_lost_count, lost_count);
  }
}

#ifndef NDEBUG
void BPFHandler::debug_bpf_lost_samples()
{
  if (buf_poller_) {
    buf_poller_->debug_bpf_lost_samples();
  }
}
#endif
