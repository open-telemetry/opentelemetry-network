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

#include <config.h>
#include <util/log.h>

#include <collector/kernel/bpf_handler.h>
#include <collector/kernel/cgroup_prober.h>
#include <collector/kernel/nat_prober.h>
#include <collector/kernel/process_prober.h>
#include <collector/kernel/socket_prober.h>

BPFHandler::BPFHandler(
    uv_loop_t &loop,
    std::string full_program,
    bool enable_http_metrics,
    bool enable_userland_tcp,
    FileDescriptor &bpf_dump_file,
    logging::Logger &log,
    ::flowmill::ingest::Encoder *encoder)
    : loop_(loop),
      bpf_module_(0),
      perf_(),
      encoder_(encoder),
      buf_poller_(nullptr),
      enable_http_metrics_(enable_http_metrics),
      enable_userland_tcp_(enable_userland_tcp),
      bpf_dump_file_(bpf_dump_file),
      log_(log),
      last_lost_count_(0)
{
  if (enable_userland_tcp) {
    full_program = "#define ENABLE_TCP_DATA_STREAM 1\n" + full_program;
  }
  int res = probe_handler_.start_bpf_module(full_program, bpf_module_, perf_);
  if (res != 0) {
    throw std::system_error(errno, std::generic_category(), "ProbeHandler couldn't load BPFModule");
  }
}

BPFHandler::~BPFHandler()
{
  probe_handler_.cleanup_probes();
  probe_handler_.cleanup_tail_calls(bpf_module_);
  buf_poller_.reset();
}

void BPFHandler::load_buffered_poller(
    IBufferedWriter &buffered_writer,
    u64 boot_time_adjustment,
    CurlEngine &curl_engine,
    u64 socket_stats_interval_sec,
    CgroupHandler::CgroupSettings const &cgroup_settings,
    ProcessHandler::CpuMemIoSettings const *cpu_mem_io_settings,
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
      bpf_module_,
      socket_stats_interval_sec,
      cgroup_settings,
      cpu_mem_io_settings,
      encoder_,
      kernel_collector_restarter);
  last_lost_count_ = serv_lost_count();
}

void BPFHandler::load_probes(::flowmill::ingest::Writer &writer)
{
  CgroupProber cgroup_prober(
      probe_handler_,
      bpf_module_,
      [this]() { buf_poller_->start(1, 1); },
      [this](std::string error_loc) { check_cb(error_loc); });

  if (cgroup_prober.error_count() > 0) {
    log_.warn("load_probes could not close {} directories", cgroup_prober.error_count());
  }

  /* Start instrumentation for processes */
  ProcessProber process_prober(
      probe_handler_,
      bpf_module_,
      [this]() { buf_poller_->start(1, 1); },
      [this](std::string error_loc) { check_cb(error_loc); });

  NatProber nat_prober(probe_handler_, bpf_module_, [this]() { buf_poller_->start(1, 1); });

  // one more poll to make sure the perf rings are clear
  buf_poller_->start(1, 1);

  // send a process_steady_state message
  writer.process_steady_state(0);

  // UDP (v4+v6) DNS Replies
  // And receive udp statistics

  // skb_consume_udp was introduced in f970bd9e3a06f, i.e.,
  // v4.10-rc1~202^2~423^2~1
  int udp_status = probe_handler_.start_probe(bpf_module_, "on_skb_consume_udp", "skb_consume_udp");
  if (udp_status != 0) {
    log_.warn("DNS probing alternative #1 failed, trying approach #2 (usually "
              "v4.7-v4.9 kernels)");
    // __skb_free_datagram_locked was introduced in 627d2d6b55009, i.e.,
    // v4.7-rc1~154^2~349^2
    udp_status = probe_handler_.start_probe(bpf_module_, "on_skb_free_datagram_locked", "__skb_free_datagram_locked");
  }
  if (udp_status != 0) {
    log_.warn("DNS probing alternative #2 failed, trying approach #3 (usually "
              "pre-4.7 kernels)");
    // skb_free_datagram_locked exists in udp_recvmsg since 9d410c7960676, i.e.,
    // v2.6.32-rc6~9^2~3
    udp_status = probe_handler_.start_probe(bpf_module_, "on_skb_free_datagram_locked", "skb_free_datagram_locked");
  }
  if (udp_status != 0) {
    log_.error("Could not instrument DNS");
  } else {
    LOG::trace("DNS instrumentation active");
  }

  // Start instrumentation for sockets
  SocketProber socket_prober(
      probe_handler_,
      bpf_module_,
      [this]() { buf_poller_->start(1, 1); },
      [this](std::string error_loc) { check_cb(error_loc); },
      log_);

  // one more poll to make sure the perf rings are clear
  buf_poller_->start(1, 1);

  // send a socket_steady_state message
  writer.socket_steady_state(0);

  // probe for steady-state data
  int rtt_status = probe_handler_.start_probe(bpf_module_, "on_tcp_rtt_estimator", "tcp_rtt_estimator");
  if (rtt_status) { // if tcp_rtt_estimator fails
    log_.warn(
        "cannot start tcp_rtt_estimator probe, trying "
        "tcp_update_pacing_rate. error {}",
        rtt_status);

    rtt_status = probe_handler_.start_probe(bpf_module_, "on_tcp_rtt_estimator", "tcp_update_pacing_rate");
    if (rtt_status) {
      log_.warn("cannot start tcp_update_pacing_rate probe, trying tcp_ack. error {}", rtt_status);

      rtt_status = probe_handler_.start_probe(bpf_module_, "on_tcp_rtt_estimator", "tcp_ack");
      if (rtt_status) {
        log_.warn("cannot start tcp_ack probe. error {}", rtt_status);
      }
    }
  }

  // SYN timeouts
  int syn_timeout = probe_handler_.start_probe(bpf_module_, "on_tcp_retransmit_timer", "tcp_retransmit_timer");
  if (syn_timeout) {
    log_.warn("cannot instrument SYN timeouts. error {}", syn_timeout);
  }

  // SYN-ACK timeouts
  int syn_ack_timeout = probe_handler_.start_probe(bpf_module_, "on_tcp_syn_ack_timeout", "tcp_syn_ack_timeout");
  if (syn_ack_timeout) {
    log_.warn("cannot instrument SYN-ACK timeouts. error {}", syn_ack_timeout);
  }

  // TCP resets
  int tcp_reset_ret = probe_handler_.start_probe(bpf_module_, "on_tcp_reset", "tcp_reset");
  if (tcp_reset_ret) {
    log_.warn("cannot instrument tcp_reset function: error {}", tcp_reset_ret);
  }
  int tcp_send_active_reset_ret = probe_handler_.start_probe(bpf_module_, "on_tcp_send_active_reset", "tcp_send_active_reset");
  if (tcp_send_active_reset_ret) {
    log_.warn("cannot instrument tcp_send_active_reset function: error {}", tcp_send_active_reset_ret);
  }

  buf_poller_->start(1, 1);
  check_cb("loading rtt probes");

  /**
   *  TODO: is it possible we're missing events from the new sock notification
   *   to the point we start the instrumentation below? we read some socket
   *   recv state when opening the socket
   */
  int ret;
  ret = probe_handler_.start_probe(bpf_module_, "on_tcp_event_data_recv", "tcp_event_data_recv");
  if (ret) {
    log_.warn("start tcp_event_data_recv probe failed. error {}", ret);
  }
  ret = probe_handler_.start_probe(bpf_module_, "on_tcp_rcv_established", "tcp_rcv_established");
  if (ret) {
    log_.warn("start tcp_rcv_established probe failed. error {}", ret);
  }
  buf_poller_->start(1, 1);
  check_cb("loading tcp steady-state probes");

  // set up tail calls table
  if (probe_handler_.register_tail_call(bpf_module_, "tail_calls", TAIL_CALL_ON_UDP_SEND_SKB__2, "on_udp_send_skb__2") != 0) {
    log_.error("failed to register tail call for on_udp_send_skb__2");
  }
  if (probe_handler_.register_tail_call(bpf_module_, "tail_calls", TAIL_CALL_ON_UDP_V6_SEND_SKB__2, "on_udp_v6_send_skb__2") !=
      0) {
    log_.error("failed to register tail call for on_udp_v6_send_skb__2");
  }
  if (probe_handler_.register_tail_call(bpf_module_, "tail_calls", TAIL_CALL_ON_IP_SEND_SKB__2, "on_ip_send_skb__2") != 0) {
    log_.error("failed to register tail call for on_ip_send_skb__2");
  }
  if (probe_handler_.register_tail_call(bpf_module_, "tail_calls", TAIL_CALL_ON_IP6_SEND_SKB__2, "on_ip6_send_skb__2") != 0) {
    log_.error("failed to register tail call for on_ip6_send_skb__2");
  }
  if (probe_handler_.register_tail_call(
          bpf_module_, "tail_calls", TAIL_CALL_HANDLE_RECEIVE_UDP_SKB, "handle_receive_udp_skb") != 0) {
    log_.error("failed to register tail call for handle_receive_udp_skb");
  }
  if (probe_handler_.register_tail_call(
          bpf_module_, "tail_calls", TAIL_CALL_HANDLE_RECEIVE_UDP_SKB__2, "handle_receive_udp_skb__2") != 0) {
    log_.error("failed to register tail call for handle_receive_udp_skb__2");
  }
  if (probe_handler_.register_tail_call(bpf_module_, "tail_calls", TAIL_CALL_CONTINUE_TCP_SENDMSG, "continue_tcp_sendmsg") !=
      0) {
    log_.error("failed to register tail call for continue_tcp_sendmsg");
  }
  if (probe_handler_.register_tail_call(bpf_module_, "tail_calls", TAIL_CALL_CONTINUE_TCP_RECVMSG, "continue_tcp_recvmsg") !=
      0) {
    log_.error("failed to register tail call for continue_tcp_recvmsg");
  }

  // udp v4 send statistics and dns requests
  int udp_send_skb_status = probe_handler_.start_probe(bpf_module_, "on_udp_send_skb", "udp_send_skb");
  if (udp_send_skb_status != 0) {
    log_.warn("udp_send_skb probe failed, using ip_send_skb hook. error {}", udp_send_skb_status);
    int ip_send_skb_status = probe_handler_.start_probe(bpf_module_, "on_ip_send_skb", "ip_send_skb");
    if (ip_send_skb_status != 0) {
      log_.warn("ip_send_skb probe failed. error {}", ip_send_skb_status);
    }
  }

  // udp v6 send statistics and dns requests
  int udp_v6_send_skb_status = probe_handler_.start_probe(bpf_module_, "on_udp_v6_send_skb", "udp_v6_send_skb");
  if (udp_v6_send_skb_status != 0) {
    log_.warn("udp_v6_send_skb probe failed, using ip6_send_skb hook. error {}", udp_v6_send_skb_status);
    int ip6_send_skb_status = probe_handler_.start_probe(bpf_module_, "on_ip6_send_skb", "ip6_send_skb");
    if (ip6_send_skb_status != 0) {
      log_.warn("ip6_send_skb probe failed. error {}", ip6_send_skb_status);
    }
  }

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

    probe_handler_.start_probe(bpf_module_, "handle_kprobe__tcp_init_sock", "tcp_init_sock", "_tcpproc");
    probe_handler_.start_probe(bpf_module_, "handle_kprobe__security_sk_free", "security_sk_free", "_tcpproc");
    probe_handler_.start_kretprobe(bpf_module_, "handle_kretprobe__inet_csk_accept", "inet_csk_accept", "_tcpproc");
    probe_handler_.start_probe(bpf_module_, "handle_kprobe__inet_csk_accept", "inet_csk_accept", "_tcpproc");
    probe_handler_.start_kretprobe(bpf_module_, "handle_kretprobe__tcp_sendmsg", "tcp_sendmsg", "_tcpproc");
    probe_handler_.start_probe(bpf_module_, "handle_kprobe__tcp_sendmsg", "tcp_sendmsg", "_tcpproc");
    probe_handler_.start_kretprobe(bpf_module_, "handle_kretprobe__tcp_recvmsg", "tcp_recvmsg", "_tcpproc");
    probe_handler_.start_probe(bpf_module_, "handle_kprobe__tcp_recvmsg", "tcp_recvmsg", "_tcpproc");

    buf_poller_->start(1, 1);
    check_cb("tcp processor probes");
  }

  buf_poller_->start(1, 1);
  check_cb("end of load_probes()");

  buf_poller_->set_all_probes_loaded();
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
