// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/agent_log.h>
#include <collector/kernel/fd_reader.h>
#include <collector/kernel/probe_handler.h>
#include <collector/kernel/proc_net_reader.h>
#include <collector/kernel/proc_reader.h>
#include <collector/kernel/socket_prober.h>
#include <config.h>
#include <iostream>
#include <set>
#include <util/log.h>

static constexpr u32 periodic_cb_mask = 0x3f;

SocketProber::SocketProber(
    ProbeHandler &probe_handler,
    struct render_bpf_bpf *skel,
    std::function<void(void)> periodic_cb,
    std::function<void(std::string)> check_cb,
    logging::Logger &log)
    : log_(log)
{
  // END
  // NOTE: Covers all protocols
  probe_handler.start_probe(skel, "on_security_sk_free", "security_sk_free");

  // inet END
  probe_handler.start_probe(skel, "on_inet_release", "inet_release");
  probe_handler.start_kretprobe(skel, "onret_inet_release", "inet_release");

  // CHANGE OF STATE
  probe_handler.start_probe(skel, "on_tcp_connect", "tcp_connect");
  probe_handler.start_probe(skel, "on_inet_csk_listen_start", "inet_csk_listen_start");

  // START
  probe_handler.start_probe(skel, "on_tcp_init_sock", "tcp_init_sock");
  // NOTE: these probes also sends out state information
  probe_handler.start_kretprobe(skel, "onret_inet_csk_accept", "inet_csk_accept");
  probe_handler.start_probe(skel, "on_inet_csk_accept", "inet_csk_accept");
  // UDP START
  probe_handler.start_kretprobe(skel, "onret_udp_v4_get_port", "udp_v4_get_port");
  probe_handler.start_kretprobe(skel, "onret_udp_v6_get_port", "udp_v6_get_port");
  probe_handler.start_probe(skel, "on_udp_v4_get_port", "udp_v4_get_port");
  probe_handler.start_probe(skel, "on_udp_v6_get_port", "udp_v6_get_port");

  // EXISTING
  probe_handler.start_probe(skel, "on_tcp4_seq_show", "tcp4_seq_show");
  probe_handler.start_probe(skel, "on_tcp6_seq_show", "tcp6_seq_show");
  probe_handler.start_probe(skel, "on_udp4_seq_show", "udp4_seq_show");
  probe_handler.start_probe(skel, "on_udp6_seq_show", "udp6_seq_show");

  periodic_cb();
  check_cb("socket prober startup");

  /* First step: fill up the "seen_inodes" bpf hashmap: inode -> pid */
  struct bpf_map *seen_inodes_map = probe_handler.get_bpf_map(skel, "seen_inodes");
  int map_fd = bpf_map__fd(seen_inodes_map);

  // Clear the map
  u32 key, next_key;
  while (bpf_map_get_next_key(map_fd, &key, &next_key) == 0) {
    bpf_map_delete_elem(map_fd, &next_key);
    key = next_key;
  }
  periodic_cb();
  check_cb("clear inode table");

  fill_inode_to_pid_map(map_fd, periodic_cb);
  check_cb("fill_inode_to_pid_map()");

  // now iterate over processes again but look through network namespaces
  // for new ns, read tcp and tcp6. this will trigger tcp46_seq_show
  trigger_seq_show(periodic_cb);
  check_cb("trigger_seq_show()");

  /* can remove existing now */
  probe_handler.cleanup_probe("tcp4_seq_show");
  periodic_cb();
  check_cb("socket prober cleanup (1)");
  probe_handler.cleanup_probe("tcp6_seq_show");
  periodic_cb();
  check_cb("socket prober cleanup (2)");
  probe_handler.cleanup_probe("udp4_seq_show");
  periodic_cb();
  check_cb("socket prober cleanup (3)");
  probe_handler.cleanup_probe("udp6_seq_show");
  periodic_cb();
  check_cb("socket prober cleanup (4)");
}

void SocketProber::fill_inode_to_pid_map(int map_fd, std::function<void(void)> periodic_cb)
{
  // iterate over /proc/
  ProcReader proc_reader;
  u32 proc_count = 0;
  u32 n_update_failures = 0;
  while (proc_reader.next()) {
    // every few procs, call periodic_cb, in case a lot of them are skipped
    if (((++proc_count) & periodic_cb_mask) == 0)
      periodic_cb();

    if (!proc_reader.is_pid())
      continue; // skip this entry if this wasn't a pid directory

    int pid = proc_reader.get_pid();
    FDReader fd_reader(pid);
    int status = fd_reader.open_task_dir();
    if (status) {
      LOG::trace_in(AgentLogKind::SOCKET, "skipping entry because task_dir couldn't be opened pid={}", pid);
      continue; // skip this entry because task_dir couldn't be opened
    }

    // for each fd of this pid.
    status = fd_reader.open_fd_dir();
    if (status) {
      LOG::trace_in(AgentLogKind::SOCKET, "skipping entry because fd_dir couldn't be opened pid={}", pid);
      continue; // skip this pid if fd_dir couldn't be opened
    }

    u32 inode_count = 0;
    while (!fd_reader.next_fd()) {
      int ino = fd_reader.get_inode();
      if (ino > 0) {
        u32 key = (u32)ino;
        u32 lookup_pid = 0;
        int lookup_result = bpf_map_lookup_elem(map_fd, &key, &lookup_pid);
        if (lookup_result == 0) {
          LOG::trace_in(
              AgentLogKind::SOCKET, "Duplicate file descriptor for pid={}, ino={} (lookup_pid={})", pid, ino, lookup_pid);
          continue;
        }
        int update_result = bpf_map_update_elem(map_fd, &key, &pid, 0);
        if (update_result != 0) {
          // log at most 10 times
          if (++n_update_failures < 10) {
            LOG::debug("Error updating hash_map: {} - {}", update_result, strerror(errno));
          }
          continue;
        }
        LOG::trace_in(AgentLogKind::SOCKET, "Added inode to hash_map: pid={}, ino={}", pid, ino);
      }

      // every few inodes, call periodic_cb
      if (((++inode_count) & periodic_cb_mask) == 0)
        periodic_cb();
    }

    // call periodic_cb for every pid
    periodic_cb();
  }

  if (n_update_failures != 0) {
    log_.warn("Recovering existing socket inodes got {} total update failures", n_update_failures);
  }
}

void SocketProber::trigger_seq_show(std::function<void(void)> periodic_cb)
{
  ProcReader proc_reader;
  std::set<int> done_network_namespaces;
  // iterate over /proc/
  while (proc_reader.next()) {
    periodic_cb();

    if (!proc_reader.is_pid()) {
      continue; // skip this entry if this wasn't a pid directory
    }

    int pid = proc_reader.get_pid();

    int network_namespace = get_network_namespace(pid);
    if (network_namespace == -1)
      continue; // something went wrong on this pid so skip to the next one

    /* if we've seen this namespace, don't re-process */
    auto ns_it = done_network_namespaces.find(network_namespace);
    if (ns_it != done_network_namespaces.end())
      continue;

    /* new network namespace -- process it */
    done_network_namespaces.insert(network_namespace);

    read_proc_net_tcp("/proc/" + std::to_string(pid) + "/net/tcp", periodic_cb);
    read_proc_net_tcp("/proc/" + std::to_string(pid) + "/net/tcp6", periodic_cb);
    read_proc_net_udp("/proc/" + std::to_string(pid) + "/net/udp", periodic_cb);
    read_proc_net_udp("/proc/" + std::to_string(pid) + "/net/udp6", periodic_cb);
  }

  periodic_cb();
}

void SocketProber::read_proc_net_tcp(const std::string &filename, std::function<void(void)> periodic_cb)
{

  ProcNetReader proc_net_reader(filename);
  u32 sk_count = 0;
  while (proc_net_reader.next()) {
    // every few sk's, call periodic_cb
    if (((++sk_count) & periodic_cb_mask) == 0)
      periodic_cb();

    // u64 sk_p = proc_net_reader.get_sk();

    // int sk_state = proc_net_reader.get_sk_state();
    // if ((sk_state != 1) && (sk_state != 10)) {
    //   LOG::trace_in(AgentLogKind::SOCKET, "sk {:x} state not listen/established: {}", sk_p, sk_state);
    //   continue;
    // }

    // int sk_ino = proc_net_reader.get_ino();
    // if (sk_ino == 0) {
    //  LOG::debug("sk {:x} sk_ino was 0: {}", sk_p, sk_ino);
    //  continue; // skip if ino=0 (sk will already be closed)
    //}

    /* TODO: log? */
    // std::cout << "successful sk: " << sk_p << "\t sk_ino: " << sk_ino <<
    // std::endl;
  }
}

void SocketProber::read_proc_net_udp(const std::string &filename, std::function<void(void)> periodic_cb)
{

  ProcNetReader proc_net_reader(filename);

  u32 sk_count = 0;
  while (proc_net_reader.next()) {
    /* just iterate to get udp sockets in udp_seq_show */

    // every few sk's, call periodic_cb
    if (((++sk_count) & periodic_cb_mask) == 0)
      periodic_cb();
  }
}

int SocketProber::get_network_namespace(int pid)
{
  char link[64];
  int network_namespace;
  snprintf(link, sizeof(link), "/proc/%d/ns/net", pid);

  char link_content[32];
  int info_len = readlink(link, link_content, sizeof(link_content) - 1);
  if (info_len == -1)
    // TODO: add logging
    return -1;

  link_content[info_len] = '\0';

  if (strncmp(link_content, "net:[", strlen("net:[")))
    // TODO: add logging
    return -1;
  //		throw std::runtime_error("get_network_namespace: readlink should
  // start with net:[");
  sscanf(link_content, "net:[%u]", &network_namespace);

  return network_namespace;
}
