/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

//
// bpf_inet_csk_accept.h - BPF handler for connection-based socket accepts
//

#pragma once

#include "bpf_debug.h"
#include "bpf_memory.h"
#include "bpf_tcp_socket.h"
#include "bpf_types.h"

extern int LINUX_KERNEL_VERSION __kconfig;

BEGIN_DECLARE_SAVED_ARGS(inet_csk_accept)
struct sock *sk;
int flags;
u32 _pad_0; // required alignment for bcc
int *err;
END_DECLARE_SAVED_ARGS(inet_csk_accept)

// --- inet_csk_accept --------------------------------------------------
// Called when a listen socket accepts gets a connection
SEC("kprobe/inet_csk_accept")
int handle_kprobe__inet_csk_accept(struct pt_regs *ctx, struct sock *sk, int flags, int *err, bool kern)
{
  // Handle parameter differences between kernel versions
  if (LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 11, 0)) {
    // In older kernels, there's no bool kern parameter
    // Parameters are: struct sock *sk, int flags, int *err
    // The kern parameter doesn't exist, so we ignore it
  }
  GET_PID_TGID

  // Ensure the parent socket has a tcp_connection
  struct tcp_connection_t *psk_info;
  psk_info = lookup_tcp_connection(sk);
  if (!psk_info) {
    psk_info = create_tcp_connection(ctx, sk);
    if (!psk_info) {
#if DEBUG_TCP_CONNECTION
      DEBUG_PRINTK("inet_csk_accept: couldn't create tcp_connection");
#endif
      bpf_log(ctx, BPF_LOG_TABLE_BAD_INSERT, BPF_TABLE_TCP_CONNECTIONS, _tgid, (u64)sk);
      return 0;
    }
  }

  // Link us to our kretprobe
  BEGIN_SAVE_ARGS(inet_csk_accept)
  SAVE_ARG(sk)
  SAVE_ARG(flags)
  SAVE_ARG(err)
  END_SAVE_ARGS(inet_csk_accept)

#if TRACE_SOCKET_ACCEPT
  DEBUG_PRINTK("inet_csk_accept enter: pid %u accepting socket on %llx\n", psk_info->upid, sk);
  DEBUG_PRINTK("                       pid_tgid %llu added\n", _pid_tgid);
#endif

  return 0;
}
SEC("kretprobe/inet_csk_accept")
int handle_kretprobe__inet_csk_accept(struct pt_regs *ctx)
{
  GET_PID_TGID

  // Ensure the accept succeeded
  struct sock *newsk = (struct sock *)PT_REGS_RC(ctx);
  if (!newsk) {

#if TRACE_SOCKET_ACCEPT
    DEBUG_PRINTK("                       accept failed\n");
    DEBUG_PRINTK("inet_csk_accept exit:  pid_tgid %llu removed\n", _pid_tgid);
#endif
    // Unlink the kprobe/kretprobe
    DELETE_ARGS(inet_csk_accept);
    return 0;
  }

  // Link us from our kprobe
  GET_ARGS(inet_csk_accept, args);
  if (args == NULL) {
    // Race condition where we might have been inside an accept when the probe
    // was inserted, ignore
    return 0;
  }

  // Set up the child socket
  struct tcp_connection_t *pconn = create_tcp_connection(ctx, newsk);
  if (!pconn) {
#if DEBUG_TCP_CONNECTION
    DEBUG_PRINTK("inet_csk_accept(ret): couldn't create tcp_connection");
#endif
    bpf_log(ctx, BPF_LOG_TABLE_BAD_INSERT, BPF_TABLE_TCP_CONNECTIONS, _tgid, (u64)newsk);
    // Unlink the kprobe/kretprobe
    DELETE_ARGS(inet_csk_accept);
    return 0;
  }

  // Note that this is a listening socket
  pconn->parent_sk = args->sk;

#if TRACE_SOCKET_ACCEPT
  DEBUG_PRINTK("                       accepted socket %llx\n", newsk);
  DEBUG_PRINTK("inet_csk_accept exit:  pid_tgid %llu removed\n", _pid_tgid);
#endif
  // Unlink the kprobe/kretprobe
  DELETE_ARGS(inet_csk_accept);

  return 0;
}
