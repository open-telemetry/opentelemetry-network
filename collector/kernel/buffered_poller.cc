// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/agent_log.h>
#include <collector/kernel/bpf_src/render_bpf.h>
#include <collector/kernel/buffered_poller.h>
#include <collector/kernel/dns/ares.h>
#include <collector/kernel/dns/dns.h>
#include <collector/kernel/kernel_collector_restarter.h>
#include <collector/kernel/perf_reader.h>
#include <collector/kernel/proc_cmdline.h>
#include <common/client_server_type.h>
#include <platform/platform.h>
#include <util/ip_address.h>
#include <util/log.h>
#include <util/lookup3.h>

#include <generated/agent_bpf_debug.inc>
#include <generated/flowmill/agent_internal.wire_message.h>
#include <generated/flowmill/agent_internal/meta.h>
#include <generated/flowmill/ingest.wire_message.h>

#include <spdlog/common.h>
#include <spdlog/fmt/bin_to_hex.h>

#include <chrono>
#include <iostream>
#include <stdexcept>

constexpr u16 DNS_MAX_PACKET_LEN = 512;

#ifdef DEBUG_PID
static BufferedPoller *singleton_ = nullptr;
#endif // DEBUG_PID

static constexpr u64 DNS_TIMEOUT_TIME_NS = 10'000'000'000ull;

BufferedPoller::BufferedPoller(
    uv_loop_t &loop,
    PerfContainer &container,
    IBufferedWriter &writer,
    u64 time_adjustment,
    CurlEngine &curl_engine,
    FileDescriptor &bpf_dump_file,
    logging::Logger &log,
    ProbeHandler &probe_handler,
    ebpf::BPFModule &bpf_module,
    u64 socket_stats_interval_sec,
    CgroupHandler::CgroupSettings const &cgroup_settings,
    ::flowmill::ingest::Encoder *encoder,
    KernelCollectorRestarter &kernel_collector_restarter)
    : PerfPoller(container),
      loop_(loop),
      time_adjustment_(time_adjustment),
      bpf_dump_file_(bpf_dump_file),
      log_(log),
      buffered_writer_(writer),
      probe_handler_(probe_handler),
      bpf_module_(bpf_module),
      writer_(buffered_writer_, monotonic, time_adjustment, encoder),
      collector_index_({writer_}),
      process_handler_(writer_, collector_index_, log_),
      cgroup_handler_(writer_, curl_engine, std::move(cgroup_settings), log),
      nat_handler_(writer_, log),
      tcp_socket_table_ever_full_(false),
      tslot_(socket_stats_interval_sec * 1e9, 16),
      tcp_socket_stats_(tslot_),
      udp_socket_table_ever_full_(false),
      udp_socket_stats_{{{tslot_}, {tslot_}}},
      all_probes_loaded_(false),
      kernel_collector_restarter_(kernel_collector_restarter)
{
  if (buffered_writer_.buf_size() < MAX_ENCODED_DNS_MESSAGE) {
    throw std::runtime_error("BufferedPoller: buf size too small for DNS");
  }

  {
    using namespace flowmill::agent_internal;

    memset(handlers_, 0, sizeof(handlers_));
    add_handler<dns_packet_message_metadata, &BufferedPoller::handle_dns_message, DNS_MAX_PACKET_LEN + 16>();
    add_handler<new_sock_created_message_metadata, &BufferedPoller::handle_new_socket>();
    add_handler<set_state_ipv4_message_metadata, &BufferedPoller::handle_set_state_ipv4>();
    add_handler<set_state_ipv6_message_metadata, &BufferedPoller::handle_set_state_ipv6>();
    add_handler<close_sock_info_message_metadata, &BufferedPoller::handle_close_socket>();
    add_handler<rtt_estimator_message_metadata, &BufferedPoller::handle_rtt_estimator>();
    add_handler<reset_tcp_counters_message_metadata, &BufferedPoller::handle_reset_tcp_counters>();
    add_handler<tcp_syn_timeout_message_metadata, &BufferedPoller::handle_tcp_syn_timeout>();
    add_handler<tcp_reset_message_metadata, &BufferedPoller::handle_tcp_reset>();
    add_handler<http_response_message_metadata, &BufferedPoller::handle_http_response>();
    add_handler<udp_new_socket_message_metadata, &BufferedPoller::handle_udp_new_socket>();
    add_handler<udp_destroy_socket_message_metadata, &BufferedPoller::handle_udp_destroy_socket>();
    add_handler<udp_stats_message_metadata, &BufferedPoller::handle_udp_stats>();
    add_handler<pid_info_message_metadata, &BufferedPoller::handle_pid_info>();
    add_handler<pid_close_message_metadata, &BufferedPoller::handle_pid_close>();
    add_handler<pid_set_comm_message_metadata, &BufferedPoller::handle_pid_set_comm>();
    add_handler<pid_exit_message_metadata, &BufferedPoller::handle_pid_exit>();
    add_handler<kill_css_message_metadata, &BufferedPoller::handle_kill_css>();
    add_handler<css_populate_dir_message_metadata, &BufferedPoller::handle_css_populate_dir>();
    add_handler<cgroup_clone_children_read_message_metadata, &BufferedPoller::handle_cgroup_clone_children_read>();
    add_handler<cgroup_attach_task_message_metadata, &BufferedPoller::handle_cgroup_attach_task>();
    add_handler<nf_nat_cleanup_conntrack_message_metadata, &BufferedPoller::handle_nf_nat_cleanup_conntrack>();
    add_handler<nf_conntrack_alter_reply_message_metadata, &BufferedPoller::handle_nf_conntrack_alter_reply>();
    add_handler<existing_conntrack_tuple_message_metadata, &BufferedPoller::handle_existing_conntrack_tuple>();
    add_handler<bpf_log_message_metadata, &BufferedPoller::handle_bpf_log>();
    add_handler<stack_trace_message_metadata, &BufferedPoller::handle_stack_trace>();
    add_handler<tcp_data_message_metadata, &BufferedPoller::handle_tcp_data>();
  }

  // Create a tcp data handler for the tcp_data message
  tcp_data_handler_ = std::make_unique<TCPDataHandler>(loop_, bpf_module, writer_, container, log_);

  // Set perf container callback for events
  container.set_callback(loop, this, [](void *ctx) { ((BufferedPoller *)ctx)->handle_event(); });

#ifdef DEBUG_PID
  singleton_ = this;
  signal(SIGUSR1, [](int) { singleton_->process_handler_.debug_pid_dump(); });
#endif // DEBUG_PID
}

void BufferedPoller::handle_event()
{
  process_samples(true);
}

void BufferedPoller::poll(void)
{
  process_samples(false);
}

void BufferedPoller::process_samples(bool is_event)
{
  u64 t = monotonic() + time_adjustment_;
  PerfReader reader(container_, t);

  // in the case of event-driven poll, print debugging information to assist
  // with understanding the profile of the perf buffers
  if (is_event && is_log_whitelisted(AgentLogKind::PERF)) {
    std::string cstr = container_.inspect();
    LOG::debug_in(AgentLogKind::PERF, "* Perf event triggered *\ncontainer_ is:\n{}\n\n", cstr);
  }

  // read the top contents of our container into our buffer
  while (!reader.empty()) {
    auto peek_type = reader.peek_type();

    auto handle_bpf_lost_samples = [this]() {
      send_report_if_recent_loss();
      log_.warn("Lost {} bpf samples - restarting kernel collector.", lost_count_);
      kernel_collector_restarter_.request_restart();
    };

#ifndef NDEBUG
    if (debug_bpf_lost_samples_) {
      lost_count_ += 1;
      handle_bpf_lost_samples();
      return;
    }
#endif

    if (peek_type == PERF_RECORD_SAMPLE) {
      if (bpf_dump_file_) {
        auto const view = reader.peek_message();
        bpf_dump_file_.write_all(view.first);
        bpf_dump_file_.write_all(view.second);
      }

      u16 length = reader.peek_unpadded_length();

      /* special handling for DNS packets */
      if (length < 10)
        throw std::runtime_error("got message < timestamp+rpc_id");

      u16 rpc_id = reader.peek_rpc_id();

      /* if rpc_id is in handlers list, call it. */
      u32 handlers_idx = agent_internal_hash(rpc_id);
      if (handlers_[handlers_idx] != nullptr) {
        (this->*handlers_[handlers_idx])(reader, length);
        continue;
      }

      /* unknown message -- this is a bug */
      throw std::runtime_error("unexpected bpf message\n");
    } else if (peek_type == PERF_RECORD_LOST) {
      lost_count_ += reader.peek_n_lost();
      reader.pop();

      handle_bpf_lost_samples();
      return;
    } else {
      throw std::runtime_error("Unexpected record type\n");
    }
  }

  /* do we need to process stats? */
  s16 relative = tcp_socket_stats_.relative_timeslot(t);
  if (relative != 0) {
    send_stats_from_queue(t);
    udp_send_stats_from_queue(t);
    send_report_if_recent_loss();
  }

  // clear out buffer if there's still anything left
  if (auto error = buffered_writer_.flush(); error && buffered_writer_.is_writable()) {
    throw std::runtime_error(fmt::format("flush failed at end: {}", error));
  }
}

void BufferedPoller::send_report_if_recent_loss()
{
  if (lost_count_ == notified_lost_count_) {
    return; // no need to report
  }

  writer_.bpf_lost_samples(lost_count_ - notified_lost_count_);
  notified_lost_count_ = lost_count_;
}

u64 BufferedPoller::serv_lost_count()
{
  return lost_count_;
}

template <
    typename MessageMetadata,
    BufferedPoller::message_handler_fn<MessageMetadata> Handler,
    std::size_t MaxPadding,
    typename Alignment>
void BufferedPoller::message_handler_entrypoint(PerfReader &reader, u16 length)
{
  struct {
    u64 timestamp;
    typename MessageMetadata::wire_message msg;
    Alignment padding[(MaxPadding + (sizeof(Alignment) - 1)) / sizeof(Alignment)];
  } in;

  if constexpr (MaxPadding) {
    if (length < sizeof(u64) + MessageMetadata::wire_message_size) {
      throw std::runtime_error(fmt::format(
          "message truncated (`{}`: {}/{})", MessageMetadata::name, length, sizeof(u64) + MessageMetadata::wire_message_size));
    } else if (length > sizeof(u64) + MessageMetadata::wire_message_size + MaxPadding) {
      throw std::runtime_error(fmt::format(
          "message too long (`{}`: {}/{})",
          MessageMetadata::name,
          length,
          sizeof(u64) + MessageMetadata::wire_message_size + MaxPadding));
    }
  } else if (length != sizeof(u64) + MessageMetadata::wire_message_size) {
    throw std::runtime_error(fmt::format(
        "invalid message length (`{}`: {}/{})",
        MessageMetadata::name,
        length,
        sizeof(u64) + MessageMetadata::wire_message_size));
  }

  // Get the cpu index we're on so we can read the right data
  // do this before the pop, so we get the right index
  std::size_t const cpu_index = reader.peek_index();

  reader.pop_and_copy_to(reinterpret_cast<char *>(&in));

  // at this point it's ok to skip the handler since the message has been
  // consumed

  // don't handle the message when disconnected
  if (!buffered_writer_.is_writable()) {
    return;
  }

  (this->*Handler)(
      {
          .timestamp = in.timestamp,
          .cpu_index = cpu_index,
          .payload = {reinterpret_cast<u8 const *>(&in), length},
          .padding = {reinterpret_cast<u8 const *>(&in.msg) + MessageMetadata::wire_message_size, MaxPadding},
      },
      in.msg);
}

template <
    typename MessageMetadata,
    BufferedPoller::message_handler_fn<MessageMetadata> Handler,
    std::size_t MaxPadding,
    typename Alignment>
void BufferedPoller::add_handler()
{
  u32 idx = agent_internal_hash(MessageMetadata::rpc_id);

  if (handlers_[idx] != nullptr) {
    throw std::runtime_error("tried to add_handler to an occupied slot");
  }

  handlers_[idx] = &BufferedPoller::message_handler_entrypoint<MessageMetadata, Handler, MaxPadding, Alignment>;
}

void BufferedPoller::handle_dns_message(message_metadata const &metadata, jb_agent_internal__dns_packet &msg)
{
  // we are assuming that struct id_addr is exactly 4 bytes and that struct
  // in6_addr is exactly 16 bytes
  //
  static_assert(
      sizeof(in_addr) == 4,
      "serialization protocol assumes IPv4 "
      "address structure is 4 bytes exactly");
  static_assert(
      sizeof(in6_addr) == 16,
      "serialization protocol assumes IPv6 address structure is 16 "
      "bytes exactly");

  LOG::debug_in(AgentLogKind::DNS, "handle_dns_message");

  u64 const sk = msg.sk;
  auto const pkt_len = msg._len - jb_agent_internal__dns_packet__data_size;
  auto const &dns_packet = metadata.padding;

  /* sanity check the message */
  if (dns_packet.data() + pkt_len > metadata.payload.data() + metadata.payload.size()) {
    throw std::runtime_error("dns: garbled message length");
  }

  // Look up the udp socket table entry
  auto pos = udp_socket_table_.find(sk);
  if (pos.index == udp_socket_table_.invalid) {
    if (!udp_socket_table_ever_full_ && all_probes_loaded_) {
      log_.error("ERROR: handle_dns_message - sk not found. sk={:x}", sk);
    }
    return;
  }
  u32 sk_id = pos.index;

  /* make variables to parse the DNS packet */
  char hostname_out[DNS_NAME_MAX_LENGTH];
  int hostname_len = 0;
  int num_ipv4_addrs = MAX_ENCODED_IP_ADDRS;
  struct in_addr ipv4_addrs[MAX_ENCODED_IP_ADDRS];
  int num_ipv6_addrs = MAX_ENCODED_IP_ADDRS;
  struct in6_addr ipv6_addrs[MAX_ENCODED_IP_ADDRS];

  /* see if this is a request */
  uint16_t type_out;
  uint16_t qid_out;
  int is_response;
  int ret = dns_parse_query(dns_packet.data(), pkt_len, &is_response, &type_out, &qid_out, hostname_out, &hostname_len);
  if (ret != ARES_SUCCESS) {
    LOG::debug_in(
        AgentLogKind::DNS,
        "dns_parse_query returned {}, total len {} valid len {} packet {:n}",
        ret,
        msg.total_len,
        pkt_len,
        spdlog::to_hex(dns_packet.data(), dns_packet.data() + pkt_len));
    return;
  }
  LOG::debug_in(
      AgentLogKind::DNS,
      "dns_parse_query successful, total len {} valid len "
      "{}\nis_response {} type_out {} qid_out {} hostname {}",
      msg.total_len,
      pkt_len,
      is_response,
      type_out,
      qid_out,
      std::string_view(hostname_out, hostname_len));

  if (!is_response) {
    DnsRequests::dns_request_key key{
        .qid = qid_out, .type = type_out, .name = std::string(hostname_out, hostname_len), .is_rx = msg.is_rx};

    // Add request to table, for later processing when response shows up
    DnsRequests::dns_request_value value{.timestamp_ns = metadata.timestamp, .sk = sk};
    dns_requests_.add(key, value);
    return;
  }

  // looking for requests in the other direction
  DnsRequests::dns_request_key key{
      .qid = qid_out, .type = type_out, .name = std::string(hostname_out, hostname_len), .is_rx = !msg.is_rx};

  // parse the reply
  int send_a_aaaa_response = 0;
  u16 sent_hostname_len = 0;
  char *sent_hostname = NULL;

  ret = dns_parse_a_aaaa_reply(
      dns_packet.data(), pkt_len, hostname_out, &hostname_len, ipv4_addrs, &num_ipv4_addrs, ipv6_addrs, &num_ipv6_addrs);

  if (hostname_len == 0 || (num_ipv4_addrs + num_ipv6_addrs) == 0) {
    LOG::debug_in(
        AgentLogKind::DNS,
        "dns_parse_a_aaaa_reply returned {}, total len {} valid len {} "
        "packet {:n}",
        ret,
        msg.total_len,
        pkt_len,
        spdlog::to_hex(dns_packet.data(), dns_packet.data() + pkt_len));
    send_a_aaaa_response = 0;
  } else {
    /**
     * we continue here even if packet was partial or corrupt, as long as we
     * successfully parsed some IP addresses and the host name
     */
    LOG::debug_in(
        AgentLogKind::DNS,
        "dns_parse_a_aaaa_reply successful, total len {} valid len "
        "{}\nnum_ipv4_addrs {} num_ipv6_addrs {}",
        msg.total_len,
        pkt_len,
        is_response,
        type_out,
        qid_out,
        std::string_view(hostname_out, hostname_len));

    send_a_aaaa_response = 1;
    // truncate hostname */
    sent_hostname_len = (hostname_len < MAX_ENCODED_DOMAIN_NAME) ? (u16)hostname_len : MAX_ENCODED_DOMAIN_NAME;
    sent_hostname = hostname_out + hostname_len - sent_hostname_len;
  }

  // Only process DNS replies have have a matching request
  // otherwise someone could be spoofing us
  std::list<DnsRequests::Request> reqs;
  dns_requests_.lookup(key, reqs);
  if (reqs.size() > 0) {

    /* see if this response matches requests we have seen */
    for (auto &req : reqs) {

      /* submit the dns response with latency information */
      u64 request_timestamp = req->second.timestamp_ns;
      u64 latency_ns = metadata.timestamp - request_timestamp;

      if (send_a_aaaa_response) {
        LOG::debug_in(
            AgentLogKind::DNS,
            "sending DNS for hostname {} num_ipv4_addrs:{} "
            "num_ipv6_addrs:{} latency_ns:{}",
            sent_hostname,
            num_ipv4_addrs,
            num_ipv6_addrs,
            latency_ns);

        // if the socket is exactly the same, then we match
        bool matching = sk == req->second.sk;
        if (!matching) {
          // if it's not, but the port and address is exactly the same, then we
          // also match
          auto pos1 = udp_socket_table_.find(sk);
          if (pos1.index == udp_socket_table_.invalid) {
            if (!udp_socket_table_ever_full_ && all_probes_loaded_) {
              log_.error("ERROR: handle_dns_message - sk not found. sk={:x}", sk);
            }
          }
          auto pos2 = udp_socket_table_.find(req->second.sk);
          if (pos2.index == udp_socket_table_.invalid) {
            if (!udp_socket_table_ever_full_ && all_probes_loaded_) {
              log_.error("ERROR: handle_dns_message - sk2 not found. sk2={}", req->second.sk);
            }
          }

          // more strict would be this, thought i'm not sure port is necessarily
          // the same in k8s environments either. matching =
          // pos1.entry->pid==pos2.entry->pid &&
          // pos1.entry->lport==pos2.entry->lport;
          matching = pos1.entry->pid == pos2.entry->pid;

          // why does the socket not match for request and response
          if (!matching) {
            LOG::debug_in(
                AgentLogKind::DNS,
                "dns socket mismatch {}@{}:{}(pid={}) != {}@{}:{}(pid={})\n"
                "hostname: {}  num_ipv4_addrs:{}  num_ipv6_addrs:{}  latency: "
                "{}",
                sk,
                IPv6Address::from(pos1.entry->laddr),
                pos1.entry->lport,
                pos1.entry->pid,
                req->second.sk,
                IPv6Address::from(pos2.entry->laddr),
                pos2.entry->lport,
                pos2.entry->pid,
                sent_hostname,
                num_ipv4_addrs,
                num_ipv6_addrs,
                latency_ns);
          }
        }

        // if receiving a dns response, this is a client and 'total time' is the
        // appropriate metric if sending a dns response, this is a server and
        // 'processing time' is the appropriate metric
        writer_.dns_response_tstamp(
            metadata.timestamp,
            sk_id,
            hostname_len,
            /* domain_name */ jb_blob{sent_hostname, sent_hostname_len},
            /* ipv4_addrs */
            jb_blob{(char *)ipv4_addrs, (u16)(sizeof(u32) * num_ipv4_addrs)},
            /* ipv6_addrs */
            jb_blob{(char *)ipv6_addrs, (u16)(sizeof(struct in6_addr) * num_ipv6_addrs)},
            latency_ns,
            msg.is_rx ? SC_CLIENT : SC_SERVER);
      }
      // else {
      //  // someday add other dns responses, or dns resolution errors
      //}
    }

    /* remove request key */
    dns_requests_.remove_all_with_key(key);
  }
}

void BufferedPoller::timeout_dns_request(u64 timestamp_ns, const DnsRequests::Request &req)
{
  u64 t_req = req->second.timestamp_ns;
  u64 sk = req->second.sk;

  // Look up the udp socket table entry
  auto pos = udp_socket_table_.find(sk);
  if (pos.index == udp_socket_table_.invalid) {
    if (!udp_socket_table_ever_full_ && all_probes_loaded_) {
      // Just a debug message here, it's possible for a udp socket lifetime to
      // race with the timeout
      LOG::debug_in(AgentLogKind::DNS, "ERROR: timeout_dns_request - sk not found. sk={:x}", sk);
    }
  } else {

    u32 sk_id = pos.index;
    u64 duration_ns = (timestamp_ns - t_req);

    /* truncate hostname */
    const char *hostname_out = req->first.name.c_str();
    size_t hostname_len = req->first.name.size();

    u16 sent_hostname_len = (hostname_len < MAX_ENCODED_DOMAIN_NAME) ? (u16)hostname_len : MAX_ENCODED_DOMAIN_NAME;

    const char *sent_hostname = hostname_out + hostname_len - sent_hostname_len;

    LOG::debug_in(AgentLogKind::DNS, "sending DNS timeout for hostname {} duration_ns {}", sent_hostname, duration_ns);

    writer_.dns_timeout_tstamp(
        timestamp_ns,
        sk_id,
        hostname_len,
        /* domain_name */ jb_blob{sent_hostname, sent_hostname_len},
        duration_ns);
  }

  // drop this request from dns_requests_
  dns_requests_.remove(req);
}

void BufferedPoller::slow_poll()
{
  u64 const t = monotonic() + time_adjustment_;
  process_dns_timeouts(t);
}

void BufferedPoller::process_dns_timeouts(u64 t)
{
  std::list<DnsRequests::Request> old_reqs;
  dns_requests_.lookup_older_than(t - DNS_TIMEOUT_TIME_NS, old_reqs);
  for (auto &req : old_reqs) {
    timeout_dns_request(t, req);
  }
}

void BufferedPoller::handle_new_socket(message_metadata const &metadata, jb_agent_internal__new_sock_created &msg)
{
  /**
   * NOTE: this is not the only handling of new sockets (contrary to name).
   *   reset_tcp_counters also reports a new socket.
   */

  LOG::debug_in(AgentLogKind::TCP, "handle_new_socket: sk={:x} pid={}", msg.sk, msg.pid);

  if (tcp_socket_table_.full()) {
    log_.warn("handle_new_socket: tcp socket table is full! dropping socket");
    tcp_socket_table_ever_full_ = true;
    return;
  }

  auto pos = tcp_socket_table_.insert(msg.sk);
  if (pos.index != tcp_socket_table_.invalid) {
    tcp_index_to_sk_[pos.index] = msg.sk;
  } else {
    log_.error("handle_new_socket: duplicate tcp socket sk={:x} pid={}", msg.sk, msg.pid);
    return;
  }

  writer_.new_sock_info_tstamp(metadata.timestamp, msg.pid, msg.sk);
}

void BufferedPoller::handle_set_state_ipv4(message_metadata const &metadata, jb_agent_internal__set_state_ipv4 &msg)
{
  LOG::trace_in(
      AgentLogKind::TCP,
      "handle_set_state_ipv4: sk:{:x}, {}:{} -> {}:{} (tx_rx={})",
      msg.sk,
      IPv4Address::from(msg.src),
      msg.sport,
      IPv4Address::from(msg.dest),
      msg.dport,
      msg.tx_rx);

  // Ensure that if we get a state_set_ipv4 that the socket is something that
  // exists in our table already
  auto pos = tcp_socket_table_.find(msg.sk);
  if (pos.index == tcp_socket_table_.invalid) {
    if (!tcp_socket_table_ever_full_ && all_probes_loaded_) {
      log_.error("handle_set_state_ipv4: socket not tracked sk={:x}", msg.sk);
    }
    return;
  }

  writer_.set_state_ipv4_tstamp(metadata.timestamp, msg.dest, msg.src, msg.dport, msg.sport, msg.sk, msg.tx_rx);

  nat_handler_.handle_set_state_ipv4(metadata.timestamp, &msg);
}

void BufferedPoller::handle_set_state_ipv6(message_metadata const &metadata, jb_agent_internal__set_state_ipv6 &msg)
{
  LOG::trace_in(
      AgentLogKind::TCP,
      "handle_set_state_ipv6: sk:{}, {}:{} -> {}:{} (tx_rx={})",
      msg.sk,
      IPv6Address::from(msg.src),
      msg.sport,
      IPv6Address::from(msg.dest),
      msg.dport,
      msg.tx_rx);

  // Ensure that if we get a state_set_ipv4 that the socket is something that
  // exists in our table already
  auto pos = tcp_socket_table_.find(msg.sk);
  if (pos.index == tcp_socket_table_.invalid) {
    if (!tcp_socket_table_ever_full_ && all_probes_loaded_) {
      log_.error("handle_set_state_ipv6: socket not tracked sk={:x}", msg.sk);
    }
    return;
  }

  writer_.set_state_ipv6_tstamp(metadata.timestamp, msg.dest, msg.src, msg.dport, msg.sport, msg.sk, msg.tx_rx);
}

void BufferedPoller::handle_close_socket(message_metadata const &metadata, jb_agent_internal__close_sock_info &msg)
{
  auto pos = tcp_socket_table_.find(msg.sk);
  if (pos.index == tcp_socket_table_.invalid) {
    // This should be prevented in BPF except when the socket table was full
    if (!tcp_socket_table_ever_full_ && all_probes_loaded_) {
      log_.error("handle_close_socket: socket not found sk={:x}", msg.sk);
    }
    return;
  }

  LOG::debug_in(AgentLogKind::TCP, "handle_close_socket: sk={:x}", msg.sk);

  // send out a statistics message if needed
  for (u32 epoch = 0; epoch < n_epochs; epoch++) {
    auto &stats = tcp_socket_stats_.lookup_relative(pos.index, epoch, false).second;
    if (stats.valid == true) {
      send_socket_stats(metadata.timestamp, msg.sk, stats);
    }
  }

  bool success = tcp_socket_table_.erase(msg.sk);
  if (!success) {
    throw std::runtime_error(fmt::format("handle_close_socket: removing socket from table failed sk={:x}", msg.sk));
  }

  writer_.close_sock_info_tstamp(metadata.timestamp, msg.sk);
  nat_handler_.handle_close_socket(metadata.timestamp, &msg);

  // Also clean up any tcp data protocol handlers this socket may have
  // associated with it
  tcp_data_handler_->handle_close_socket(msg.sk);
}

void BufferedPoller::handle_rtt_estimator(message_metadata const &metadata, jb_agent_internal__rtt_estimator &msg)
{
  auto pos = tcp_socket_table_.find(msg.sk);
  if (pos.index == tcp_socket_table_.invalid) {
    if (!tcp_socket_table_ever_full_ && all_probes_loaded_) {
      LOG::debug_in(AgentLogKind::TCP, "handle_rtt_estimator: rtt_estimator on unknown socket sk={:x}", msg.sk);
    }
    return;
  }
  auto entry = pos.entry;

  /* we have a valid position. will go on to compute statistics */
  u64 diff_bytes_acked = msg.bytes_acked - entry->bytes_acked;
  u32 diff_delivered = msg.packets_delivered - entry->packets_delivered;
  u32 diff_retrans = msg.packets_retrans - entry->packets_retrans;

  u64 diff_bytes_received = msg.bytes_received - entry->bytes_received;
  u32 diff_rcv_holes = msg.rcv_holes - entry->rcv_holes;
  u32 diff_rcv_delivered = msg.rcv_delivered - entry->rcv_delivered;

  /* differences should be positive, if interpreted as signed numbers.
     avoid accumulating these huge diffs in stats below. */
  if (((s64)diff_bytes_acked) < ((s64)0)) {
    diff_bytes_acked = 0;
  }
  if (((s32)diff_delivered) < ((s32)0)) {
    // on kernels < 4.6, packets_delivered is an estimate and can under-count
    // and even become negative. make sure we're reporting non-negative numbers
    diff_delivered = 0;
  }
  if (((s32)diff_retrans) < ((s32)0)) {
    diff_retrans = 0;
  }
  if (((s64)diff_bytes_received) < ((s64)0)) {
    diff_bytes_received = 0;
  }
  if (((s32)diff_rcv_holes) < ((s32)0)) {
    diff_rcv_holes = 0;
  }
  if (((s32)diff_rcv_delivered) < ((s32)0)) {
    diff_rcv_delivered = 0;
  }

  /* update the entry for next time */
  entry->bytes_acked = msg.bytes_acked;
  entry->packets_delivered = msg.packets_delivered;
  entry->packets_retrans = msg.packets_retrans;

  entry->bytes_received = msg.bytes_received;
  entry->rcv_holes = msg.rcv_holes;
  entry->rcv_delivered = msg.rcv_delivered;

  /* find the statistics, and ask it to enqueue */
  auto &stats = tcp_socket_stats_.lookup(pos.index, metadata.timestamp, true).second;

  /* if stats were invalid, reset the values */
  if (!stats.valid) {
    stats.diff_bytes_acked = diff_bytes_acked;
    stats.diff_delivered = diff_delivered;
    stats.diff_retrans = diff_retrans;
    stats.max_srtt = msg.srtt;

    stats.diff_bytes_received = diff_bytes_received;
    stats.diff_rcv_holes = diff_rcv_holes;
    stats.diff_rcv_delivered = diff_rcv_delivered;
    stats.max_rcv_rtt = msg.rcv_rtt;
    stats.valid = true;
  } else {
    stats.diff_bytes_acked += diff_bytes_acked;
    stats.diff_delivered += diff_delivered;
    stats.diff_retrans += diff_retrans;
    stats.max_srtt = std::max(stats.max_srtt, msg.srtt);

    stats.diff_bytes_received += diff_bytes_received;
    stats.diff_rcv_holes += diff_rcv_holes;
    stats.diff_rcv_delivered += diff_rcv_delivered;
    stats.max_rcv_rtt = std::max(stats.max_rcv_rtt, msg.rcv_rtt);
  }
}

void BufferedPoller::handle_reset_tcp_counters(message_metadata const &metadata, jb_agent_internal__reset_tcp_counters &msg)
{

  LOG::debug_in(AgentLogKind::TCP, "handle_reset_tcp_counters: sk={:x} pid={}", msg.sk, msg.pid);

  // first, write telemetry like handle_new_socket
  writer_.new_sock_info_tstamp(metadata.timestamp, msg.pid, msg.sk);

  if (tcp_socket_table_.full()) {
    log_.warn("handle_reset_tcp_counters: tcp socket table is full! dropping socket");
    tcp_socket_table_ever_full_ = true;
    return;
  }

  auto pos = tcp_socket_table_.insert(msg.sk);
  if (pos.index != tcp_socket_table_.invalid) {
    tcp_index_to_sk_[pos.index] = msg.sk;
  } else {
    log_.error("handle_reset_tcp_counters: duplicate tcp socket sk={:x} pid={}", msg.sk, msg.pid);
    return;
  }
  auto entry = pos.entry;

  entry->bytes_acked = msg.bytes_acked;
  entry->packets_delivered = msg.packets_delivered;
  entry->packets_retrans = msg.packets_retrans;
  entry->bytes_received = msg.bytes_received;
}

void BufferedPoller::handle_tcp_syn_timeout(message_metadata const &metadata, jb_agent_internal__tcp_syn_timeout &msg)
{
  writer_.syn_timeout_tstamp(metadata.timestamp, msg.sk);
}

void BufferedPoller::handle_tcp_reset(message_metadata const &metadata, jb_agent_internal__tcp_reset &msg)
{
  writer_.tcp_reset_tstamp(metadata.timestamp, msg.sk, msg.is_rx);
}

void BufferedPoller::handle_http_response(message_metadata const &metadata, jb_agent_internal__http_response &msg)
{
  LOG::debug_in(
      AgentLogKind::HTTP,
      "handle_http_response: timestamp={}, sk={:x}, pid={}, code={}, "
      "latency_ns={}, client_server={}",
      metadata.timestamp,
      msg.sk,
      msg.pid,
      msg.code,
      msg.latency_ns,
      client_server_type_to_string((enum CLIENT_SERVER_TYPE)msg.client_server));

  writer_.http_response_tstamp(metadata.timestamp, msg.sk, msg.pid, msg.code, msg.latency_ns, msg.client_server);
}

void BufferedPoller::send_socket_stats(u64 t, u64 sk, tcp_statistics &stats)
{
  if ((stats.diff_bytes_acked > 0) || (stats.diff_retrans > 0)) {
    writer_.socket_stats_tstamp(t, sk, stats.diff_bytes_acked, stats.diff_delivered, stats.diff_retrans, stats.max_srtt, 0);
  }

  if ((stats.diff_bytes_received > 0) || (stats.diff_rcv_holes > 0)) {
    writer_.socket_stats_tstamp(
        t, sk, stats.diff_bytes_received, stats.diff_rcv_delivered, stats.diff_rcv_holes, stats.max_rcv_rtt, 1);
  }

  stats.valid = false;
}

void BufferedPoller::send_stats_from_queue(u64 t)
{
  if (tcp_socket_stats_.relative_timeslot(t) == 0) {
    /* not ready */
    return;
  }

  auto &queue = tcp_socket_stats_.current_queue();
  while (!queue.empty()) {
    /* get the next index */
    u32 index = queue.peek();

    /* get the stats entry for that index */
    auto &stats = tcp_socket_stats_.lookup_relative(index, 0, false).second;

    if (stats.valid) {
      /* write the message */
      send_socket_stats(t, tcp_index_to_sk_[index], stats);

      /* send_socket_stats sets stats.valid = false */
    }
    queue.pop();
  }

  /* done. advance the current timeslot */
  tcp_socket_stats_.advance();
}

void BufferedPoller::handle_udp_new_socket(message_metadata const &metadata, jb_agent_internal__udp_new_socket &msg)
{
  LOG::debug_in(
      AgentLogKind::UDP,
      "handle_udp_new_socket: sk={:x} pid={} laddr={} lport={}",
      msg.sk,
      msg.pid,
      IPv6Address::from(msg.laddr),
      msg.lport);

  /* first, insert into table */
  if (udp_socket_table_.full()) {
    log_.warn("handle_udp_new_socket: udp socket table full! dropping socket");
    udp_socket_table_ever_full_ = true;
    return;
  }
  auto pos = udp_socket_table_.insert(msg.sk);
  if (pos.index == udp_socket_table_.invalid) {
    log_.error("handle_udp_new_socket: duplicate udp socket");
    return;
  }
  pos.entry->pid = msg.pid;
  pos.entry->sk = msg.sk;
  memcpy(&pos.entry->laddr, msg.laddr, sizeof(struct in6_addr));
  pos.entry->lport = msg.lport;

  udp_send_new_socket(metadata.timestamp, pos.entry, pos.index);

  //	char ipaddr6_buf[INET6_ADDRSTRLEN];
  //	const char *addr_s = inet_ntop(AF_INET6, &msg.laddr, ipaddr6_buf,
  // INET6_ADDRSTRLEN);
  //
  //	std::cout << (reported ? "MSG: ": "")
  //			<< "UDP existing socket pid " << msg.pid
  //			<< " sk " << msg.sk
  //			<< " lport " << msg.lport
  //			<< " laddr " << addr_s << std::endl;
}

void BufferedPoller::handle_udp_destroy_socket(message_metadata const &metadata, jb_agent_internal__udp_destroy_socket &msg)
{
  LOG::debug_in(AgentLogKind::UDP, "handle_udp_destroy_socket: sk={:x}", msg.sk);

  auto pos = udp_socket_table_.find(msg.sk);
  if (pos.index == udp_socket_table_.invalid) {
    if (!udp_socket_table_ever_full_ && all_probes_loaded_) {
      LOG::debug_in(AgentLogKind::UDP, "handle_udp_destroy_socket: socket not found sk={:x}", msg.sk);
    }
    return;
  }

  // Ensure dns queries on this socket are timed out
  std::list<DnsRequests::Request> reqs;
  dns_requests_.lookup_socket(msg.sk, reqs);
  for (auto &req : reqs) {
    timeout_dns_request(metadata.timestamp, req);
  }

  /* send out statistics message if available */
  if (pos.entry->reported) {
    for (int is_rx = 0; is_rx < 2; is_rx++) {
      for (u32 epoch = 0; epoch < n_epochs; epoch++) {
        auto &stats = udp_socket_stats_[is_rx].lookup_relative(pos.index, epoch, false).second;
        if (stats.valid == true)
          udp_send_stats(metadata.timestamp, pos.index, is_rx, *pos.entry, stats);
      }
    }
    /* notify of the destruction */
    writer_.udp_destroy_socket_tstamp(metadata.timestamp, pos.index);
  }

  bool success = udp_socket_table_.erase(msg.sk);
  if (!success) {
    log_.error("handle_udp_destroy_socket: removing socket from table failed sk={:x}", msg.sk);
  }
}

void BufferedPoller::handle_udp_stats(message_metadata const &metadata, jb_agent_internal__udp_stats &msg)
{
  auto pos = udp_socket_table_.find(msg.sk);
  if (pos.index == udp_socket_table_.invalid) {
    // bpf should prevent this unless the socket table was ever full
    if (!udp_socket_table_ever_full_ && all_probes_loaded_) {
      // kprobe is inserted only after steady state, so this should never happen
      log_.error("handle_udp_stats: stats for missing socket sk={:x}", msg.sk);
    }
    return;
  }
  auto &entry = *pos.entry;

  /* find the statistics, and ask it to enqueue */
  u8 is_rx = msg.is_rx;
  auto &stats = udp_socket_stats_[is_rx].lookup(pos.index, metadata.timestamp, true).second;

  /* if stats are valid and address changed, output the previous stat and update
   * address */
  if (msg.changed_af != 0) {
    /* there might be statistics for a different address */
    if (stats.valid) {
      /* send the stats. will clear the stats */
      udp_send_stats(metadata.timestamp, pos.index, is_rx, entry, stats);
    }

    // Lookup whether this is a NAT-ed connection if this is ipv4
    hostport_tuple *ft = nullptr;
    if (msg.changed_af == AF_INET) {
      // NOTE: the bpf code always has a changed_af event before
      // the first statistics are sent, since the remote_addr in the bpf
      // table is initialized to 0:0.
      u32 laddr = ((u32 *)msg.laddr)[3];
      u32 raddr = ((u32 *)msg.raddr)[3];
      ft = nat_handler_.get_nat_mapping(laddr, raddr, ntohs(msg.lport), ntohs(msg.rport), IPPROTO_UDP);
    }

    /* set the address */
    auto &addr = entry.addrs[is_rx];
    if (ft != nullptr) {
      u32 laddr[4] = {0, 0, 0xffff0000, ft->src_ip};
      memcpy(&entry.laddr, laddr, sizeof(struct in6_addr));
      entry.lport = ntohs(ft->src_port);

      u32 raddr[4] = {0, 0, 0xffff0000, ft->dst_ip};
      memcpy(&addr.addr, raddr, sizeof(struct in6_addr));
      addr.port = ntohs(ft->dst_port);
    } else {
      memcpy(&entry.laddr, msg.laddr, sizeof(struct in6_addr));
      entry.lport = ntohs(msg.lport);

      memcpy(&addr.addr, msg.raddr, sizeof(struct in6_addr));
      addr.port = ntohs(msg.rport);
    }
    addr.changed_af = msg.changed_af;

    // fast track stats for address changes. we can't delay pushing address
    // changes to the server, because the next messages (dns
    // responses/timeouts/etc for example) may require the address to be set
    udp_send_stats(metadata.timestamp, pos.index, is_rx, entry, stats);
  }

  if (!stats.valid) {
    /* if stats were invalid, reset the values */
    stats.packets = msg.packets;
    stats.bytes = msg.bytes;
    stats.drops = msg.drops;
    stats.valid = true;
  } else {
    /* stats were valid, aggregate */
    stats.packets += msg.packets;
    stats.bytes += msg.bytes;
    stats.drops += msg.drops;
  }

  char lipaddr6_buf[INET6_ADDRSTRLEN];
  const char *laddr_s = inet_ntop(AF_INET6, &msg.laddr, lipaddr6_buf, INET6_ADDRSTRLEN);
  char ripaddr6_buf[INET6_ADDRSTRLEN];
  const char *raddr_s = inet_ntop(AF_INET6, &msg.raddr, ripaddr6_buf, INET6_ADDRSTRLEN);

  LOG::trace_in(
      AgentLogKind::UDP,
      "UDP {} sk {:x} sk_id {} laddr {} lport {} raddr {} rport {} "
      "packets {} bytes {} changed_af {} drops {}",
      (msg.is_rx ? "RX" : "TX"),
      msg.sk,
      pos.index,
      laddr_s,
      ntohs(msg.lport),
      raddr_s,
      ntohs(msg.rport),
      msg.packets,
      msg.bytes,
      int(msg.changed_af),
      int(msg.drops));
}

void BufferedPoller::handle_pid_info(message_metadata const &metadata, jb_agent_internal__pid_info &msg)
{
  pid_count_++;

  LOG::debug_in(AgentLogKind::PID, "{}: msg={} pid_count_={}", __func__, msg, pid_count_);

  cgroup_handler_.handle_pid_info(msg.pid, msg.cgroup, msg.comm);
  process_handler_.on_new_process(std::chrono::nanoseconds{metadata.timestamp}, msg);

  // Read the process command-line from /proc/PID/cmdline.
  // By this time the process could have exited, so reading this entry can fail.
  // We are using the `try_read_proc_cmdline` function which ignores the
  // "no such file or directory" error and returns an empty string instead.
  std::string cmdline;
  if (auto r = try_read_proc_cmdline(msg.pid)) {
    cmdline = *r;
  } else {
    log_.error("handle_pid_info: error reading cmdline for pid={}: {}", msg.pid, r.error());
  }

  writer_.pid_info_create_tstamp(metadata.timestamp, msg.pid, msg.comm, msg.cgroup, msg.parent_pid, jb_blob(cmdline));
}

void BufferedPoller::handle_pid_close(message_metadata const &metadata, jb_agent_internal__pid_close &msg)
{
  pid_count_--;

  LOG::debug_in(AgentLogKind::PID, "{}: msg={} pid_count_={}", __func__, msg, pid_count_);

  process_handler_.on_process_end(std::chrono::nanoseconds{metadata.timestamp}, msg);
  writer_.pid_close_info_tstamp(metadata.timestamp, msg.pid, msg.comm);
}

void BufferedPoller::handle_pid_set_comm(message_metadata const &metadata, jb_agent_internal__pid_set_comm &msg)
{
  LOG::debug_in(AgentLogKind::PID, "{}: msg={}", __func__, msg);

  process_handler_.set_process_command(std::chrono::nanoseconds{metadata.timestamp}, msg);
  writer_.pid_set_comm_tstamp(metadata.timestamp, msg.pid, msg.comm);
}

void BufferedPoller::handle_pid_exit(message_metadata const &metadata, jb_agent_internal__pid_exit &msg)
{
  LOG::debug_in(AgentLogKind::PID, "{}: msg={}", __func__, msg);

  process_handler_.pid_exit(std::chrono::nanoseconds{metadata.timestamp}, msg);
}

void trace_print_udp_socket_entry(const std::string_view &location, udp_socket_entry *entry)
{
  LOG::trace_in(
      AgentLogKind::UDP,
      "{}: udp_socket_entry\n"
      "  laddr={}  lport={}\n"
      "  reported={}  pid={}  sk={:x}\n"
      "  addrs[TX]: addr={}  port={}  changed_af={}\n"
      "  addrs[RX]: addr={}  port={}  changed_af={}",
      location,
      IPv6Address::from(entry->laddr),
      entry->lport,
      entry->reported,
      entry->pid,
      entry->sk,
      IPv6Address::from(entry->addrs[0].addr),
      entry->addrs[0].port,
      entry->addrs[0].changed_af,
      IPv6Address::from(entry->addrs[1].addr),
      entry->addrs[1].port,
      entry->addrs[1].changed_af);
}

void BufferedPoller::udp_send_new_socket(u64 ts, udp_socket_entry *entry, u64 index)
{
  trace_print_udp_socket_entry("udp_send_new_socket", entry);

  if (entry->lport != 0) {
    writer_.udp_new_socket_tstamp(ts, entry->pid, index, (uint8_t *)(entry->laddr.data()), entry->lport);
    entry->reported = true;
  }
}

void BufferedPoller::udp_send_stats(u64 t, u32 sk_id, u8 is_rx, udp_socket_entry &entry, udp_statistics &stats)
{
  trace_print_udp_socket_entry("udp_send_stats", &entry);

  auto &addr = entry.addrs[is_rx];
  if (entry.reported == false) {
    LOG::trace("BufferedPoller::udp_send_stats - entry.reported == false");
    udp_send_new_socket(t, &entry, sk_id);
  }

  switch (addr.changed_af) {
  case AF_INET:
    writer_.udp_stats_addr_changed_v4_tstamp(t, sk_id, is_rx, stats.packets, stats.bytes, addr.addr[3], addr.port);
    // TODO: Can uncomment this once we decide to support laddr info
    // writer_.udp_stats_addr_changed_v4_tstamp(
    //     t, sk_id, is_rx, stats.packets, stats.bytes, addr.addr[3], addr.port,
    //     entry.laddr[3], entry.lport);
    break;
  case AF_INET6:
    writer_.udp_stats_addr_changed_v6_tstamp(t, sk_id, is_rx, stats.packets, stats.bytes, (u8 *)&addr.addr, addr.port);
    // TODO: Can uncomment this once we decide to support laddr info
    // writer_.udp_stats_addr_changed_v6_tstamp(t, sk_id, is_rx, stats.packets,
    //                                          stats.bytes, (u8 *)&addr.addr,
    //                                          addr.port, (u8*)&entry.laddr,
    //                                          entry.lport);
    break;
  default:
    writer_.udp_stats_addr_unchanged_tstamp(t, sk_id, is_rx, stats.packets, stats.bytes);
    break;
  }

  // Send drops for receive side only, if we have drops
  if (is_rx && stats.drops > 0) {
    writer_.udp_stats_drops_changed_tstamp(t, sk_id, stats.drops);
  }

  if (is_log_whitelisted(AgentLogKind::UDP)) {
    LOG::trace_in(
        AgentLogKind::UDP,
        "BufferedPoller::udp_send_stats - sk_id: {}, raddr: {}, rport: {}, "
        "packets: {}, bytes: {}, changed_af: {}, drops: {}",
        sk_id,
        IPv6Address::from(addr.addr),
        addr.port,
        stats.packets,
        stats.bytes,
        int(addr.changed_af),
        stats.drops);
  }

  /* don't need to report the address next time if it doesn't change */
  addr.changed_af = 0;

  /* the current statistic is invalid, don't try to send */
  stats.valid = false;
}

void BufferedPoller::udp_send_stats_from_queue(u64 t)
{
  for (int is_rx = 0; is_rx < 2; is_rx++) {
    auto &store = udp_socket_stats_[is_rx];

    if (store.relative_timeslot(t) == 0) {
      /* not ready */
      continue;
    }

    auto &queue = store.current_queue();
    while (!queue.empty()) {
      /* get the next index */
      u32 index = queue.peek();

      /* get the stats entry for that index */
      auto &stats = store.lookup_relative(index, 0, false).second;

      if (stats.valid) {
        /* write the message */
        udp_send_stats(t, index, is_rx, udp_socket_table_[index], stats);

        /* udp_send_stats sets stats.valid = false */
      }

      queue.pop();
    }

    /* done. advance the current timeslot */
    store.advance();
  }
}

u32 BufferedPoller::u64_hasher::operator()(u64 const &s) const noexcept
{
  return lookup3_hashword((u32 *)&s, sizeof(u64) / 4, 0x7AFBAF00);
}

void BufferedPoller::handle_kill_css(message_metadata const &metadata, jb_agent_internal__kill_css &msg)
{
  cgroup_handler_.kill_css(metadata.timestamp, &msg);

  writer_.cgroup_close_tstamp(metadata.timestamp, msg.cgroup);
}

void BufferedPoller::handle_css_populate_dir(message_metadata const &metadata, jb_agent_internal__css_populate_dir &msg)
{
  cgroup_handler_.css_populate_dir(metadata.timestamp, &msg);

  writer_.cgroup_create_tstamp(metadata.timestamp, msg.cgroup, msg.cgroup_parent, msg.name);
}

void BufferedPoller::handle_cgroup_clone_children_read(
    message_metadata const &metadata, jb_agent_internal__cgroup_clone_children_read &msg)
{
  cgroup_handler_.cgroup_clone_children_read(metadata.timestamp, &msg);

  writer_.cgroup_create_tstamp(metadata.timestamp, msg.cgroup, msg.cgroup_parent, msg.name);
}

void BufferedPoller::handle_cgroup_attach_task(message_metadata const &metadata, jb_agent_internal__cgroup_attach_task &msg)
{
  cgroup_handler_.cgroup_attach_task(metadata.timestamp, &msg);

  process_handler_.on_cgroup_move(std::chrono::nanoseconds{metadata.timestamp}, msg);

  writer_.pid_cgroup_move_tstamp(metadata.timestamp, msg.pid, msg.cgroup);
}

void BufferedPoller::handle_nf_nat_cleanup_conntrack(
    message_metadata const &metadata, jb_agent_internal__nf_nat_cleanup_conntrack &msg)
{
  nat_handler_.handle_nf_nat_cleanup_conntrack(metadata.timestamp, &msg);
}

void BufferedPoller::handle_nf_conntrack_alter_reply(
    message_metadata const &metadata, jb_agent_internal__nf_conntrack_alter_reply &msg)
{
  nat_handler_.handle_nf_conntrack_alter_reply(metadata.timestamp, &msg);
}

void BufferedPoller::handle_existing_conntrack_tuple(
    message_metadata const &metadata, jb_agent_internal__existing_conntrack_tuple &msg)
{
  nat_handler_.handle_existing_conntrack_tuple(metadata.timestamp, &msg);
}

void BufferedPoller::handle_bpf_log(message_metadata const &metadata, jb_agent_internal__bpf_log &msg)
{
  // eventually, pass this to server using individual error messages
  // and turn this into LOG::debug_in(AgentLogKind::BPF,...)
  auto const filelineid = msg.filelineid;
  auto const linenumber = g_bpf_debug_line_info[filelineid];
  std::string_view const filename = g_bpf_debug_file_info[filelineid];

  writer_.bpf_log(jb_blob{filename}, linenumber, msg.code, msg.arg0, msg.arg1, msg.arg2);
}

void BufferedPoller::handle_stack_trace(message_metadata const &metadata, jb_agent_internal__stack_trace &msg)
{
#if DEBUG_ENABLE_STACKTRACE
  std::string stacktrace = probe_handler_.get_stack_trace(bpf_module_, msg.kernel_stack_id, msg.user_stack_id, msg.tgid);
  LOG::debug_in(
      AgentLogKind::BPF,
      "stack_trace: timestamp={}, kernel_stack_id={}, "
      "user_stack_id={}, tgid={}, comm={}\n{}\n",
      metadata.timestamp,
      msg.kernel_stack_id,
      msg.user_stack_id,
      msg.tgid,
      msg.comm,
      stacktrace);
#endif
}

void BufferedPoller::handle_tcp_data(message_metadata const &metadata, jb_agent_internal__tcp_data &msg)
{
  LOG::debug_in(
      AgentLogKind::PROTOCOL,
      "tcp_data: idx={}, timestamp={}, sk={:x}, pid={}, length={}, offset={}, "
      "stream_type={}({}), client_server={}({})\n",
      metadata.cpu_index,
      metadata.timestamp,
      msg.sk,
      msg.pid,
      msg.length,
      msg.offset,
      msg.stream_type,
      stream_type_to_string((enum STREAM_TYPE)msg.stream_type),
      msg.client_server,
      client_server_type_to_string((enum CLIENT_SERVER_TYPE)msg.client_server));

  tcp_data_handler_->process(
      metadata.cpu_index,
      metadata.timestamp,
      msg.sk,
      msg.pid,
      msg.length,
      msg.offset,
      (STREAM_TYPE)msg.stream_type,
      (CLIENT_SERVER_TYPE)msg.client_server);
}

void BufferedPoller::set_all_probes_loaded()
{
  all_probes_loaded_ = true;
}

#ifndef NDEBUG
void BufferedPoller::debug_bpf_lost_samples()
{
  debug_bpf_lost_samples_ = true;
}
#endif
