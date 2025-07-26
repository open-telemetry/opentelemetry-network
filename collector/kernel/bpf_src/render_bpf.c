/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "ebpf_net"
#endif


/* include net/sock, ignore the enum-conversion warning */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wenum-conversion"
#pragma clang diagnostic ignored "-Wtautological-compare"
#include <vmlinux.h>
#pragma clang diagnostic pop

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_tracing.h>

extern int LINUX_KERNEL_VERSION __kconfig;

// Networking macros
#define tcp_sk(ptr) container_of(ptr, struct tcp_sock, inet_conn.icsk_inet.sk)
#define inet_csk(ptr) container_of(ptr, struct inet_connection_sock, icsk_inet.sk)
#define inet_sk(ptr) container_of(ptr, struct inet_sock, sk)
#define sk_num			__sk_common.skc_num
#define sk_dport		__sk_common.skc_dport
#define sk_v6_daddr		__sk_common.skc_v6_daddr
#define sk_v6_rcv_saddr	__sk_common.skc_v6_rcv_saddr
#define sk_daddr		__sk_common.skc_daddr
#define sk_rcv_saddr		__sk_common.skc_rcv_saddr
#define sk_family		__sk_common.skc_family
#define sk_state		__sk_common.skc_state
#define AF_INET		2	/* Internet IP Protocol 	*/
#define AF_INET6	10	/* IP version 6			*/
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
#define inet_num		sk.__sk_common.skc_num
#define fl4_sport		uli.ports.sport
#define fl4_dport		uli.ports.dport
#define fl6_sport		uli.ports.sport
#define fl6_dport		uli.ports.dport

// Configuration
#include "config.h"
#include "render_bpf.h"
// Perf events - per-CPU perf ring buffer
struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(u32));
    __uint(value_size, sizeof(u32));
} events SEC(".maps");
#include "ebpf_net/agent_internal/bpf.h"

// Common utility functions
#include "tcp-processor/bpf_debug.h"
#include "tcp-processor/bpf_types.h"

static u64 abs_val(int val)
{
  return val < 0 ? -val : val;
}

// using constants for placeholders for readability after code dump
#pragma passthrough on
#define REPORT_DEBUG_EVENTS REPORT_DEBUG_EVENTS_PLACEHOLDER
#pragma passthrough off

/* HELPERS FOR FILLERS */
struct pkts_if_t {
  u32 packets_out;
  u32 sacked_out;
  u32 lost_out;
  u32 retrans_out;
};

static inline u32 packets_in_flight_helper(struct sock *sk)
{
  struct tcp_sock *tp = tcp_sk(sk);
  struct pkts_if_t t = {};

  bpf_probe_read(&t.packets_out, sizeof(u32), &tp->packets_out);
  bpf_probe_read(&t.sacked_out, sizeof(u32), &tp->sacked_out);
  bpf_probe_read(&t.lost_out, sizeof(u32), &tp->lost_out);
  bpf_probe_read(&t.retrans_out, sizeof(u32), &tp->retrans_out);
  return t.packets_out - (t.sacked_out + t.lost_out) + t.retrans_out;
}

/**
 * Tracking open sockets
 */
struct tcp_open_socket_t {
  u32 tgid;
  u32 rcv_holes;
  u64 last_output;
  u64 bytes_received; /* last observed */
  u32 rcv_delivered;
  u32 padding;
#if TCP_STATS_ON_PARENT
  struct sock *parent; /* parent listen socket if accepted, null otherwise */
#endif
};

struct udp_stats_t {
  u64 last_output;
  u32 laddr6[4];
  u32 raddr6[4];
  u16 lport;
  u16 rport;
  u8 addr_changed;
  u32 bytes;   /* bytes seen on socket since last submit */
  u32 packets; /* packets seen since last submit */
  u32 drops;   /* drops total for socket as of last submit (receive side only) */
};

/**
 * Tracking open sockets
 */
struct udp_open_socket_t {
  u32 tgid;
  struct udp_stats_t stats[2];
};

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, TGID);
    __type(value, TGID);
    __uint(max_entries, TABLE_SIZE__TGID_INFO);
} tgid_info_table SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct task_struct *);
    __type(value, struct task_struct *);
    __uint(max_entries, TABLE_SIZE__DEAD_GROUP_TASKS);
} dead_group_tasks SEC(".maps");

/* BPF_F_NO_PREALLOC was introduced in 6c9059817432, contained in
 * v4.6-rc1~91^2~108^2~6 */
static void setup_seen_inodes_table() {
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 6, 0)) {
    // Use regular BPF_HASH for older kernels
  } else {
    // Use BPF_F_NO_PREALLOC for newer kernels
  }
}

// Hash table with no preallocation
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u32);
    __type(value, u32);
    __uint(max_entries, TABLE_SIZE__SEEN_INODES);
    __uint(map_flags, BPF_F_NO_PREALLOC);
} seen_inodes SEC(".maps");
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct nf_conn *);
    __type(value, struct nf_conn *);
    __uint(max_entries, TABLE_SIZE__SEEN_CONNTRACKS);
} seen_conntracks SEC(".maps"); // Conntracks that we've reported to userspace
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct sock *);
    __type(value, struct tcp_open_socket_t);
    __uint(max_entries, TABLE_SIZE__TCP_OPEN_SOCKETS);
} tcp_open_sockets SEC(".maps"); /* information on live sks */

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct sock *);
    __type(value, struct udp_open_socket_t);
    __uint(max_entries, TABLE_SIZE__UDP_OPEN_SOCKETS);
} udp_open_sockets SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, u64);
    __type(value, struct sock *);
    __uint(max_entries, TABLE_SIZE__UDP_GET_PORT_HASH);
} udp_get_port_hash SEC(".maps");

BEGIN_DECLARE_SAVED_ARGS(cgroup_exit)
pid_t tgid;
END_DECLARE_SAVED_ARGS(cgroup_exit)

/*
 * cgroups subsystem used for cgroup probing
 * This is used by cgroup related probes to filter out cgroups that aren't in the memory hierarchy.
 * See SUBSYS macro in /linux_kernel/kernel/cgroup/cgroup.c.
 */
static int get_flow_cgroup_subsys() {
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(3, 14, 79)) {
    return mem_cgroup_subsys_id;
  } else {
    return memory_cgrp_id;
  }
}

#define FLOW_CGROUP_SUBSYS (get_flow_cgroup_subsys())

/* forward declarations */

static void
perf_check_and_submit_dns(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb, u8 proto, u16 sport, u16 dport, int is_rx);

////////////////////////////////////////////////////////////////////////////////////
/* PROCESSES */

static int report_pid_exit(TIMESTAMP timestamp, struct pt_regs *ctx, struct task_struct *task)
{
  perf_submit_agent_internal__pid_exit(ctx, timestamp, task->tgid, task->pid, task->exit_code);
  return 1;
}

static int set_task_group_dead(struct pt_regs *ctx, struct task_struct *tsk)
{
  int ret = bpf_map_update_elem(&dead_group_tasks, &tsk, &tsk, BPF_NOEXIST);
  if (ret != 0) {
    // Check if key already exists to distinguish duplicate vs table full
    void *existing = bpf_map_lookup_elem(&dead_group_tasks, &tsk);
    if (existing) {
#if DEBUG_OTHER_MAP_ERRORS
      bpf_trace_printk("set_task_group_dead: set_task_group_dead duplicate insert, dropping tsk tsk=%llx\n", tsk);
#endif
      bpf_log(ctx, BPF_LOG_TABLE_DUPLICATE_INSERT, BPF_TABLE_DEAD_GROUP_TASKS, 0, (u64)tsk);
      return 0;
    } else {
#if DEBUG_OTHER_MAP_ERRORS
      bpf_trace_printk("set_task_group_dead: set_task_group_dead table is full, dropping tsk tsk=%llx\n", tsk);
#endif
      bpf_log(ctx, BPF_LOG_TABLE_FULL, BPF_TABLE_DEAD_GROUP_TASKS, 0, (u64)tsk);
      return 0;
    }
  }

  return 1;
}

static int task_group_check_dead_and_remove(struct pt_regs *ctx, struct task_struct *tsk)
{
  int ret = bpf_map_delete_elem(&dead_group_tasks, &tsk);
  if (ret != 0) {
    // wasn't in map
    return 0;
  }

  return 1;
}

static int task_is_group_leader(struct pt_regs *ctx, struct task_struct *tsk)
{
  int ret;

  if (tsk == NULL) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return 0;
  }

  struct task_struct *group_leader = NULL;
  ret = bpf_probe_read(&group_leader, sizeof(group_leader), &tsk->group_leader);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }
  if (group_leader == NULL) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return 0;
  }

  if (tsk == group_leader) {
    return 1;
  }

  return 0;
}

static u64 get_task_cgroup(struct pt_regs *ctx, struct task_struct *tsk)
{
  int ret;

  if (tsk == NULL) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return 0;
  }

  struct css_set *set = NULL;
  ret = bpf_probe_read(&set, sizeof(set), &tsk->cgroups);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }
  if (set == NULL) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return 0;
  }

  struct cgroup_subsys_state *css = NULL;
  ret = bpf_probe_read(&css, sizeof(css), &set->subsys[FLOW_CGROUP_SUBSYS]);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }
  if (css == NULL) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return 0;
  }

  struct cgroup *cgrp = NULL;
  ret = bpf_probe_read(&cgrp, sizeof(cgrp), &css->cgroup);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }
  if (cgrp == NULL) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return 0;
  }

  return (u64)cgrp;
}

static pid_t get_task_parent(struct pt_regs *ctx, struct task_struct *tsk)
{
  int ret = 0;

  struct task_struct *parent_tsk = NULL;
  ret = bpf_probe_read(&parent_tsk, sizeof(parent_tsk), &tsk->parent);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return -1;
  }

  pid_t parent_tgid = 0;
  ret = bpf_probe_read(&parent_tgid, sizeof(parent_tgid), &parent_tsk->tgid);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return -1;
  }

  return parent_tgid;
}

// insert_tgid_info:
// adds a task to the 'tgid_info' table
// returns 0 if the task was not inserted because it was a duplicate or table full
// returns 1 if a task is new and was inserted
static int insert_tgid_info(struct pt_regs *ctx, TGID tgid)
{
  int ret = bpf_map_update_elem(&tgid_info_table, &tgid, &tgid, BPF_NOEXIST);
  if (ret != 0) {
    // Check if key already exists to distinguish duplicate vs table full
    void *existing = bpf_map_lookup_elem(&tgid_info_table, &tgid);
    if (existing) {
      // if we've already seen it, then don't make a duplicate msg
      // could arise if a new task shows up during the initial scan
      // or this is a thread inside an existing group, so we're seeing the tgid again
      return 0;
    } else {
#if DEBUG_OTHER_MAP_ERRORS
      bpf_trace_printk("insert_tgid_info: tgid_info table is full, dropping task tgid=%u\n", tgid);
#endif
      bpf_log(ctx, BPF_LOG_TABLE_FULL, BPF_TABLE_TGID_INFO, tgid, tgid);
      return 0;
    }
  }

  return 1;
}

// remove_tgid_info:
// removes a task to the 'tgid_info' table
// returns 0 if the tgid was not found in the table or if there was an error
// returns 1 if a task was removed successfully
static int remove_tgid_info(struct pt_regs *ctx, TGID tgid)
{
  int ret = bpf_map_delete_elem(&tgid_info_table, &tgid);
  if (ret != 0) {
#if DEBUG_OTHER_MAP_ERRORS
    bpf_trace_printk("remove_tgid_info: can't remove missing tgid=%u\n", tgid);
#endif
    return 0;
  }

  return 1;
}

// note is the tgid is dead or not
// used later by on_cgroup_exit handling
int on_taskstats_exit(struct pt_regs *ctx, struct task_struct *tsk, int group_dead)
{

  if (group_dead) {
    set_task_group_dead(ctx, tsk);
  }
  return 0;
}

// end
// this routine is called by 'do_exit' near the end of the destruction of a task
// but after all of the resources has been cleaned up, including file descriptor references
int on_cgroup_exit(struct pt_regs *ctx, struct task_struct *tsk)
{
  int ret;
  pid_t tgid = 0;
  ret = bpf_probe_read(&tgid, sizeof(tgid), &(tsk->tgid));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }

  // only consider tasks that are the last task in the thread group
  // since the process will not be terminating if there are more threads left
  if (!task_group_check_dead_and_remove(ctx, tsk)) {
    return 0;
  }

  GET_PID_TGID;

  BEGIN_SAVE_ARGS(cgroup_exit)
  SAVE_ARG(tgid)
  END_SAVE_ARGS(cgroup_exit)

  return 0;
}

int onret_cgroup_exit(struct pt_regs *ctx)
{
  GET_PID_TGID;

  GET_ARGS_MISSING_OK(cgroup_exit, args)
  if (args == NULL) {
    return 0;
  }

  pid_t tgid = args->tgid;

  DELETE_ARGS(cgroup_exit);

  // do some cleanup from our hashmap
  // if we didn't record this tgid before, a log message will have
  // fired, and won't send this along to the server
  if (!remove_tgid_info(ctx, tgid)) {
    return 0;
  }

  u64 now = get_timestamp();

  char comm[16] = {};
  bpf_get_current_comm(comm, sizeof(comm));

  perf_submit_agent_internal__pid_close(ctx, now, tgid, (u8 *)comm);

  return 0;
}

// set_task_comm: notice when command line is set for a process
int on_set_task_comm(struct pt_regs *ctx, struct task_struct *tsk, const char *buf)
{
  int ret;

  // only interested tasks considered the 'leader' of the group,
  // effectively userland processes, not threads
  if (!task_is_group_leader(ctx, tsk)) {
    return 0;
  }

  u64 now = get_timestamp();

  pid_t tgid = 0;
  ret = bpf_probe_read(&tgid, sizeof(tgid), &tsk->tgid);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }

  perf_submit_agent_internal__pid_set_comm(ctx, now, tgid, (uint8_t *)buf);
  return 0;
}

// start
int on_wake_up_new_task(struct pt_regs *ctx, struct task_struct *tsk)
{
  int ret;

  pid_t tgid = 0;
  ret = bpf_probe_read(&tgid, sizeof(tgid), &tsk->tgid);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }

  // mark the thread group id as seen, and if we've already seen it, return
  if (!insert_tgid_info(ctx, tgid)) {
    return 0;
  }

  u64 now = get_timestamp();
  u64 cgroup = get_task_cgroup(ctx, tsk);
  pid_t parent_tgid = get_task_parent(ctx, tsk);

  u8 comm[16] = {};
  ret = bpf_probe_read(comm, sizeof(comm), tsk->comm);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }

  perf_submit_agent_internal__pid_info(ctx, now, tgid, comm, cgroup, parent_tgid);

  return 0;
}

// existing
int onret_get_pid_task(struct pt_regs *ctx)
{
  int ret;
  struct task_struct *tsk = (struct task_struct *)PT_REGS_RC(ctx);

  pid_t tgid = 0;
  ret = bpf_probe_read(&tgid, sizeof(tgid), &tsk->tgid);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), (u64)tsk, 0);
    return 0;
  }

  // mark the task as seen, and if we've already seen it, return
  if (!insert_tgid_info(ctx, tgid)) {
    return 0;
  }

  u64 now = get_timestamp();
  u64 cgroup = get_task_cgroup(ctx, tsk);
  pid_t parent_tgid = get_task_parent(ctx, tsk);

  u8 comm[16] = {};
  ret = bpf_probe_read(comm, sizeof(comm), tsk->comm);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }

  perf_submit_agent_internal__pid_info(ctx, now, tgid, comm, cgroup, parent_tgid);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/* TCP SOCKETS */
static inline u32 tcp_get_delivered(struct sock *sk)
{
  struct tcp_sock *tp = tcp_sk(sk);
/* delivered accounting was introduced in ddf1af6fa00e77, i.e.,
 * v4.6-rc1~91^2~316^2~3 */
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 6, 0)) {
    u32 packets_out = 0;
    u32 sacked_out = 0;
    bpf_probe_read(&packets_out, sizeof(packets_out), &tp->packets_out);
    bpf_probe_read(&sacked_out, sizeof(sacked_out), &tp->sacked_out);
    return packets_out - sacked_out;
  } else {
    u32 delivered = 0;
    bpf_probe_read(&delivered, sizeof(delivered), &tp->delivered);
    return delivered;
  }
}

static inline void
report_rtt_estimator(struct pt_regs *ctx, struct sock *sk, struct tcp_open_socket_t *sk_info, u64 now, bool adjust)
{
  int ret;
  sk_info->last_output = now; // update the time in place

  u32 rcv_rtt_us = 0;
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 12, 0)) {
    bpf_probe_read(&rcv_rtt_us, sizeof(rcv_rtt_us), &tcp_sk(sk)->rcv_rtt_est.rtt);
  } else {
    bpf_probe_read(&rcv_rtt_us, sizeof(rcv_rtt_us), &tcp_sk(sk)->rcv_rtt_est.rtt_us);
  }

  // These values need to be taken from bpf_probe_read
  u32 srtt = 0;
  u32 snd_cwnd = 0;
  u64 bytes_acked = 0;
  u8 ca_state = 0;
  u32 packets_retrans = 0;
  u64 bytes_received = 0;

  ret = bpf_probe_read(&srtt, sizeof(srtt), &(tcp_sk(sk)->srtt_us));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  ret = bpf_probe_read(&snd_cwnd, sizeof(snd_cwnd), &(tcp_sk(sk)->snd_cwnd));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  ret = bpf_probe_read(&bytes_acked, sizeof(bytes_acked), &(tcp_sk(sk)->bytes_acked));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  ret = bpf_probe_read(&ca_state, sizeof(ca_state), &(*(&(inet_csk(sk)->icsk_sync_mss) + 1)));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  ret = bpf_probe_read(&packets_retrans, sizeof(packets_retrans), &(tcp_sk(sk)->total_retrans));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  ret = bpf_probe_read(&bytes_received, sizeof(bytes_received), &(tcp_sk(sk)->bytes_received));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  // adjustment for bytes reported when the connection is closed
  // zero out last two bits to account for this
  if (adjust) {
    bytes_acked &= ~3ull;
    bytes_received &= ~3ull;
  }

  perf_submit_agent_internal__rtt_estimator(
      ctx,
      now,
      srtt,
      snd_cwnd,
      bytes_acked,
      ca_state,
      (__u64)sk,
      packets_in_flight_helper(sk),
      tcp_get_delivered(sk),
      packets_retrans,
      sk_info->rcv_holes,
      bytes_received,
      sk_info->rcv_delivered,
      rcv_rtt_us);
}

// add tcp_open_socket
//
// should be paired with a `perf_submit_agent_internal__new_sock_created`
// in most circumstances, however we don't do this in the case of already
// existing sockets.
//
// returns 1 if it added the socket, 0 if it already existed,
// and -1 if the table is full, broken, or out of memory
// this operation is atomic and not susceptible to race conditions, because
// map.insert is atomic

static int add_tcp_open_socket(struct pt_regs *ctx, struct sock *sk, u32 tgid, u64 now, struct sock *parent)
{
  int ret;

#if TRACE_TCP_SOCKETS
  bpf_trace_printk("add_tcp_open_socket: %llx (tgid=%u)\n", sk, tgid);
#endif

  struct tcp_open_socket_t sk_info = {
    .tgid = tgid,
    .last_output = 0, // always do the first reporting
    .rcv_holes = 0,
    .rcv_delivered = 0,
#if TCP_STATS_ON_PARENT
    .parent = parent
#endif
  };

  ret = bpf_probe_read(&sk_info.bytes_received, sizeof(sk_info.bytes_received), &tcp_sk(sk)->bytes_received);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return -1;
  }

  ret = bpf_map_update_elem(&tcp_open_sockets, &sk, &sk_info, BPF_NOEXIST);

  if (ret != 0) {
    // Check if key already exists to distinguish duplicate vs table full
    void *existing = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
    if (existing) {
      return 0;
    } else {
#if DEBUG_TCP_SOCKET_ERRORS
      bpf_trace_printk("add_tcp_open_socket: tcp_open_sockets table is full, dropping socket sk=%llx (tgid=%u)\n", sk, tgid);
#endif
      bpf_log(ctx, BPF_LOG_TABLE_FULL, BPF_TABLE_TCP_OPEN_SOCKETS, tgid, (u64)sk);
      return -1;
    }
  }
  return 1;
}

static void remove_tcp_open_socket(struct pt_regs *ctx, struct sock *sk)
{
  struct tcp_open_socket_t *sk_info_p;
  sk_info_p = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (!sk_info_p) {
    return;
  }

#if TRACE_TCP_SOCKETS
  bpf_trace_printk("remove_tcp_open_socket: %llx\n", sk);
#endif

  // The lookup was successful.  Make a copy before deleting the entry, and only send related telemetry if the delete is
  // successful.
  struct tcp_open_socket_t sk_info = *sk_info_p;

  // do some cleanup from our hashmap
  int ret = bpf_map_delete_elem(&tcp_open_sockets, &sk);
  if (ret != 0) {
    // Another thread must have already deleted the entry for this sk.
    return;
  }

  // always report last rtt estimator before close
  // for short-lived connections, we won't see any data otherwise
  u64 now = get_timestamp();
  report_rtt_estimator(ctx, sk, &sk_info, now, true);

  perf_submit_agent_internal__close_sock_info(ctx, now, (__u64)sk);
}

static inline void submit_set_state_ipv6(struct pt_regs *ctx, u64 now, int tx_rx, struct sock *sk)
{
  struct sock *skp = NULL;
  bpf_probe_read(&skp, sizeof(skp), &sk);
  if (!skp) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return;
  }
  u16 dport = 0;
  u16 sport = 0;
  uint8_t daddr[16] = {};
  uint8_t saddr[16] = {};
  // These values need to be taken from bpf_probe_read
  bpf_probe_read(&dport, sizeof(dport), &(skp->sk_dport));
  bpf_probe_read(&sport, sizeof(sport), &(skp->sk_num));
  bpf_probe_read(daddr, sizeof(daddr), (uint8_t *)(sk->sk_v6_daddr.in6_u.u6_addr32));
  bpf_probe_read(saddr, sizeof(saddr), (uint8_t *)(sk->sk_v6_rcv_saddr.in6_u.u6_addr32));
  perf_submit_agent_internal__set_state_ipv6(ctx, now, daddr, saddr, bpf_ntohs(dport), sport, (__u64)sk, tx_rx);
}

// state - we want to get the 5-tuple as early as possible.
static inline void submit_set_state_ipv4(struct pt_regs *ctx, u64 now, int tx_rx, struct sock *sk)
{
  struct sock *skp = NULL;
  bpf_probe_read(&skp, sizeof(skp), &sk);
  if (!skp) {
    bpf_log(ctx, BPF_LOG_INVALID_POINTER, 0, 0, 0);
    return;
  }
  u16 dport = 0;
  u16 sport = 0;
  // These values need to be taken from bpf_probe_read
  bpf_probe_read(&dport, sizeof(dport), &(skp->sk_dport));
  bpf_probe_read(&sport, sizeof(sport), &(skp->sk_num));

  perf_submit_agent_internal__set_state_ipv4(ctx, now, sk->sk_daddr, sk->sk_rcv_saddr, bpf_ntohs(dport), sport, (__u64)sk, tx_rx);
}

static inline void submit_reset_tcp_counters(struct pt_regs *ctx, u64 now, u64 pid, struct sock *sk)
{
  int ret;
  u64 bytes_acked = 0;
  u32 packets_retrans = 0;
  u64 bytes_received = 0;

  ret = bpf_probe_read(&bytes_acked, sizeof(bytes_acked), &(tcp_sk(sk)->bytes_acked));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  ret = bpf_probe_read(&packets_retrans, sizeof(packets_retrans), &(tcp_sk(sk)->total_retrans));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  ret = bpf_probe_read(&bytes_received, sizeof(bytes_received), &(tcp_sk(sk)->bytes_received));
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  perf_submit_agent_internal__reset_tcp_counters(
      ctx, now, (__u64)sk, bytes_acked, tcp_get_delivered(sk), packets_retrans, bytes_received, pid);
}

// common logic for handling existing sockets
static int ensure_tcp_existing(struct pt_regs *ctx, struct sock *sk, u32 pid)
{
  if (sk->sk_family != AF_INET && sk->sk_family != AF_INET6) {
    return -1;
  }

  u64 now = get_timestamp();

  int tx_rx = 0;
  if (sk->sk_state == TCP_LISTEN)
    tx_rx = 2;

  int ret = add_tcp_open_socket(ctx, sk, pid, now, NULL);
  if (ret == 1) {
    submit_reset_tcp_counters(ctx, now, pid, sk);
    if (sk->sk_family == AF_INET6) {
      submit_set_state_ipv6(ctx, now, tx_rx, sk);
    } else {
      submit_set_state_ipv4(ctx, now, tx_rx, sk);
    }
  }
  return ret;
}

//
// HACK: Add tcp sockets that should exist to the table but don't
//

#if TCP_EXISTING_HACK
static struct tcp_open_socket_t *tcp_existing_hack(struct pt_regs *ctx, struct sock *sk)
{
  // Can only do this hack if we are in a userland process context
  PID_TGID _pid_tgid = bpf_get_current_pid_tgid();
  TGID _tgid = TGID_FROM_PID_TGID(_pid_tgid);
  PID _pid = PID_FROM_PID_TGID(_pid_tgid);
  if (_tgid == 0 || _pid == 0) {
    return NULL;
  }

  // ensure the tcp socket exists
  int ret = ensure_tcp_existing(ctx, sk, _tgid);
#if DEBUG_TCP_SOCKET_ERRORS
  if (ret == 1) {
    bpf_trace_printk(
        "tcp_update_stats: add_tcp_open_socket of missing socket: %llx (tgid=%u, cpu=%u)\n",
        sk,
        _tgid,
        bpf_get_smp_processor_id());
    bpf_log(ctx, BPF_LOG_EXISTING_HACK, BPF_TABLE_TCP_OPEN_SOCKETS, (u64)sk, 0);
  } else if (ret == 0) {
    // already existing, okay
  } else if (ret < 0) {
    bpf_trace_printk("tcp_update_stats: add_tcp_open_socket failed: %llx (tgid=%u)\n", sk, _tgid);
  }
#endif
  struct tcp_open_socket_t *sk_info_p = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  return sk_info_p;
}
#endif

// Ends a tcp socket lifetime and starts a new one
// Prep for Ticket #1166
static void restart_tcp_socket(struct pt_regs *ctx, TIMESTAMP now, struct sock *sk)
{
  GET_PID_TGID;

  // Connect to af_unspec starts a new span
  remove_tcp_open_socket(ctx, sk);

  // add an entry to our hash map
  int ret = add_tcp_open_socket(ctx, sk, _tgid, now, NULL);
  if (ret == 1) {
    perf_submit_agent_internal__new_sock_created(ctx, now, _tgid, (__u64)sk);
  } else if (ret == 0) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("tcp_init_sock: add_tcp_open_socket of existing socket: %llx (tgid=%u)\n", sk, _tgid);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_DUPLICATE_INSERT, BPF_TABLE_TCP_OPEN_SOCKETS, _tgid, (u64)sk);
  } else {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("tcp_init_sock: add_tcp_open_socket failed: %llx (tgid=%u)\n", sk, _tgid);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_BAD_INSERT, BPF_TABLE_TCP_OPEN_SOCKETS, _tgid, abs_val(ret));
  }
}

// connectors
int on_tcp_connect(struct pt_regs *ctx, struct sock *sk)
{
  struct tcp_open_socket_t *sk_info;
  sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (!sk_info) {
#if TCP_EXISTING_HACK

    sk_info = tcp_existing_hack(ctx, sk);
    if (sk_info == NULL) {
      return 0;
    }

#else

    bpf_log(ctx, BPF_LOG_TABLE_MISSING_KEY, BPF_TABLE_TCP_OPEN_SOCKETS, (u64)sk, 0);
    return 0;

#endif
  }

  u64 now = get_timestamp();
  int tx_rx = 1;
  u16 family = sk->sk_family;
  if (family == AF_INET) {
    submit_set_state_ipv4(ctx, now, tx_rx, sk);
  } else if (family == AF_INET6) {
    submit_set_state_ipv6(ctx, now, tx_rx, sk);
  } else {
    bpf_log(ctx, BPF_LOG_UNREACHABLE, 0, 0, 0);
  }
  return 0;
}

// listeners (acceptors)
// XXX: this function might fail so maybe we want to do some sort of kretprobe?
// I assume that if it fails, the user will handle things in a clever way
// and destroy the socket. But who knows?
int on_inet_csk_listen_start(struct pt_regs *ctx, struct sock *sk)
{
  // filter out non-tcp connections
  u16 family = sk->sk_family;
  if (family != AF_INET && family != AF_INET6)
    return 0;

  struct tcp_open_socket_t *sk_info;
  sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (!sk_info) {
#if TCP_EXISTING_HACK

    sk_info = tcp_existing_hack(ctx, sk);
    if (sk_info == NULL) {
      return 0;
    }

#else

    bpf_log(ctx, BPF_LOG_TABLE_MISSING_KEY, BPF_TABLE_TCP_OPEN_SOCKETS, (u64)sk, 0);
    return 0;

#endif
  }

  u64 now = get_timestamp();
  int tx_rx = 2; // this is the listener
  if (family == AF_INET) {
    submit_set_state_ipv4(ctx, now, tx_rx, sk);
  } else if (family == AF_INET6) {
    submit_set_state_ipv6(ctx, now, tx_rx, sk);
  } else {
    bpf_log(ctx, BPF_LOG_UNREACHABLE, 0, 0, 0);
  }
  return 0;
}

#if TCP_LIFETIME_HACK
//
// HACK: Remove open socket from table if it exists
//
static void tcp_lifetime_hack(struct pt_regs *ctx, struct sock *sk)
{
  struct tcp_open_socket_t *sk_info;
  sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (sk_info) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("tcp_lifetime_hack: tcp_lifetime_hack, sk=%llx, cpu=%u\n", sk, bpf_get_smp_processor_id());
    bpf_log(ctx, BPF_LOG_LIFETIME_HACK, BPF_TABLE_TCP_OPEN_SOCKETS, (u64)sk, 0);
#endif
#if REPORT_DEBUG_EVENTS
    u64 now = get_timestamp();
    perf_submit_agent_internal__report_debug_event(ctx, now, TCP_LIFETIME_HACK_CODE, (u64)sk, 0, 0, 0);
#endif // REPORT_DEBUG_EVENTS
    remove_tcp_open_socket(ctx, sk);
  }
}
#endif

// --- tcp_init_sock ----------------------------------------------------
// Where the start of TCP socket lifetimes is for IPv4 and IPv6
int on_tcp_init_sock(struct pt_regs *ctx, struct sock *sk)
{
  GET_PID_TGID;

  u64 now = get_timestamp();

#if TCP_LIFETIME_HACK
  // remove the entry from our hash map if it's there
  tcp_lifetime_hack(ctx, sk);
#endif

  // add an entry to our hash map
  int ret = add_tcp_open_socket(ctx, sk, _tgid, now, NULL);
  if (ret == 1) {
    perf_submit_agent_internal__new_sock_created(ctx, now, _tgid, (__u64)sk);
  } else if (ret == 0) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("tcp_init_sock: add_tcp_open_socket of existing socket: %llx (tgid=%u)\n", sk, _tgid);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_DUPLICATE_INSERT, BPF_TABLE_TCP_OPEN_SOCKETS, _tgid, (u64)sk);
  } else {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("tcp_init_sock: add_tcp_open_socket failed: %llx (tgid=%u)\n", sk, _tgid);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_BAD_INSERT, BPF_TABLE_TCP_OPEN_SOCKETS, _tgid, abs_val(ret));
  }

  return 0;
}

// --- inet_csk_accept --------------------------------------------------
// Called when a listen socket accepts gets a connection

BEGIN_DECLARE_SAVED_ARGS(on_inet_csk_accept)
struct sock *sk;
int flags;
u32 _pad_0; // required alignment for bcc
int *err;
END_DECLARE_SAVED_ARGS(on_inet_csk_accept)

int on_inet_csk_accept(struct pt_regs *ctx, struct sock *sk, int flags, int *err, bool kern)
{
  // Handle parameter differences between kernel versions
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 11, 0)) {
    // In older kernels, there's no bool kern parameter
    // The kern parameter doesn't exist, so we ignore it
  }
  GET_PID_TGID;

#if TCP_STATS_ON_PARENT
#if TCP_EXISTING_HACK // Only need to do this if we are using the parent stats reporting and need the socket to exist
  struct tcp_open_socket_t *sk_info;
  sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (!sk_info) {

    sk_info = tcp_existing_hack(ctx, sk);
    if (sk_info == NULL) {
      return 0;
    }
  }
#endif
#endif

  // Link us to our kretprobe
  BEGIN_SAVE_ARGS(on_inet_csk_accept)
  SAVE_ARG(sk)
  SAVE_ARG(flags)
  SAVE_ARG(err)
  END_SAVE_ARGS(on_inet_csk_accept)

  return 0;
}

int onret_inet_csk_accept(struct pt_regs *ctx)
{
  GET_PID_TGID;

  // Ensure the accept succeeded
  struct sock *newsk = (struct sock *)PT_REGS_RC(ctx);
  if (!newsk) {
#if TRACE_TCP_SOCKETS
    DEBUG_PRINTK("onret_inet_csk_accept: accept failed, tgid=%u\n", _tgid);
#endif
    // Unlink the kprobe/kretprobe
    DELETE_ARGS(on_inet_csk_accept);
    return 0;
  }

  // filter out non-tcp connections
  u16 family = 0;
  bpf_probe_read(&family, sizeof(family), &newsk->sk_family);
  if (family != AF_INET && family != AF_INET6) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("onret_inet_csk_accept: family is not ipv4 or ipv6 sk=%llx\n", newsk);
#endif
    bpf_log(ctx, BPF_LOG_UNEXPECTED_TYPE, (u64)newsk, (u64)family, 0);
    DELETE_ARGS(on_inet_csk_accept);
    return 0;
  }

  // Link us from our kprobe
  GET_ARGS(on_inet_csk_accept, args);
  if (args == NULL) {
    // Race condition where we might have been inside an accept when the probe
    // was inserted, ignore
    return 0;
  }

  // Set up the child socket
  u64 now = get_timestamp();

#if TCP_LIFETIME_HACK
  // remove the entry from our hash map if it's there
  tcp_lifetime_hack(ctx, newsk);
#endif

#if TCP_STATS_ON_PARENT
  int ret = add_tcp_open_socket(ctx, newsk, _tgid, now, args->sk);
#else
  int ret = add_tcp_open_socket(ctx, newsk, _tgid, now, NULL);
#endif
  if (ret == 1) {
    // Socket doesn't exist yet, create it in our table
    perf_submit_agent_internal__new_sock_created(ctx, now, _tgid, (__u64)newsk);
  } else if (ret == 0) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("onret_inet_csk_accept: add_tcp_open_socket of existing socket: %llx (tgid=%u)\n", newsk, _tgid);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_DUPLICATE_INSERT, BPF_TABLE_TCP_OPEN_SOCKETS, _tgid, (u64)newsk);
    DELETE_ARGS(on_inet_csk_accept);
    return 0;
  } else {
    // no log here because we already had another log inside add_tcp_open_socket for this
    DELETE_ARGS(on_inet_csk_accept);
    return 0;
  }

  // Set the state
  u16 dport = 0;
  u16 sport = 0;
  bpf_probe_read(&dport, sizeof(dport), &newsk->sk_dport);
  bpf_probe_read(&sport, sizeof(sport), &newsk->sk_num);

  if (family == AF_INET) {
    perf_submit_agent_internal__set_state_ipv4(
        ctx, now, newsk->sk_daddr, newsk->sk_rcv_saddr, bpf_ntohs(dport), sport, (__u64)newsk, 2);
  } else if (family == AF_INET6) {
    uint8_t daddr[16] = {};
    uint8_t saddr[16] = {};
    bpf_probe_read(daddr, sizeof(daddr), (uint8_t *)(newsk->sk_v6_daddr.in6_u.u6_addr32));
    bpf_probe_read(saddr, sizeof(saddr), (uint8_t *)(newsk->sk_v6_rcv_saddr.in6_u.u6_addr32));
    perf_submit_agent_internal__set_state_ipv6(ctx, now, daddr, saddr, bpf_ntohs(dport), sport, (__u64)newsk, 2);
  }

#if TRACE_TCP_SOCKETS
  DEBUG_PRINTK("on_inet_csk_accept exit: accepted socket %llx, tgid=%u\n", newsk, _tgid);
#endif

  DELETE_ARGS(on_inet_csk_accept);
  return 0;
}

// existing
int on_tcp46_seq_show(struct pt_regs *ctx, struct seq_file *seq, void *v)
{
  struct sock *sk = v;

  u32 ino = (u32)sk->sk_socket->file->f_inode->i_ino;
  u32 *lookup_tgid = bpf_map_lookup_elem(&seen_inodes, &ino);
  if (!lookup_tgid) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("on_tcp46_seq_show: ignoring socket %llx because of missing inode %u\n", sk, ino);
#endif
    return 0;
  }
  u32 tgid = *lookup_tgid;

#if DEBUG_TCP_SOCKET_ERRORS
  bpf_trace_printk("on_tcp46_seq_show: creating socket %llx with inode %u\n", sk, ino);
#endif

  /* we don't want to explore this inode again */
  bpf_map_delete_elem(&seen_inodes, &ino);

  int ret = ensure_tcp_existing(ctx, sk, tgid);
  if (ret == 0) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("on_tcp46_seq_show: add_tcp_open_socket of existing socket: %llx (tgid=%u)\n", sk, tgid);
    bpf_log(ctx, BPF_LOG_TABLE_DUPLICATE_INSERT, BPF_TABLE_TCP_OPEN_SOCKETS, tgid, (u64)sk);
#endif
  } else if (ret < 0) {
#if DEBUG_TCP_SOCKET_ERRORS
    bpf_trace_printk("on_tcp46_seq_show: add_tcp_open_socket failed: %llx (tgid=%u)\n", sk, tgid);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_BAD_INSERT, BPF_TABLE_TCP_OPEN_SOCKETS, tgid, abs_val(ret));
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/* UDP SOCKETS */

// add udp_open_socket
//
// should be paired with a `perf_submit_agent_internal__udp_new_socket`
// in most circumstances, however we don't do this in the case of already
// existing sockets.
//
// returns 1 if it added the socket, 0 if it already existed,
// and -1 if the table is full, broken, or out of memory
// this operation is atomic and not susceptible to race conditions, because
// map.insert is atomic

static inline int add_udp_open_socket(struct pt_regs *ctx, struct sock *sk, u32 tgid, u64 now)
{
#if TRACE_UDP_SOCKETS
  bpf_trace_printk("add_udp_open_socket: %llx (tgid=%u, cpu=%u)\n", sk, tgid, bpf_get_smp_processor_id());
#endif

  struct udp_open_socket_t sk_info = {
      .tgid = tgid,
  };
  int ret = bpf_map_update_elem(&udp_open_sockets, &sk, &sk_info, BPF_NOEXIST);
  if (ret != 0) {
    // Check if key already exists to distinguish duplicate vs table full
    void *existing = bpf_map_lookup_elem(&udp_open_sockets, &sk);
    if (existing) {
      return 0;
    } else {
#if DEBUG_UDP_SOCKET_ERRORS
      bpf_trace_printk("add_udp_open_socket: udp_open_sockets table is full, dropping socket sk=%llx (tgid=%u)\n", sk, tgid);
#endif
      bpf_log(ctx, BPF_LOG_TABLE_FULL, BPF_TABLE_UDP_OPEN_SOCKETS, (u64)tgid, (u64)sk);
      return -1;
    }
  }
  return 1;
}

static int ensure_udp_existing(struct pt_regs *ctx, struct sock *sk, u32 tgid)
{
  if (sk->sk_family != AF_INET && sk->sk_family != AF_INET6) {
    return -1;
  }

  u64 now = get_timestamp();

  int ret = add_udp_open_socket(ctx, sk, tgid, now);
  if (ret == 1) {
    struct in6_addr addr;
    u16 family = sk->sk_family;

    if (family == AF_INET6) {
      addr = sk->sk_v6_rcv_saddr;
    } else {
      addr.s6_addr32[0] = addr.s6_addr32[1] = 0;
      addr.s6_addr16[4] = 0;
      addr.s6_addr16[5] = 0xffff;
      addr.s6_addr32[3] = sk->sk_rcv_saddr;
    }

    u16 lport = 0;
    bpf_probe_read(&lport, sizeof(lport), &(inet_sk(sk)->inet_num));
    perf_submit_agent_internal__udp_new_socket(ctx, now, tgid, (__u64)sk, (uint8_t *)(&addr), lport);
  }
  return ret;
}

//
// HACK: Add tcp sockets that should exist to the table but don't
//
#if UDP_EXISTING_HACK
static struct udp_open_socket_t *udp_existing_hack(struct pt_regs *ctx, struct sock *sk)
{
  // Can only do this hack if we are in a userland process context
  PID_TGID _pid_tgid = bpf_get_current_pid_tgid();
  TGID _tgid = TGID_FROM_PID_TGID(_pid_tgid);
  PID _pid = PID_FROM_PID_TGID(_pid_tgid);
  if (_tgid == 0 || _pid == 0) {
    return NULL;
  }

  // Ensure the udp socket exists
  int ret = ensure_udp_existing(ctx, sk, _tgid);
#if DEBUG_UDP_SOCKET_ERRORS
  if (ret == 1) {
    bpf_trace_printk(
        "udp_existing_hack: add_udp_open_socket of missing socket: %llx (tgid=%u, cpu=%u)\n",
        sk,
        _tgid,
        bpf_get_smp_processor_id());
    bpf_log(ctx, BPF_LOG_EXISTING_HACK, BPF_TABLE_UDP_OPEN_SOCKETS, (u64)sk, 0);
  } else if (ret == 0) {
    // already existed, okay
  } else if (ret < 0) {
    bpf_trace_printk("udp_existing_hack: add_udp_open_socket failed: %llx (tgid=%u)\n", sk, _tgid);
  }
#endif
  struct udp_open_socket_t *sk_info_p = bpf_map_lookup_elem(&udp_open_sockets, &sk);
  return sk_info_p;
}
#endif

static void udp_send_stats_if_nonempty(struct pt_regs *ctx, u64 now, struct sock *sk, struct udp_stats_t *stats, u8 is_rx)
{
  /* is no data to send, return */
  if (!stats->addr_changed && stats->packets == 0 && sk->sk_drops.counter == stats->drops)
    return;

  // bpf_trace_printk("is_rx: %d   sk->sk_drops.counter: %d   stats->drops:
  // %d\n", is_rx, sk->sk_drops.counter, stats->drops);

  /* there is data, send a report */
  /* only send drops on receive side, since udp sends never drop packets */
  perf_submit_agent_internal__udp_stats(
      ctx,
      now,
      (__u64)sk,
      (uint8_t *)(stats->raddr6),
      stats->packets,
      stats->bytes,
      stats->addr_changed,
      stats->rport,
      is_rx,
      (uint8_t *)(stats->laddr6),
      stats->lport,
      is_rx ? (sk->sk_drops.counter - stats->drops) : 0);
}

static void remove_udp_open_socket(struct pt_regs *ctx, struct sock *sk)
{
  struct udp_open_socket_t *sk_info_p;
  sk_info_p = bpf_map_lookup_elem(&udp_open_sockets, &sk);
  if (!sk_info_p) {
    return;
  }

#if TRACE_UDP_SOCKETS
  bpf_trace_printk("remove_udp_open_socket: %llx (cpu=%u)\n", sk, bpf_get_smp_processor_id());
#endif

  // The lookup was successful.  Make a copy before deleting the entry, and only send related telemetry if the delete is
  // successful.
  struct udp_open_socket_t sk_info = *sk_info_p;

  int ret = bpf_map_delete_elem(&udp_open_sockets, &sk);
  if (ret != 0) {
    // Another thread must have already deleted the entry for this sk.
    return;
  }

  u64 now = get_timestamp();
  udp_send_stats_if_nonempty(ctx, now, sk, &sk_info.stats[0], 0);
  udp_send_stats_if_nonempty(ctx, now, sk, &sk_info.stats[1], 1);

  perf_submit_agent_internal__udp_destroy_socket(ctx, now, (__u64)sk);
}

#if UDP_LIFETIME_HACK
//
// HACK: Remove open socket from table if it exists
//
static void udp_lifetime_hack(struct pt_regs *ctx, struct sock *sk)
{
  struct udp_open_socket_t *sk_info;
  sk_info = bpf_map_lookup_elem(&udp_open_sockets, &sk);
  if (sk_info) {
#if DEBUG_UDP_SOCKET_ERRORS
    bpf_trace_printk("udp_lifetime_hack: udp_lifetime_hack sk=%llx cpu=%u\n", sk, bpf_get_smp_processor_id());
    bpf_log(ctx, BPF_LOG_LIFETIME_HACK, BPF_TABLE_UDP_OPEN_SOCKETS, (u64)sk, 0);
#endif
#if REPORT_DEBUG_EVENTS
    u64 now = get_timestamp();
    perf_submit_agent_internal__report_debug_event(ctx, now, UDP_LIFETIME_HACK_CODE, (u64)sk, 0, 0, 0);
#endif // REPORT_DEBUG_EVENTS
    remove_udp_open_socket(ctx, sk);
  }
}
#endif

// EXISTING
int on_udp46_seq_show(struct pt_regs *ctx, struct seq_file *seq, void *v)
{
  struct sock *sk = v;

  u32 ino = (u32)sk->sk_socket->file->f_inode->i_ino;
  u32 *lookup_tgid = bpf_map_lookup_elem(&seen_inodes, &ino);
  if (!lookup_tgid) {
#if DEBUG_UDP_SOCKET_ERRORS
    bpf_trace_printk("on_udp46_seq_show: ignoring socket %llx because of missing inode %u\n", sk, ino);
#endif
    return 0;
  }
  u32 tgid = *lookup_tgid;

  /* we don't want to explore this inode again */
  bpf_map_delete_elem(&seen_inodes, &ino);

  int ret = ensure_udp_existing(ctx, sk, tgid);
  if (ret == 0) {
#if DEBUG_UDP_SOCKET_ERRORS
    bpf_trace_printk("on_udp46_seq_show: add_udp_open_socket of existing socket: %llx (tgid=%u)\n", sk, tgid);
    bpf_log(ctx, BPF_LOG_TABLE_DUPLICATE_INSERT, BPF_TABLE_UDP_OPEN_SOCKETS, tgid, (u64)sk);
#endif
  } else if (ret < 0) {
#if DEBUG_UDP_SOCKET_ERRORS
    bpf_trace_printk("on_udp46_seq_show: add_udp_open_socket failed: %llx (tgid=%u)\n", sk, tgid);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_BAD_INSERT, BPF_TABLE_UDP_OPEN_SOCKETS, tgid, abs_val(ret));
  }

  return 0;
}

// NEW
int on_udp_v46_get_port(struct pt_regs *ctx, struct sock *sk)
{
  GET_PID_TGID;

  int ret = bpf_map_update_elem(&udp_get_port_hash, &_cpu, &sk, BPF_NOEXIST);
  if (ret != 0) {
    // Check if key already exists to distinguish duplicate vs table full
    void *existing = bpf_map_lookup_elem(&udp_get_port_hash, &_cpu);
    if (existing) {
#if DEBUG_OTHER_MAP_ERRORS
      bpf_trace_printk("on_udp_v46_get_port: should not see existing hash entry (tgid=%u, cpu=%u)\n", _tgid, _cpu);
#endif
      return 0;
    } else {
#if DEBUG_OTHER_MAP_ERRORS
      bpf_trace_printk("on_udp_v46_get_port: udp_get_port_hash table is full, dropping call (tgid=%u, cpu=%u)\n", _tgid, _cpu);
#endif
      return 0;
    }
  }
  return 0;
}

int onret_udp_v46_get_port(struct pt_regs *ctx)
{
  struct sock **found = NULL;
  struct sock *sk = NULL;
  GET_PID_TGID;
  int retval = (int)PT_REGS_RC(ctx);

  found = bpf_map_lookup_elem(&udp_get_port_hash, &_cpu);
  if (!found)
    return 0;
  sk = *found;

  /**
   * on success, udp_lib_get_port sets inet_sk(sk)->inet_num.
   *
   * get_port is called from:
   *   - inet_autobind; does not set inet->inet_rcv_saddr
   *   - inet_bind, which sets inet->inet_rcv_saddr and inet->inet_saddr
   *       just before calling get_port
   *
   */

  if (retval == 0) {

#if UDP_LIFETIME_HACK
    // remove the entry from our hash map if it's there
    udp_lifetime_hack(ctx, sk);
#endif

    int ret = ensure_udp_existing(ctx, sk, _tgid);
#if DEBUG_UDP_SOCKET_ERRORS
    if (ret == 0) {
      bpf_trace_printk(
          "common_ret_udp_v46_get_port: add_udp_open_socket of existing socket: %llx (tgid=%u, cpu=%u)\n", sk, _tgid, _cpu);
    } else if (ret < 0) {
      bpf_trace_printk("common_ret_udp_v46_get_port: add_udp_open_socket failed: %llx (tgid=%u, cpu=%u)\n", sk, _tgid, _cpu);
    }
#endif
  }

  bpf_map_delete_elem(&udp_get_port_hash, &_cpu);

  return 0;
}

/*
 * socket lifetime end
 * Note:  This function is called from on_security_sk_free() and on_inet_release().  Historically, only the security_sk_free()
 * kprobe was used, but there were some unexplained missing socket close events that seemed to be due to security_sk_free()
 * probes not being triggered from certain kernel thread states/contexts.  The inet_release() kprobe was added to make sure all
 * socket close events were captured.  inet_release() calls security_sk_free(), synchronously in some cases and asynchronously
 * via call_rcu() in other cases. Asynchronous calls can result in multiple threads executing this and subsequent functions
 * concurrently, so they need to deal with tcp/udp_open_sockets map lookup and delete failures appropriately to make sure only
 * one of the calls sends the final socket telemetry. (i.e. If a lookup or delete fails, assume that the other thread already
 * successfully deleted the entry, and only send the final telemetry if the delete is successful.)
 */
static inline void remove_open_socket(struct pt_regs *ctx, struct sock *sk)
{
  {
    // TCP(v4/v6) Socket?
    struct tcp_open_socket_t *tcp_sk_info;
    tcp_sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
    if (tcp_sk_info) {
      remove_tcp_open_socket(ctx, sk);
      return;
    }
  }
  {
    // UDP(v4/v6) Socket?
    struct udp_open_socket_t *udp_sk_info;
    udp_sk_info = bpf_map_lookup_elem(&udp_open_sockets, &sk);
    if (udp_sk_info) {
      remove_udp_open_socket(ctx, sk);
      return;
    }
  }
  // Must be some other kind of socket we don't yet care about
}

// --- security_sk_free ----------------------------------------------------
// This is where final socket destruction happens for all socket types
int on_security_sk_free(struct pt_regs *ctx, struct sock *sk)
{
  remove_open_socket(ctx, sk);
  return 0;
}

BEGIN_DECLARE_SAVED_ARGS(inet_release)
struct sock *sk;
END_DECLARE_SAVED_ARGS(inet_release)

int on_inet_release(struct pt_regs *ctx, struct socket *sock)
{
  GET_PID_TGID;

  struct sock *sk = NULL;
  bpf_probe_read(&sk, sizeof(sk), &sock->sk);

  if (!sk) {
    return 0;
  }

  BEGIN_SAVE_ARGS(inet_release)
  SAVE_ARG(sk)
  END_SAVE_ARGS(inet_release)

  return 0;
}

int onret_inet_release(struct pt_regs *ctx)
{
  GET_PID_TGID;

  GET_ARGS_MISSING_OK(inet_release, args);
  if (args == NULL) {
    return 0;
  }

  struct sock *sk = args->sk;

  DELETE_ARGS(inet_release);

  if (sk) {
    remove_open_socket(ctx, sk);
  }

  return 0;
}

// TCP reset
static void handle_tcp_reset(struct pt_regs *ctx, struct sock *sk, u8 is_rx)
{
  if (!bpf_map_lookup_elem(&tcp_open_sockets, &sk)) {
    // don't send an event on sockets we never notified userspace about
    // bpf_trace_printk("handle_tcp_reset: no socket\n");
    return;
  }

  // bpf_trace_printk("handle_tcp_reset: perf_submit_agent_internal__tcp_reset\n");
  u64 now = get_timestamp();
  perf_submit_agent_internal__tcp_reset(ctx, now, (__u64)sk, is_rx);
}

// receive TCP RST
int on_tcp_reset(struct pt_regs *ctx, struct sock *sk)
{
  // bpf_trace_printk("on_tcp_reset\n");
  // receive RST (is_rx = 1)
  handle_tcp_reset(ctx, sk, 1);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/* LIVE UDP */

// laddr, raddr is in big endian format, lport and rport are in little endian
// format
static void udp_update_stats(
    struct pt_regs *ctx,
    struct sock *sk,
    struct sk_buff *skb,
    struct in6_addr *laddr,
    u16 lport,
    struct in6_addr *raddr,
    u16 rport,
    u8 is_rx)
{
  const u16 family = sk->sk_family;
  struct udp_open_socket_t *sk_info_p;
  struct udp_stats_t *stats;

  sk_info_p = bpf_map_lookup_elem(&udp_open_sockets, &sk);
  if (sk_info_p == NULL) {
#if UDP_EXISTING_HACK

    sk_info_p = udp_existing_hack(ctx, sk);
    if (sk_info_p == NULL) {
      return;
    }

#else

    bpf_log(ctx, BPF_LOG_TABLE_MISSING_KEY, BPF_TABLE_UDP_OPEN_SOCKETS, (u64)sk, 0);
    return;

#endif
  }

  stats = &sk_info_p->stats[is_rx];

  /* compare sk_info_p's address and the one in fl4 */
  u32 lchanged = (stats->laddr6[0] ^ laddr->s6_addr32[0]) | (stats->laddr6[1] ^ laddr->s6_addr32[1]) |
                 (stats->laddr6[2] ^ laddr->s6_addr32[2]) | (stats->laddr6[3] ^ laddr->s6_addr32[3]) | (stats->lport ^ lport);
  u32 rchanged = (stats->raddr6[0] ^ raddr->s6_addr32[0]) | (stats->raddr6[1] ^ raddr->s6_addr32[1]) |
                 (stats->raddr6[2] ^ raddr->s6_addr32[2]) | (stats->raddr6[3] ^ raddr->s6_addr32[3]) | (stats->rport ^ rport);
  u32 dchanged = is_rx ? (sk->sk_drops.counter != stats->drops) : 0;
  u32 changed = lchanged | rchanged | dchanged;

  u64 now = get_timestamp();

  if (changed || ((now - stats->last_output) >= FILTER_NS)) {

    /* set the address */
    if (lchanged) {
      stats->laddr6[0] = laddr->s6_addr32[0];
      stats->laddr6[1] = laddr->s6_addr32[1];
      stats->laddr6[2] = laddr->s6_addr32[2];
      stats->laddr6[3] = laddr->s6_addr32[3];
      stats->lport = lport;
    }
    if (rchanged) {
      stats->raddr6[0] = raddr->s6_addr32[0];
      stats->raddr6[1] = raddr->s6_addr32[1];
      stats->raddr6[2] = raddr->s6_addr32[2];
      stats->raddr6[3] = raddr->s6_addr32[3];
      stats->rport = rport;
    }
    stats->addr_changed = (lchanged || rchanged) ? (u8)family : 0;

    /* send the update if anything has changed, or if we have statistics */
    udp_send_stats_if_nonempty(ctx, now, sk, stats, is_rx);

    /* reset statistics */
    stats->packets = 1;
    stats->drops = is_rx ? sk->sk_drops.counter : 0;
    stats->bytes = skb->len;

    /* schedule next update */
    stats->last_output = now;

    return;
  }

  /* address is the same and too early to send a notification, just update */
  stats->packets++;
  stats->bytes += skb->len;

  /** don't update 'drops' here, because it's not cumulative,
   * it's a total since last reset
   */

  return;
}

/* Tail calls used by kprobes below, so we can have enough stack space */
struct {
	__uint(type, BPF_MAP_TYPE_PROG_ARRAY);
	__uint(max_entries, NUM_TAIL_CALLS);
	__uint(key_size, sizeof(__u32));
	__uint(value_size, sizeof(__u32));
} tail_calls SEC(".maps");

int on_udp_send_skb__2(struct pt_regs *ctx, struct sk_buff *skb, struct flowi4 *fl4)
{
  perf_check_and_submit_dns(ctx, skb->sk, skb, IPPROTO_UDP, fl4->fl4_sport, fl4->fl4_dport, 0);
  return 0;
}

int on_udp_v6_send_skb__2(struct pt_regs *ctx, struct sk_buff *skb, struct flowi6 *fl6)
{
  perf_check_and_submit_dns(ctx, skb->sk, skb, IPPROTO_UDP, fl6->fl6_sport, fl6->fl6_dport, 0);
  return 0;
}

int on_ip_send_skb__2(struct pt_regs *ctx, struct net *net, struct sk_buff *skb)
{
  struct sock *sk = skb->sk;
  struct iphdr *ip_hdr = (struct iphdr *)(skb->head + skb->network_header);
  if (ip_hdr->protocol == IPPROTO_UDP) {
    struct udphdr *udp_hdr = (struct udphdr *)(skb->head + skb->transport_header);

    perf_check_and_submit_dns(ctx, sk, skb, IPPROTO_UDP, udp_hdr->source, udp_hdr->dest, 0);
  }

  return 0;
}

int on_ip6_send_skb__2(struct pt_regs *ctx, struct sk_buff *skb)
{
  struct sock *sk = skb->sk;
  struct ipv6hdr *ipv6_hdr = (struct ipv6hdr *)(skb->head + skb->network_header);
  if (ipv6_hdr->nexthdr == IPPROTO_UDP) {
    struct udphdr *udp_hdr = (struct udphdr *)(skb->head + skb->transport_header);

    perf_check_and_submit_dns(ctx, sk, skb, IPPROTO_UDP, udp_hdr->source, udp_hdr->dest, 0);
  }

  return 0;
}

/* udp kprobes, reference the tail calls above */

// may be optimized out on some kernels, if hook fails, we will use
// on_ip_send_skb
int on_udp_send_skb(struct pt_regs *ctx, struct sk_buff *skb, struct flowi4 *fl4)
{
  GET_PID_TGID;

  struct in6_addr laddr = make_ipv6_address(fl4->saddr);
  struct in6_addr raddr = make_ipv6_address(fl4->daddr);

  udp_update_stats(ctx, skb->sk, skb, &laddr, fl4->fl4_sport, &raddr, fl4->fl4_dport, 0);

  // Call on_udp_send_skb__2
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_ON_UDP_SEND_SKB__2);

  return 0;
}

// may be optimized out on some kernels, if hook fails, we will use
// on_ip6_send_skb
int on_udp_v6_send_skb(struct pt_regs *ctx, struct sk_buff *skb, struct flowi6 *fl6)
{
  GET_PID_TGID;

  struct in6_addr laddr = fl6->saddr;
  struct in6_addr raddr = fl6->daddr;

#if DEBUG_UDP_SOCKET_ERRORS
  __check_broken_in6_addr(&laddr, __LINE__);
  __check_broken_in6_addr(&raddr, __LINE__);
#endif

  udp_update_stats(ctx, skb->sk, skb, &laddr, fl6->fl6_sport, &raddr, fl6->fl6_dport, 0);

  // Call on_udp_v6_send_skb__2
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_ON_UDP_V6_SEND_SKB__2);

  return 0;
}

// send TCP RST
int on_tcp_send_active_reset(struct pt_regs *ctx, struct sock *sk, gfp_t priority)
{
  // bpf_trace_printk("on_tcp_send_active_reset\n");
  // send RST (is_rx = 0)
  handle_tcp_reset(ctx, sk, 0);
  return 0;
}

int on_ip_send_skb(struct pt_regs *ctx, struct net *net, struct sk_buff *skb)
{
  struct sock *sk = skb->sk;
  struct iphdr *ip_hdr = (struct iphdr *)(skb->head + skb->network_header);

  if (ip_hdr->protocol == IPPROTO_UDP) {
    struct udphdr *udp_hdr = (struct udphdr *)(skb->head + skb->transport_header);

    struct in6_addr laddr = make_ipv6_address(ip_hdr->saddr);
    struct in6_addr raddr = make_ipv6_address(ip_hdr->daddr);

    udp_update_stats(ctx, sk, skb, &laddr, udp_hdr->source, &raddr, udp_hdr->dest, 0);
  }

  if (ip_hdr->protocol == IPPROTO_TCP) {
    struct tcphdr *tcp_hdr = (struct tcphdr *)(skb->head + skb->transport_header);

    u16 flags = 0;
    bpf_probe_read(&flags, 2, ((u8 *)tcp_hdr) + 12);

    if (flags & TCP_FLAG_RST) {
      // bpf_trace_printk("on_ip_send_skb: tcp rst is set\n");
      // send RST (is_rx = 0)
      handle_tcp_reset(ctx, sk, 0);
    }
  }

  // Call on_ip_send_skb__2
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_ON_IP_SEND_SKB__2);

  return 0;
}

int on_ip6_send_skb(struct pt_regs *ctx, struct sk_buff *skb)
{
  struct sock *sk = skb->sk;
  struct ipv6hdr *ipv6_hdr = (struct ipv6hdr *)(skb->head + skb->network_header);

  GET_PID_TGID;

  if (ipv6_hdr->nexthdr == IPPROTO_UDP) {
    struct udphdr *udp_hdr = (struct udphdr *)(skb->head + skb->transport_header);

    struct in6_addr laddr = ipv6_hdr->saddr;
    struct in6_addr raddr = ipv6_hdr->daddr;

#if DEBUG_UDP_SOCKET_ERRORS
    __check_broken_in6_addr(&laddr, __LINE__);
    __check_broken_in6_addr(&raddr, __LINE__);
#endif

    udp_update_stats(ctx, sk, skb, &laddr, udp_hdr->source, &raddr, udp_hdr->dest, 0);
  }

  if (ipv6_hdr->nexthdr == IPPROTO_TCP) {
    struct tcphdr *tcp_hdr = (struct tcphdr *)(skb->head + skb->transport_header);

    u16 flags = 0;
    bpf_probe_read(&flags, 2, ((u8 *)tcp_hdr) + 12);

    if (flags & TCP_FLAG_RST) {
      // bpf_trace_printk("on_ip6_send_skb: tcp rst is set\n");
      // send RST (is_rx = 0)
      handle_tcp_reset(ctx, sk, 0);
    }
  }

  // Call on_ip6_send_skb__2
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_ON_IP6_SEND_SKB__2);

  return 0;
}

// Common handler -tail call- for receiving udp skb's
// step one, update stats, make sure udp socket exists
int handle_receive_udp_skb(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb)
{
  // find offsets for ip and udp headers
  char *skb_head = skb->head;
  __u16 transport_header = skb->transport_header;
  __u16 network_header = skb->network_header;

  // get the ip header
  struct iphdr *ip_hdr = (struct iphdr *)(skb_head + network_header);
  struct udphdr *udp_hdr = (struct udphdr *)(skb_head + transport_header);

  // get the version from the ip packet (common location for ipv4 and ipv6)
  u8 version;
  bpf_probe_read(&version, 1, (const u8 *)ip_hdr);
  version &= 0xF0;

  // Parse the addresses out of the header
  struct in6_addr laddr, raddr;
  if (version == 0x40) {
    // IPv4
    laddr = make_ipv6_address(ip_hdr->daddr);
    raddr = make_ipv6_address(ip_hdr->saddr);
  } else if (version & 0x60) {
    // IPV6
    laddr = ((struct ipv6hdr *)ip_hdr)->daddr;
    raddr = ((struct ipv6hdr *)ip_hdr)->saddr;
  } else {
    // Unknown IP Protocol version, possibly malformed packet received?
    bpf_log(ctx, BPF_LOG_UNREACHABLE, (u64)version, 0, 0);
    return 0;
  }

#if DEBUG_UDP_SOCKET_ERRORS
  if (__check_broken_in6_addr(&laddr, __LINE__) || __check_broken_in6_addr(&raddr, __LINE__)) {
    bpf_trace_printk("sk_family = %d, version = %u\n", sk->sk_family, (unsigned int)version);
    stack_trace(ctx);
  }
#endif

  udp_update_stats(ctx, sk, skb, &laddr, udp_hdr->dest, &raddr, udp_hdr->source, 1);

  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_HANDLE_RECEIVE_UDP_SKB__2);
  return 0;
}

// Common handler -tail call- for receiving udp skb's
// step two, check for receiving dns packets
int handle_receive_udp_skb__2(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb)
{
  GET_PID_TGID;
  const struct udphdr *hdr = (const struct udphdr *)(skb->head + skb->transport_header);
  perf_check_and_submit_dns(ctx, sk, skb, IPPROTO_UDP, hdr->source, hdr->dest, 1);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/* LIVE TCP */
static void report_rtt_estimator_if_time(struct pt_regs *ctx, struct sock *sk, struct tcp_open_socket_t *sk_info)
{
  u64 now = get_timestamp();

  if ((now - sk_info->last_output) < FILTER_NS)
    return; // too little time passed

  report_rtt_estimator(ctx, sk, sk_info, now, false);
}

int on_tcp_rtt_estimator(struct pt_regs *ctx, struct sock *sk)
{
  struct tcp_open_socket_t *sk_info; /* for filtering */

  if (sk->sk_state != TCP_ESTABLISHED) {
    return 0;
  }

  sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (!sk_info) {
    // Okay if socket is not yet tracked because it could be in kernel accept
    // queue but not returned by inet_csk_accept yet.
    return 0;
  }

  report_rtt_estimator_if_time(ctx, sk, sk_info);

  return 0;
}

int on_tcp_rcv_established(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb)
{
  struct tcp_open_socket_t *sk_info;
  sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (!sk_info) {
    // Okay if socket is not yet tracked because it could be in kernel accept
    // queue but not returned by inet_csk_accept yet.
    return 0;
  }

  int ret;
  u64 bytes_received = 0;
  ret = bpf_probe_read(&bytes_received, sizeof(bytes_received), &tcp_sk(sk)->bytes_received);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }

  if (bytes_received != sk_info->bytes_received) {
    /* update statistic for next report */
    sk_info->rcv_holes++;
    sk_info->rcv_delivered++;

    /* update the bytes_received so we won't report this occurrence again */
    sk_info->bytes_received = bytes_received;

    report_rtt_estimator_if_time(ctx, sk, sk_info);
  }

  return 0;
}

int on_tcp_event_data_recv(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb)
{
  struct tcp_open_socket_t *sk_info;
  sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
  if (!sk_info) {
    // Okay if socket is not yet tracked because it could be in kernel accept
    // queue but not returned by inet_csk_accept yet.
    return 0;
  }

  int ret;
  u64 bytes_received = 0;
  ret = bpf_probe_read(&bytes_received, sizeof(bytes_received), &tcp_sk(sk)->bytes_received);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return 0;
  }

  sk_info->rcv_delivered++;
  sk_info->bytes_received = bytes_received;

  report_rtt_estimator_if_time(ctx, sk, sk_info);

  return 0;
}

// SYN timeouts.
static void handle_syn_timeout(struct pt_regs *ctx, struct sock *sk)
{
  // is this a SYN timeout?
  if (((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) == 0) {
    // no, return.
    return;
  }

  if (!bpf_map_lookup_elem(&tcp_open_sockets, &sk)) {
    // don't send a retransmit event on sockets we never notified userspace about
    return;
  }

  // okay, can send an event
  u64 now = get_timestamp();
  perf_submit_agent_internal__tcp_syn_timeout(ctx, now, (__u64)sk);
}

// tcp_retransmit_timer is an ancient function which retained basically the
// same structure for a long time (since the initial git repo at 1da177e4c3f4,
// "Linux-2.6.12-rc2"). The static qualifier was removed in f1ecd5d9e7366
// (v2.6.32-rc1~703^2~172)
int on_tcp_retransmit_timer(struct pt_regs *ctx, struct sock *sk)
{
  handle_syn_timeout(ctx, sk);
  return 0;
}

// NOTE: tcp_syn_ack_timeout's signature was different pre v4.1 kernels.
// See kernel commit 42cb80a2353f4, (v4.1-rc1~128^2~175^2~6).
// the function seems to have been around since 72659ecce6858
// (v2.6.34-rc1~233^2~563).
int on_tcp_syn_ack_timeout(
    struct pt_regs *ctx,
    const struct request_sock *req)
{
  // Handle parameter differences between kernel versions
  struct sock *sock = NULL;
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 1, 0)) {
    // In older kernels, there's an additional struct sock *sock parameter
    sock = (struct sock *)PT_REGS_PARM2(ctx);
    req = (const struct request_sock *)PT_REGS_PARM3(ctx);
  }

#if TCP_STATS_ON_PARENT

  struct sock *sk = NULL;
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 4, 0)) {
    /* Linux<4.4 does not have req->rsk_listener */
    bpf_probe_read(&sk, sizeof(sk), &(req->sk));

    struct tcp_open_socket_t *sk_info;
    sk_info = bpf_map_lookup_elem(&tcp_open_sockets, &sk);
    if (!sk_info) {
      //  don't send a retransmit event on sockets we never notified userspace about
      return 0;
    }

    handle_syn_timeout(ctx, sk_info->parent);
  } else {
    handle_syn_timeout(ctx, (struct sock *)req->rsk_listener);
  }

#else

  struct sock *sk = NULL;
  bpf_probe_read(&sk, sizeof(sk), &(req->sk));
  if (sk == NULL) {
    bpf_log(ctx, BPF_LOG_UNREACHABLE, 0, 0, 0);
    return 0;
  }
  handle_syn_timeout(ctx, sk);

#endif
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/* DNS */

// Use stack-based approach on kernels below 4.15 (chosen using empirical testing).
// This is limited by max eBPF stack size (512 bytes). On newer kernels we can
// use per-CPU arrays for copying data which allows processing larger packets.

// Define maximum DNS packet length based on kernel version
static int get_dns_max_packet_len() {
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 15, 0)) {
    return 380; // limited by the eBPF stack size
  } else {
    return 512; // max size of DNS UDP packet
  }
}

#define DNS_MAX_PACKET_LEN (get_dns_max_packet_len())

// For newer kernels, define the structure and per-CPU array
struct dns_message_data {
  char data[512 + sizeof(struct bpf_agent_internal__dns_packet) + 16];
};
// use per-CPU array to overcome eBPF stack size limit
BPF_PERCPU_ARRAY(dns_message_array, struct dns_message_data, 1);
#pragma passthrough off

// Depending on when the skb is inspected, the header may or may not be filled
// in yet so we are passing in protocol and port components as parameters here
// sport and dport are in -network byte order-
static void
perf_check_and_submit_dns(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb, u8 proto, u16 sport, u16 dport, int is_rx)
{
  unsigned int len = 0;
  bpf_probe_read(&len, sizeof(len), &skb->len);

  // Filter for DNS requests and responses
  if (!((proto == IPPROTO_UDP) && ((sport == htons(53)) || (dport == htons(53))) && (len > 0))) {
    return;
  }

  // Read skb fields directly to avoid bpf verifier errors caused by bcc inconsistencies
  int ret;

  unsigned int skb_data_len = 0;
  ret = bpf_probe_read(&skb_data_len, sizeof(skb->data_len), &skb->data_len);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  unsigned char *from = NULL;
  ret = bpf_probe_read(&from, sizeof(skb->data), &skb->data);
  if (ret != 0 || from == NULL) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  unsigned char *skb_head = NULL;
  ret = bpf_probe_read(&skb_head, sizeof(skb->head), &skb->head);
  if (ret != 0 || skb_head == NULL) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  u16 skb_transport_header = 0;
  ret = bpf_probe_read(&skb_transport_header, sizeof(skb->transport_header), &skb->transport_header);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  u16 skb_network_header = 0;
  ret = bpf_probe_read(&skb_network_header, sizeof(skb->network_header), &skb->network_header);
  if (ret != 0) {
    bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, abs_val(ret), 0, 0);
    return;
  }

  /* we only take the linear part of the skb, not the paged part
   * 'valid_len' here is the amount of -non paged- data in the skb,
   * since 'data_len' refers to just the amount of -paged- data, and 'len'
   * is the total amount of skb data
   */
  unsigned int valid_len = len - skb_data_len;

  /* in e6afc8ace6dd5 (i.e., v4.7-rc1~154^2~349^2~1), udp_recvmsg started
     pulling the udp header before our kprobe. before then, skb->data pointed to
     the udp header. make sure we don't copy the udp header on pre-4.7 kernels
   */
  /* if data points to transport header (udp) then advance to the packet body */
  if (from == (skb_head + skb_transport_header)) {
    if (valid_len < sizeof(struct udphdr))
      return;
    from += sizeof(struct udphdr);
    valid_len -= sizeof(struct udphdr);
  }
  /* if data points to network header (ip) then advance to the packet body */
  else if (from == skb_head + skb_network_header) {
    from = skb_head + skb_transport_header + sizeof(struct udphdr);
    unsigned int diff = (from - (skb_head + skb_network_header));
    if (valid_len < diff)
      return;
    valid_len -= diff;
  }

  /* only enable this if we need to detect
  if (valid_len < len) {
    // we are facing a paged or fragmented skb. only copy headlen bytes
    bpf_log(ctx, BPF_LOG_DATA_TRUNCATED, (u64)len, (u64)valid_len, 0);
  }
  */

  // truncate dns packets at our maximum packet length right now
  if (valid_len > DNS_MAX_PACKET_LEN) {
    bpf_log(ctx, BPF_LOG_DATA_TRUNCATED, (u64)valid_len, (u64)DNS_MAX_PACKET_LEN, 0);
    valid_len = DNS_MAX_PACKET_LEN;
  }

  /* allocate buffer for event */
  char *buf;
  char stack_buf[380 + sizeof(struct bpf_agent_internal__dns_packet) + 16] = {};
  
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 15, 0)) {
    // use stack based buffer on older kernels
    buf = stack_buf;
  } else {
    // use bigger per-CPU array based buffer on newer kernels
    int zero = 0;
    struct dns_message_data *pkt = dns_message_array.lookup(&zero);
    if (pkt == NULL) {
      bpf_log(ctx, BPF_LOG_BPF_CALL_FAILED, 0, 0, 0);
      return;
    }
    buf = pkt->data;
  }

  /* the actual offset into buf has to start from buf's start */
  char *to = buf + bpf_agent_internal__dns_packet__data_size;

  bpf_probe_read(to, DNS_MAX_PACKET_LEN, from);

  struct bpf_agent_internal__dns_packet *const msg = (struct bpf_agent_internal__dns_packet *)&buf[0];
  struct jb_blob blob = {to, valid_len};
  bpf_fill_agent_internal__dns_packet(msg, get_timestamp(), (u64)sk, blob, len, is_rx);

  events.perf_submit(
      ctx, &msg->unpadded_size, ((DNS_MAX_PACKET_LEN + sizeof(struct jb_agent_internal__dns_packet) + 8 + 7) / 8) * 8 + 4);
}

// - Receive UDP packets ---------------------------------------
int on_skb_consume_udp(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb, int len)
{
  // Call handle_receive_udp_skb
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_HANDLE_RECEIVE_UDP_SKB);

  return 0;
}

// Compatibility layer for kernels pre skb_consume_udp (pre-4.10)
int on_skb_free_datagram_locked(struct pt_regs *ctx, struct sock *sk, struct sk_buff *skb, int len)
{
  // Handle parameter differences between kernel versions
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 9, 0)) {
    // In older kernels, there's no int len parameter
    // Parameters are: struct sock *sk, struct sk_buff *skb
    // len parameter doesn't exist, so we ignore it
  }
  
  // Call handle_receive_udp_skb
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_HANDLE_RECEIVE_UDP_SKB);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/* CGROUPS */

// Define structures and functions for older kernels (< 3.12)
struct css_id { /* From cgroup.c */
  struct cgroup_subsys_state __rcu *css;
  unsigned short id;
  unsigned short depth;
  struct rcu_head rcu_head;
  unsigned short stack[0]; /* Array of Length (depth+1) */
};

static u32 get_css_id(struct cgroup_subsys_state *css)
{
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(3, 12, 0)) {
    if (!css->id)
      return 0;
    u32 ssid = (u32)css->id->id;
    return ssid;
  } else {
    if (!css->ss)
      return 0;
    u32 ssid = (u32)css->ss->id;
    return ssid;
  }
}

static struct cgroup *get_css_parent_cgroup(struct cgroup_subsys_state *css)
{
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(3, 12, 0)) {
    struct cgroup *parent_cgroup = css->cgroup->parent;
    return parent_cgroup;
  } else {
    if (!css->parent)
      return NULL;
    struct cgroup *parent_cgroup = css->parent->cgroup;
    return parent_cgroup;
  }
}

static const char *get_cgroup_name(struct cgroup *cg)
{
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(3, 15, 0)) {
    return (const char *)&(cg->name->name[0]);
  } else {
    return cg->kn->name;
  }
}

// close

// Function that handles both kernel versions for killing CSS
int on_kill_css(struct pt_regs *ctx, struct cgroup_subsys_state *css)
{
  if (LINUX_KERNEL_VERSION >= KERNEL_VERSION(3, 12, 0)) {
    // For Kernel >= 3.12
    u32 ssid = get_css_id(css);
    if (ssid != FLOW_CGROUP_SUBSYS)
      return 0;

    struct cgroup *parent_cgroup = get_css_parent_cgroup(css);

    u64 now = get_timestamp();
    perf_submit_agent_internal__kill_css(
        ctx, now, (__u64)css->cgroup, (__u64)parent_cgroup, (void *)get_cgroup_name(css->cgroup));
    return 0;
  } else {
    // This should be called as on_cgroup_destroy_locked for older kernels
    return 0;
  }
}

// For Kernel < 3.12
int on_cgroup_destroy_locked(struct pt_regs *ctx, struct cgroup *cgrp)
{
  struct cgroup_subsys_state *css = NULL;
  bpf_probe_read(&css, sizeof(css), &(cgrp->subsys[FLOW_CGROUP_SUBSYS]));
  if (css == NULL)
    return 0;

  u64 now = get_timestamp();
  perf_submit_agent_internal__kill_css(ctx, now, (__u64)cgrp, (__u64)cgrp->parent, (void *)get_cgroup_name(cgrp));
  return 0;
}

// start

// Function that handles both kernel versions for populating CSS directories
int on_css_populate_dir(struct pt_regs *ctx, struct cgroup_subsys_state *css)
{
  if (LINUX_KERNEL_VERSION >= KERNEL_VERSION(4, 4, 0)) {
    // For Kernel >= 4.4
    u32 ssid = get_css_id(css);
    if (ssid != FLOW_CGROUP_SUBSYS)
      return 0;

    struct cgroup *parent_cgroup = get_css_parent_cgroup(css);

    u64 now = get_timestamp();
    perf_submit_agent_internal__css_populate_dir(
        ctx, now, (__u64)css->cgroup, (__u64)parent_cgroup, (void *)get_cgroup_name(css->cgroup));
    return 0;
  } else {
    // This should be called as on_cgroup_populate_dir for older kernels
    return 0;
  }
}

// For Kernel < 4.4
int on_cgroup_populate_dir(struct pt_regs *ctx, struct cgroup *cgrp, unsigned long subsys_mask)
{
  struct cgroup_subsys_state *css = NULL;
  bpf_probe_read(&css, sizeof(css), &(cgrp->subsys[FLOW_CGROUP_SUBSYS]));
  if (css == NULL)
    return 0;

  u64 now = get_timestamp();
  perf_submit_agent_internal__css_populate_dir(ctx, now, (__u64)cgrp, (__u64)cgrp->parent, (void *)get_cgroup_name(cgrp));
  return 0;
}

// existing cgroups v2
BEGIN_DECLARE_SAVED_ARGS(cgroup_control)
struct cgroup *cgrp;
END_DECLARE_SAVED_ARGS(cgroup_control)

int on_cgroup_control(struct pt_regs *ctx, struct cgroup *cgrp)
{
  // Only available for kernel >= 4.6.0
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 6, 0)) {
    return 0;
  }
  
  GET_PID_TGID;

  BEGIN_SAVE_ARGS(cgroup_control)
  SAVE_ARG(cgrp)
  END_SAVE_ARGS(cgroup_control)

  return 0;
}

int onret_cgroup_control(struct pt_regs *ctx)
{
  // Only available for kernel >= 4.6.0
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 6, 0)) {
    return 0;
  }
  
  GET_PID_TGID;

  GET_ARGS_MISSING_OK(cgroup_control, args)
  if (args == NULL) {
    return 0;
  }

  struct cgroup *cgrp = args->cgrp;

  DELETE_ARGS(cgroup_control);

  u16 subsys_mask = (u16)PT_REGS_RC(ctx);
  if (!(subsys_mask & 1 << FLOW_CGROUP_SUBSYS))
    return 0;

  u64 now = get_timestamp();
  struct cgroup *parent_cgroup = get_css_parent_cgroup(&cgrp->self);

  perf_submit_agent_internal__existing_cgroup_probe(ctx, now, (__u64)cgrp, (__u64)parent_cgroup, (void *)get_cgroup_name(cgrp));

  return 0;
}

// existing cgroups v1

// Function that handles both kernel versions for clone children read
int on_cgroup_clone_children_read_css(struct pt_regs *ctx, struct cgroup_subsys_state *css, struct cftype *cft)
{
  // For Kernel >= 3.12.0
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(3, 12, 0)) {
    return 0; // Should use the cgroup version instead
  }
  
  u32 subsys_mask = (u32)css->cgroup->root->subsys_mask;
  if (subsys_mask != 1 << FLOW_CGROUP_SUBSYS)
    return 0;

  u64 now = get_timestamp();
  struct cgroup *parent_cgroup = get_css_parent_cgroup(css);

  perf_submit_agent_internal__existing_cgroup_probe(
      ctx, now, (__u64)css->cgroup, (__u64)parent_cgroup, (void *)get_cgroup_name(css->cgroup));

  return 0;
}

// For Kernel < 3.12.0
int on_cgroup_clone_children_read(struct pt_regs *ctx, struct cgroup *cgrp, struct cftype *cft)
{
  u32 subsys_mask = (u32)cgrp->root->subsys_mask;
  if (subsys_mask != 1 << FLOW_CGROUP_SUBSYS)
    return 0;

  u64 now = get_timestamp();
  struct cgroup *parent_cgroup = cgrp->parent;

  perf_submit_agent_internal__existing_cgroup_probe(ctx, now, (__u64)cgrp, (__u64)parent_cgroup, (void *)get_cgroup_name(cgrp));

  return 0;
}

// modify
int on_cgroup_attach_task(struct pt_regs *ctx, struct cgroup *dst_cgrp, struct task_struct *leader, bool threadgroup)
{
  u32 subsys_mask = (u32)dst_cgrp->root->subsys_mask;
  if (subsys_mask != 1 << FLOW_CGROUP_SUBSYS)
    return 0;

  u64 now = get_timestamp();
  perf_submit_agent_internal__cgroup_attach_task(ctx, now, (__u64)dst_cgrp, leader->pid, leader->comm);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/* NAT */
/* end */
int on_nf_nat_cleanup_conntrack(struct pt_regs *ctx, struct nf_conn *ct)
{
  u64 now = get_timestamp();

  // filter out conntracks we haven't seen before, as only a subset of all
  // conntracks are monitored via on_nf_conntrack_alter_reply. All conntracks
  // will eventually be cleaned up via this function, but since only a subset
  // of conntracks (i.e. the nat-ed connections) go through
  // nf_conntrack_alert_reply, there will be a bunch of non-nat conntrack
  // entries that this probe will trigger on and which we don't need to process.
  // XXX: more testing and verification of this to be done after completion of
  // "existing" conntrack probing.
  struct nf_conn **seen_conntrack = seen_conntracks.lookup(&ct);
  if (!seen_conntrack) {
    return 0;
  }

  perf_submit_agent_internal__nf_nat_cleanup_conntrack(
      ctx,
      now,
      (u64)ct,
      (u32)ct->tuplehash[0].tuple.src.u3.ip,
      (u16)ct->tuplehash[0].tuple.src.u.all,
      (u32)ct->tuplehash[0].tuple.dst.u3.ip,
      (u16)ct->tuplehash[0].tuple.dst.u.all,
      (u8)ct->tuplehash[0].tuple.dst.protonum);

  bpf_map_delete_elem(&seen_conntracks, &ct);

  return 0;
}

/* start */
int on_nf_conntrack_alter_reply(struct pt_regs *ctx, struct nf_conn *ct, const struct nf_conntrack_tuple *newreply)
{
  u64 now = get_timestamp();

  // filter out non ipv4
  if ((u16)ct->tuplehash[0].tuple.src.l3num != AF_INET) {
    return 0;
  }

  // filter out non-tcp and non-udp
  u8 proto = (u8)newreply->dst.protonum;
  if (proto != IPPROTO_TCP && proto != IPPROTO_UDP) {
    return 0;
  }

  // Permit duplicates in this table, because they may represent a changed conntrack
  int ret = bpf_map_update_elem(&seen_conntracks, &ct, &ct, BPF_ANY);
  if (!ret) {
#if DEBUG_OTHER_MAP_ERRORS
    bpf_trace_printk("on_nf_conntrack_alter_reply: seen_conntracks table is full, dropping conntrack\n");
#endif
    return 0;
  }
  
  // Note that the nf_conntrack_tuple has dir=1, so we flip
  // src and dst when reporting the connection to the agent
  // to preserve four-tuple order.
  perf_submit_agent_internal__nf_conntrack_alter_reply(
      ctx,
      now,
      (u64)ct,
      (u32)ct->tuplehash[0].tuple.src.u3.ip,
      (u16)ct->tuplehash[0].tuple.src.u.all,
      (u32)ct->tuplehash[0].tuple.dst.u3.ip,
      (u16)ct->tuplehash[0].tuple.dst.u.all,
      (u8)ct->tuplehash[0].tuple.dst.protonum,
      (u32)newreply->dst.u3.ip,
      (u16)newreply->dst.u.all,
      (u32)newreply->src.u3.ip,
      (u16)newreply->src.u.all,
      (u8)newreply->dst.protonum);
  return 0;
}

int on_ctnetlink_dump_tuples(struct pt_regs *ctx, struct sk_buff *skb, const struct nf_conntrack_tuple *ct)
{
  u64 now = get_timestamp();

  // "struct nf_conn" contains two "struct nf_conntrack_tuple_hash". On the
  // dir=1 case, we want to report the addr of the dir=0, which we expected to
  // see first. this is addr is sizeof(struct nf_conntrack_tuple_hash) ahead of
  // the start of our current "ct"
  u64 ct_addr = (u64)ct;
  if (ct->dst.dir == 0) {
    perf_submit_agent_internal__existing_conntrack_tuple(
        ctx,
        now,
        (u64)ct_addr,
        (u32)ct->src.u3.ip,
        (u16)ct->src.u.all,
        (u32)ct->dst.u3.ip,
        (u16)ct->dst.u.all,
        (u8)ct->dst.protonum,
        (u8)ct->dst.dir);
  } else {
    ct_addr = ct_addr - sizeof(struct nf_conntrack_tuple_hash);
    // NOTE: in the dir=1 case, we flip src/dst to preserve four-tuple order.
    perf_submit_agent_internal__existing_conntrack_tuple(
        ctx,
        now,
        (u64)ct_addr,
        (u32)ct->dst.u3.ip,
        (u16)ct->dst.u.all,
        (u32)ct->src.u3.ip,
        (u16)ct->src.u.all,
        (u8)ct->dst.protonum,
        (u8)ct->dst.dir);
  }
  return 0;
}

//
// Include other modules
//

#include "tcp-processor/bpf_tcp_processor.c"
