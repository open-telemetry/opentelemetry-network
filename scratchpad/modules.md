# Kernel Collector Module Survey

This document maps the current monolithic eBPF program into conceptual modules by entity type. For each module it lists:
- C++ counterparts that handle messages from eBPF
- Messages produced by eBPF and their handlers
- Hooks used (tagged as START, END, EXISTING, TELEMETRY) and which message(s) they generate
- eBPF maps and other state used

Paths referenced below use repository‑relative locations.

---

## Processes

- C++ counterparts
  - `collector/kernel/buffered_poller.cc`: `handle_pid_*`, dispatches to `ProcessHandler`
  - `collector/kernel/process_handler.{h,cc}`: maintains process table and emits ingest messages

- Messages (BufferedPoller handlers)
  - `pid_info` → `BufferedPoller::handle_pid_info()` → `ProcessHandler::on_new_process()`
  - `pid_close` → `BufferedPoller::handle_pid_close()` → `ProcessHandler::on_process_end()`
  - `pid_set_comm` → `BufferedPoller::handle_pid_set_comm()` → `ProcessHandler::set_process_command()`
  - `pid_exit` → `BufferedPoller::handle_pid_exit()` → `ProcessHandler::pid_exit()`
  - `cgroup_attach_task` → `BufferedPoller::handle_cgroup_attach_task()` → `ProcessHandler::on_cgroup_move()`

- Hooks and classification (rendered by `collector/kernel/bpf_src/render_bpf.c`)
  - START
    - `kprobe/wake_up_new_task` → `perf_submit_agent_internal__pid_info` (new process)
  - END
    - `kretprobe/cgroup_exit` → `perf_submit_agent_internal__pid_close`
  - EXISTING
    - `kretprobe/get_pid_task` → `perf_submit_agent_internal__pid_info` (discovery)
  - TELEMETRY
    - `kprobe/__set_task_comm` → `perf_submit_agent_internal__pid_set_comm` (command updates)
    - Helper exists: `report_pid_exit(...)` → `perf_submit_agent_internal__pid_exit` (task exits)

- Maps and state
  - `tgid_info_table` (HASH): set of TGIDs already reported; dedupes START/EXISTING
  - `dead_group_tasks` (HASH): marks thread‑group leaders seen by `taskstats_exit`; used to gate END

---

## Containers (cgroups)

- C++ counterparts
  - `collector/kernel/buffered_poller.cc`: `handle_kill_css`, `handle_css_populate_dir`, `handle_existing_cgroup_probe`, `handle_cgroup_attach_task`
  - `collector/kernel/cgroup_handler.{h,cc}`: tracks cgroup hierarchy and fetches Docker/K8s metadata

- Messages (BufferedPoller handlers)
  - `kill_css` → `CgroupHandler::kill_css()`; also emits `writer_.cgroup_close_tstamp`
  - `css_populate_dir` → `CgroupHandler::css_populate_dir()`; also emits `writer_.cgroup_create_tstamp`
  - `existing_cgroup_probe` → `CgroupHandler::existing_cgroup_probe()`; also emits `writer_.cgroup_create_tstamp`
  - `cgroup_attach_task` → `CgroupHandler::cgroup_attach_task()`; also `ProcessHandler::on_cgroup_move()` and `writer_.pid_cgroup_move_tstamp`

- Hooks and classification (in `render_bpf.c`)
  - START
    - `kprobe/css_populate_dir` (>= 4.4) → `perf_submit_agent_internal__css_populate_dir`
    - `kprobe/cgroup_populate_dir` (< 4.4) → `perf_submit_agent_internal__css_populate_dir`
  - END
    - `kprobe/kill_css` (>= 3.12) → `perf_submit_agent_internal__kill_css`
    - `kprobe/cgroup_destroy_locked` (< 3.12) → `perf_submit_agent_internal__kill_css`
  - EXISTING
    - `kprobe/cgroup_control` + `kretprobe/cgroup_control` (>= 4.6) → `perf_submit_agent_internal__existing_cgroup_probe`
    - `kretprobe/cgroup_get_from_fd` → `perf_submit_agent_internal__existing_cgroup_probe`
    - `kprobe/cgroup_clone_children_read` (v1) → `perf_submit_agent_internal__existing_cgroup_probe`
  - TELEMETRY
    - `kprobe/cgroup_attach_task` → `perf_submit_agent_internal__cgroup_attach_task`

- Maps and state
  - No dedicated BPF maps; uses cgroup APIs and perf ring

---

## TCP Sockets (lifecycle, addressing, metrics)

- C++ counterparts
  - `collector/kernel/buffered_poller.{h,cc}`: TCP table/stats; handlers for tcp lifecycle/metrics messages
  - NAT correlation done in `NatHandler` for `set_state_*`

- Messages (BufferedPoller handlers)
  - Lifecycle
    - `new_sock_created` → `handle_new_socket`
    - `close_sock_info` → `handle_close_socket`
  - Addressing/state
    - `set_state_ipv4` → `handle_set_state_ipv4` (also forwards to `NatHandler`)
    - `set_state_ipv6` → `handle_set_state_ipv6` (also forwards to `NatHandler`)
  - Metrics/telemetry
    - `rtt_estimator` → `handle_rtt_estimator`
    - `reset_tcp_counters` → `handle_reset_tcp_counters` (initial metrics snapshot)
    - `tcp_syn_timeout` → `handle_tcp_syn_timeout`
    - `tcp_reset` → `handle_tcp_reset`
  - Protocol
    - `http_response` → `handle_http_response`
    - `tcp_data` → `handle_tcp_data` (delegates to `TCPDataHandler`)

- Hooks and classification (in `render_bpf.c`, plus TCP processor includes)
  - START
    - `kprobe/tcp_init_sock` → `perf_submit_agent_internal__new_sock_created`
    - `kretprobe/inet_csk_accept` → `perf_submit_agent_internal__new_sock_created` (+ `set_state_*`)
  - END
    - `kprobe/security_sk_free` → `perf_submit_agent_internal__close_sock_info`
    - `kretprobe/inet_release` → `perf_submit_agent_internal__close_sock_info`
  - EXISTING
    - `kprobe/tcp4_seq_show`, `kprobe/tcp6_seq_show` → ensure existing → `reset_tcp_counters` + `set_state_*`
  - TELEMETRY
    - Address: `kprobe/tcp_connect`, `kprobe/inet_csk_listen_start` → `set_state_*`
    - RTT/health: `kprobe/tcp_rtt_estimator`, `kprobe/tcp_rcv_established`, `kprobe/tcp_event_data_recv` → `rtt_estimator`
    - SYN timeouts: `kprobe/tcp_retransmit_timer`, `kprobe/tcp_syn_ack_timeout` → `tcp_syn_timeout`
    - Resets: `kprobe/tcp_reset`, and transmit path in `kprobe/ip_send_skb`, `kprobe/ip6_send_skb` (RST detection) → `tcp_reset`
    - Protocol data: TCP processor hooks `kprobe/tcp_sendmsg`/`kretprobe/tcp_sendmsg` and `kprobe/tcp_recvmsg`/`kretprobe/tcp_recvmsg` (via tail calls) → `tcp_data` and `http_response`

- Maps and state
  - `tcp_open_sockets` (HASH): tracks live sockets, last stats, parent listen socket
  - `seen_inodes` (HASH): dedupe/seeding for EXISTING path via `/proc` seq_show hooks
  - `tail_calls` (PROG_ARRAY): tail calls for multi‑stage handlers
  - TCP processor maps (in `collector/kernel/bpf_src/tcp-processor/`):
    - `_tcp_connections` (HASH): connection state
    - `_tcp_control` (HASH): backchannel enable/start per stream; read/written by `TCPDataHandler`
    - `data_channel` (PERF_EVENT_ARRAY): per‑CPU perf ring for TCP payload chunks

---

## UDP Sockets (lifecycle, stats)

- C++ counterparts
  - `collector/kernel/buffered_poller.{h,cc}`: handlers `handle_udp_new_socket`, `handle_udp_destroy_socket`, `handle_udp_stats`
  - DNS handling is integrated here (see DNS module)

- Messages (BufferedPoller handlers)
  - `udp_new_socket` → `handle_udp_new_socket`
  - `udp_destroy_socket` → `handle_udp_destroy_socket`
  - `udp_stats` → `handle_udp_stats` (tx/rx packets/bytes; addr changes; rx drops)

- Hooks and classification (in `render_bpf.c`)
  - START
    - `kretprobe/udp_v4_get_port`, `kretprobe/udp_v6_get_port` (success) → ensure `udp_open_socket` → `udp_new_socket`
  - END
    - `kprobe/security_sk_free`, `kretprobe/inet_release` → `udp_destroy_socket`
  - EXISTING
    - `kprobe/udp4_seq_show`, `kprobe/udp6_seq_show` → ensure existing → `udp_new_socket`
  - TELEMETRY
    - TX path: `kprobe/udp_send_skb`, `kprobe/udp_v6_send_skb`, `kprobe/ip_send_skb`, `kprobe/ip6_send_skb` (+ tail calls) → `udp_stats`
    - RX path: `kprobe/__skb_free_datagram_locked`, `kprobe/skb_free_datagram_locked` (tail call to `handle_receive_udp_skb`) → `udp_stats`

- Maps and state
  - `udp_open_sockets` (HASH): per‑socket stats and last reported endpoints (tx/rx)
  - `udp_get_port_hash` (HASH): per‑CPU scratch to link `udp_v*_get_port` kprobe/kretprobe
  - `seen_inodes` (HASH): shared dedupe for EXISTING via `/proc` seq_show
  - `tail_calls` (PROG_ARRAY): multi‑stage send/receive and DNS splitting

---

## NAT

- C++ counterparts
  - `collector/kernel/buffered_poller.{h,cc}`: delegates to `NatHandler` for NAT messages and to correlate with socket state
  - `collector/kernel/nat_handler.{h,cc}`: manages NAT mappings and emits `nat_remapping` ingest messages

- Messages (BufferedPoller → NatHandler)
  - `nf_conntrack_alter_reply` (START) → `NatHandler::handle_nf_conntrack_alter_reply()` (records NAT mapping; may emit remapping if socket known)
  - `nf_nat_cleanup_conntrack` (END) → `NatHandler::handle_nf_nat_cleanup_conntrack()` (removes mapping)
  - `existing_conntrack_tuple` (EXISTING) → `NatHandler::handle_existing_conntrack_tuple()` (learns both directions; records mapping if NAT‑ed)
  - Also consumes socket addressing:
    - `set_state_ipv4`/`set_state_ipv6` → record `sk ↔ 4‑tuple` for remapping correlation
    - `close_sock_info` → cleanup socket ↔ tuple index

- Hooks and classification (in `render_bpf.c`)
  - START: `kprobe/nf_conntrack_alter_reply` → `nf_conntrack_alter_reply`
  - END: `kprobe/nf_nat_cleanup_conntrack` → `nf_nat_cleanup_conntrack`
  - EXISTING: `kprobe/ctnetlink_dump_tuples` → `existing_conntrack_tuple`
  - TELEMETRY: (none standalone; NAT remap emission is driven by above plus socket `set_state_*`)

- Maps and state
  - BPF: `seen_conntracks` (HASH): marks conntracks seen to gate END reporting
  - Userland (`NatHandler`): `nat_table_`, `nat_table_rev_`, `existing_conntrack_table_`, `existing_sk_table_`, `existing_sk_table_rev_`

---

## DNS

- C++ counterparts
  - `collector/kernel/buffered_poller.{h,cc}`: `handle_dns_message()`; manages request table (`DnsRequests`) and emits `dns_request/response/timeout` ingest messages

- Messages
  - `dns_packet` (raw UDP payload and direction flag). BufferedPoller parses and correlates into higher‑level ingest records.

- Hooks and classification (in `render_bpf.c`)
  - TELEMETRY only
    - TX detection: `kprobe/udp_send_skb`, `kprobe/udp_v6_send_skb`, `kprobe/ip_send_skb`, `kprobe/ip6_send_skb` and tail‑call stages `on_*__2` → `perf_check_and_submit_dns`
    - RX detection: `kprobe/__skb_free_datagram_locked`, `kprobe/skb_free_datagram_locked` → tail call `handle_receive_udp_skb`/`__2` → `perf_check_and_submit_dns`

- Maps and state
  - `dns_message_array` (PERCPU_ARRAY): per‑CPU scratch buffer to copy DNS payloads safely
  - Reuses `udp_open_sockets` to resolve `sk` → socket entry for correlation
  - Userland: `DnsRequests` table for outstanding queries and timeouts

---

## TCP Data / HTTP Protocol

- C++ counterparts
  - `collector/kernel/buffered_poller.{h,cc}`: `handle_tcp_data()` delegates to `TCPDataHandler`
  - `collector/kernel/tcp_data_handler.{h,cc}`: reads `data_channel` rings, applies per‑socket protocol handlers, and emits ingest

- Messages
  - `tcp_data` (control message with sk/pid/len/offset/dir)
  - `http_response` (status/latency/client_server)

- Hooks and classification
  - TELEMETRY only
    - `kprobe/tcp_sendmsg`/`kretprobe/tcp_sendmsg` (tail‑called via `continue_tcp_sendmsg`)
    - `kprobe/tcp_recvmsg`/`kretprobe/tcp_recvmsg` (tail‑called via `continue_tcp_recvmsg`)
    - Lifecycle hooks for data map cleanup are shared with the main TCP module (`tcp_init_sock`, `security_sk_free`)

- Maps and state
  - `_tcp_connections`, `_tcp_control` (HASH): connection state and backchannel control
  - `data_channel` (PERF_EVENT_ARRAY): per‑CPU ring for payload chunks

---

## BPF Logging / Diagnostics

- C++ counterparts
  - `collector/kernel/buffered_poller.{h,cc}`: `handle_bpf_log`, `handle_stack_trace`
  - `collector/kernel/probe_handler.{h,cc}`: stack trace resolution utilities

- Messages
  - `bpf_log` (throttled; error codes/args)
  - `stack_trace` (optional; when `DEBUG_ENABLE_STACKTRACE`)

- Hooks and classification
  - TELEMETRY only; emitted from many code paths via helper macros

- Maps and state
  - Log throttle state (per‑CPU) and stack trace lookup (via probe handler); primary perf ring `events`

---

## Perf Rings

- Control/events ring: `events` (PERF_EVENT_ARRAY) in `render_bpf.c` carries all control messages listed above.
- Data ring(s): `data_channel` (PERF_EVENT_ARRAY) in TCP processor streams TCP payload chunks; `TCPDataHandler` reads the per‑CPU ring corresponding to the control ring’s CPU index for ordering.

---

## Quick Cross‑Reference of Message → Module

- Processes: `pid_info`, `pid_close`, `pid_set_comm`, `pid_exit`, `cgroup_attach_task`
- Containers: `kill_css`, `css_populate_dir`, `existing_cgroup_probe`, `cgroup_attach_task`
- TCP Sockets: `new_sock_created`, `set_state_ipv4`, `set_state_ipv6`, `close_sock_info`, `rtt_estimator`, `reset_tcp_counters`, `tcp_syn_timeout`, `tcp_reset`, `http_response`, `tcp_data`
- UDP Sockets: `udp_new_socket`, `udp_destroy_socket`, `udp_stats`, `dns_packet`
- NAT: `nf_conntrack_alter_reply`, `nf_nat_cleanup_conntrack`, `existing_conntrack_tuple` (plus consumes `set_state_*`, `close_sock_info`)
- BPF Logging: `bpf_log`, `stack_trace`

This survey should enable splitting the monolithic program into loadable modules that share perf rings (by configuring each module’s `BPF_MAP_TYPE_PERF_EVENT_ARRAY` with the same FDs at load time) and reuse common maps as appropriate.

