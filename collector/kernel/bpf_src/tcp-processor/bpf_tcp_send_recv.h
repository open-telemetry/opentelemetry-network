/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

//
// bpf_tcp_send_recv.h - BPF TCP socket streaming receive/send buffering
//
// Requires:
//   To declare a tcp client handler, #define TCP_CLIENT_HANDLER(...)
//   To declare a tcp server handler, #define TCP_SERVER_HANDLER(...)

#pragma once

#include "bpf_debug.h"
#include "bpf_memory.h"
#include "bpf_tcp_socket.h"
#include "bpf_types.h"

#ifndef TCP_CLIENT_HANDLER
#error "Must define TCP_CLIENT_HANDLER"
#endif
#ifndef TCP_SERVER_HANDLER
#error "Must define TCP_SERVER_HANDLER"
#endif

////////////////////////////////////////////////////////////////////////////
// TCP Receive

BEGIN_DECLARE_SAVED_ARGS(tcp_recvmsg)
struct sock *sk;
struct msghdr *msg;
size_t len;
int nonblock;
int flags;
// int *addr_len;
// decoded msg
void *iov_base;
size_t iov_len;
int written;
int depth;
END_DECLARE_SAVED_ARGS(tcp_recvmsg)

////////////////////////////////////////////////////////////////////////////
// TCP Send

BEGIN_DECLARE_SAVED_ARGS(tcp_sendmsg)
struct sock *sk;
struct msghdr *msg;
size_t size;
// decoded msg
struct iovec *iov;
void *iov_ptr;
size_t iov_len;
size_t iov_offset;
unsigned long nr_segs;
int written;
int depth;
END_DECLARE_SAVED_ARGS(tcp_sendmsg)

////////////////////////////////////////////////////////////////////////////
// Send / Receive Stream Handlers
// Processes data that ends up in the send and receive streams
// to determine protocols and deal with them.

static void tcp_send_stream_handler(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    struct tcp_control_value_t *pctrl,
    enum STREAM_TYPE streamtype,
    const void *data,
    size_t data_len)
{
  // Are we an accepting socket, or an originating socket?
  if (pconn->parent_sk) {
    // Accepting sockets send from server
    TCP_SERVER_HANDLER(ctx, pconn, pctrl, streamtype, data, data_len);
  } else {
    // Originating sockets send from clients
    TCP_CLIENT_HANDLER(ctx, pconn, pctrl, streamtype, data, data_len);
  }
}

static void tcp_recv_stream_handler(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    struct tcp_control_value_t *pctrl,
    enum STREAM_TYPE streamtype,
    const void *data,
    size_t data_len)
{
  // Are we an accepting socket, or an originating socket?
  if (pconn->parent_sk) {
    // Accepting sockets receive from clients
    TCP_CLIENT_HANDLER(ctx, pconn, pctrl, streamtype, data, data_len);
  } else {
    // Originating sockets receive from servers
    TCP_SERVER_HANDLER(ctx, pconn, pctrl, streamtype, data, data_len);
  }
}

////////////////////////////////////////////////////////////////////////////

// --- tcp_sendmsg ----------------------------------------------------
// Called when data is to be send to a TCP socket

#pragma passthrough on
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)
int handle_kprobe__tcp_sendmsg(struct pt_regs *ctx, struct kiocb *iocb, struct sock *sk, struct msghdr *msg, size_t size)
#else
int handle_kprobe__tcp_sendmsg(struct pt_regs *ctx, struct sock *sk, struct msghdr *msg, size_t size)
#endif
#pragma passthrough off
{
  GET_PID_TGID

  struct tcp_connection_t *pconn;
  pconn = lookup_tcp_connection(sk);
  if (!pconn) {
    // For now ignore sends on sockets we haven't seen the tcp_init_sock for
    //#if TRACE_TCP_SEND
    //    DEBUG_PRINTK("tcp_sendmsg found no tcp connection in kprobe\n");
    //#endif
    return 0;
  }
  struct tcp_control_value_t *pctrl = get_tcp_control(pconn);
  if (!pctrl) {
    return 0;
  }

  // Quick ignore for sockets we deem uninteresting

#ifndef ENABLE_TCP_DATA_STREAM
  if (pconn->protocol_state.candidates == 0) {
#if TRACE_TCP_SEND
    DEBUG_PRINTK("no candidates left on tcp_sendmsg\n");
#endif
    return 0;
  }
  if (pconn->streams[ST_SEND].protocol_count == 0) {
#if TRACE_TCP_SEND
    DEBUG_PRINTK("no protocols interested in tcp_sendmsg\n");
#endif
    return 0;
  }
#endif

  if (pctrl->streams[ST_SEND].enable == 0) {
#if TRACE_TCP_SEND
    DEBUG_PRINTK("send stream is disabled for sk=%llx\n", (u64)sk);
#endif
    return 0;
  }

  // decode msg

  struct iovec *iov = NULL;
  unsigned long nr_segs = 0;
  size_t iov_offset = 0;
#pragma passthrough on
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
  bpf_probe_read(&iov, sizeof(iov), &(msg->msg_iov));
  bpf_probe_read(&nr_segs, sizeof(nr_segs), &(msg->msg_iovlen));
  // iov_offset is not a thing in older kernels
#else
  int type = 0;
  bpf_probe_read(&type, sizeof(type), &(msg->msg_iter.type));
  // ensure this is an IOVEC or KVEC, low bit indicates read/write
  // note: iov_iter.iov and iov_iter.kvec are union and have same layout
  // first condition makes it work on pre-5 kernels where ITER_IOVEC=0
  if (type > 1 && !(type & (ITER_IOVEC | ITER_KVEC))) {
#if DEBUG_TCP_SEND
    DEBUG_PRINTK("unsupported iov type: %d\n", type);
#endif
    bpf_log(ctx, BPF_LOG_UNSUPPORTED_IO, (u64)ST_SEND, (u64)sk, (u64)type);
    return 0;
  }
  // can access through iov since iov and kvec are union and have same layout
  bpf_probe_read(&iov, sizeof(iov), &(msg->msg_iter.iov));
  bpf_probe_read(&nr_segs, sizeof(nr_segs), &(msg->msg_iter.nr_segs));
  bpf_probe_read(&iov_offset, sizeof(iov_offset), &(msg->msg_iter.iov_offset));
#endif
#pragma passthrough off

  void *iov_ptr = NULL;
  size_t iov_len = 0;
  int written = 0;
  int depth = 1;

  // Defer arguments to kretprobe
  BEGIN_SAVE_ARGS(tcp_sendmsg)
  SAVE_ARG(sk)
  SAVE_ARG(msg)
  SAVE_ARG(size)
  // decoded msg structure
  SAVE_ARG(iov)
  SAVE_ARG(iov_ptr)
  SAVE_ARG(iov_len)
  SAVE_ARG(iov_offset)
  SAVE_ARG(nr_segs)
  SAVE_ARG(written)
  SAVE_ARG(depth)
  END_SAVE_ARGS(tcp_sendmsg)

#if TRACE_TCP_SEND
  DEBUG_PRINTK(
      "tcp_sendmsg enter: pid %u request to send up to %u bytes "
      "on socket %llx\n",
      pconn->upid,
      (unsigned int)size,
      sk);
  DEBUG_PRINTK("                   nr_segs=%lu, iov_offset=%lu\n", nr_segs, iov_offset);
#endif

  return 0;
}

int handle_kretprobe__tcp_sendmsg(struct pt_regs *ctx)
{
  // This call recurses up to TCP_TAIL_CALL_MAX_DEPTH times,
  // writing up to DATA_CHANNEL_CHUNK_MAX each time
  tail_calls.call(ctx, TAIL_CALL_CONTINUE_TCP_SENDMSG);
  return 0;
}

int continue_tcp_sendmsg(struct pt_regs *ctx)
{
  GET_PID_TGID

  GET_ARGS_MISSING_OK(tcp_sendmsg, args)
  if (args == NULL) {
    return 0;
  }

  // Get copied byte count
  int copied = (int)PT_REGS_RC(ctx);
  if (copied <= 0) {
    DELETE_ARGS(tcp_sendmsg);
    return 0;
  }

  // Lookup our connection
  struct tcp_connection_t *pconn;
  pconn = lookup_tcp_connection(args->sk);
  if (!pconn) {
    // Socket was destroyed -during- tcp_sendmsg (happens on some control flow paths?)
#if DEBUG_TCP_SEND
    DEBUG_PRINTK("tcp_sendmsg found no tcp connection in kretprobe\n");
    bpf_log(ctx, BPF_LOG_TABLE_MISSING_KEY, BPF_TABLE_TCP_CONNECTIONS, (u64)args->sk, _tgid);
#endif
    DELETE_ARGS(tcp_sendmsg);
    return 0;
  }

  if (args->iov_len == 0) {
    // Load next iovec
    if (args->nr_segs > 0) {
      void *iov_ptr = NULL;
      size_t iov_len = 0;
      bpf_probe_read(&iov_ptr, sizeof(iov_ptr), &(args->iov->iov_base));
      bpf_probe_read(&iov_len, sizeof(iov_len), &(args->iov->iov_len));
#if TRACE_TCP_SEND
      DEBUG_PRINTK("tcp_sendmsg continue: ptr=%llx, len=%d\n", iov_ptr, iov_len);
#endif
      args->iov_ptr = iov_ptr;
      args->iov_len = iov_len;
      args->iov++;
      args->nr_segs--;
      if (args->iov_offset) {
        // Data in the first iovec can have an offset to the first byte
        args->iov_ptr += args->iov_offset;
        args->iov_offset = 0;
      }
    } else {
      DELETE_ARGS(tcp_sendmsg);
      return 0;
    }
  }

  const u8 *data = (const u8 *)args->iov_ptr;
  int remaining = copied - args->written;
  int to_copy = remaining > DATA_CHANNEL_CHUNK_MAX ? DATA_CHANNEL_CHUNK_MAX : remaining;

  if (to_copy > args->iov_len) {
    to_copy = args->iov_len;
  }

  write_to_tcp_stream(ctx, pconn, ST_SEND, data, to_copy, tcp_send_stream_handler);

  args->written += to_copy;
  args->iov_ptr += to_copy;
  args->iov_len -= to_copy;

  if (to_copy == remaining || args->depth == TCP_TAIL_CALL_MAX_DEPTH) {
    DELETE_ARGS(tcp_sendmsg);
  } else {
    args->depth++;
    tail_calls.call(ctx, TAIL_CALL_CONTINUE_TCP_SENDMSG);
  }

  return 0;
}

// --- tcp_recvmsg ----------------------------------------------------
// Called when data is to be received by a TCP socket

#pragma passthrough on
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)
int handle_kprobe__tcp_recvmsg(
    struct pt_regs *ctx,
    struct kiocb *iocb,
    struct sock *sk,
    struct msghdr *msg,
    size_t len,
    int nonblock,
    int flags /*, int *addr_len*/)
#else
int handle_kprobe__tcp_recvmsg(
    struct pt_regs *ctx, struct sock *sk, struct msghdr *msg, size_t len, int nonblock, int flags, int *addr_len)
#endif
#pragma passthrough off
{
  GET_PID_TGID

  struct tcp_connection_t *pconn;
  pconn = lookup_tcp_connection(sk);
  if (!pconn) {
    // Ignore receives on sockets we haven't seen the tcp_init_sock for
    //#if TRACE_TCP_RECEIVE
    //    DEBUG_PRINTK("tcp_recvmsg found no tcp connection in kprobe\n");
    //#endif
    return 0;
  }
  struct tcp_control_value_t *pctrl = get_tcp_control(pconn);
  if (!pctrl) {
    return 0;
  }

  // Quick ignore for sockets we deem uninteresting

#ifndef ENABLE_TCP_DATA_STREAM
  if (pconn->protocol_state.candidates == 0) {
#if TRACE_TCP_RECEIVE
    DEBUG_PRINTK("no candidates left on tcp_recvmsg\n");
#endif
    return 0;
  }
  if (pconn->streams[ST_RECV].protocol_count == 0) {
#if TRACE_TCP_RECEIVE
    DEBUG_PRINTK("no protocols interested in tcp_recvmsg\n");
#endif
    return 0;
  }
#endif

  if (pctrl->streams[ST_RECV].enable == 0) {
#if TRACE_TCP_RECEIVE
    DEBUG_PRINTK("recv stream is disabled for sk=%llx\n", (u64)sk);
#endif
    return 0;
  }

  // decode msg

  struct iovec *iov = NULL;
#pragma passthrough on
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
  bpf_probe_read(&iov, sizeof(iov), &(msg->msg_iov));
#else
  int type = 0;
  bpf_probe_read(&type, sizeof(type), &(msg->msg_iter.type));
  // ensure this is an IOVEC or KVEC, low bit indicates read/write
  // note: iov_iter.iov and iov_iter.kvec are union and have same layout
  // first condition makes it work on pre-5 kernels where ITER_IOVEC=0
  if (type > 1 && !(type & (ITER_IOVEC | ITER_KVEC))) {
#if DEBUG_TCP_RECEIVE
    DEBUG_PRINTK("unsupported iov type: %d\n", type);
#endif
    bpf_log(ctx, BPF_LOG_UNSUPPORTED_IO, (u64)ST_RECV, (u64)sk, (u64)type);
    return 0;
  }
  // can access through iov since iov and kvec are union and have same layout
  bpf_probe_read(&iov, sizeof(iov), &(msg->msg_iter.iov));
#endif
#pragma passthrough off

  void *iov_base = NULL;
  size_t iov_len = 0;
  int written = 0;
  int depth = 1;
  bpf_probe_read(&iov_base, sizeof(iov_base), &(iov->iov_base));
  bpf_probe_read(&iov_len, sizeof(iov_len), &(iov->iov_len));

  // Add to receiver table
  BEGIN_SAVE_ARGS(tcp_recvmsg)
  SAVE_ARG(sk)
  SAVE_ARG(msg)
  SAVE_ARG(len)
  SAVE_ARG(nonblock)
  SAVE_ARG(flags)
  // SAVE_ARG(addr_len)
  // decoded msg structure
  SAVE_ARG(iov_base)
  SAVE_ARG(iov_len)
  SAVE_ARG(written)
  SAVE_ARG(depth)
  END_SAVE_ARGS(tcp_recvmsg)

#if TRACE_TCP_RECEIVE
  DEBUG_PRINTK(
      "tcp_recvmsg enter: pid %u request to receive up to %u "
      "bytes on socket %llx\n",
      pconn->upid,
      (unsigned int)len,
      sk);
  DEBUG_PRINTK("                   iov_base=%llx, iov_len=%d\n", iov_base, iov_len);

#endif

  return 0;
}

int handle_kretprobe__tcp_recvmsg(struct pt_regs *ctx)
{
  // This call recurses up to TCP_TAIL_CALL_MAX_DEPTH times,
  // writing up to DATA_CHANNEL_CHUNK_MAX each time
  tail_calls.call(ctx, TAIL_CALL_CONTINUE_TCP_RECVMSG);
  return 0;
}

int continue_tcp_recvmsg(struct pt_regs *ctx)
{
  GET_PID_TGID

  GET_ARGS_MISSING_OK(tcp_recvmsg, args)
  if (args == NULL) {
    return 0;
  }

  // Get copied byte count
  int copied = (int)PT_REGS_RC(ctx);
  if (copied <= 0) {
    DELETE_ARGS(tcp_recvmsg);
    return 0;
  }

  // Lookup our connection
  struct tcp_connection_t *pconn;
  pconn = lookup_tcp_connection(args->sk);
  if (!pconn) {
    // Socket was destroyed -during- tcp_recvmsg (happens on some control flow paths?)
#if DEBUG_TCP_RECEIVE
    DEBUG_PRINTK("tcp_recvmsg found no tcp connection in kretprobe\n");
    bpf_log(ctx, BPF_LOG_TABLE_MISSING_KEY, BPF_TABLE_TCP_CONNECTIONS, (u64)args->sk, _tgid);
#endif
    DELETE_ARGS(tcp_recvmsg);
    return 0;
  }

  const u8 *data = ((const u8 *)args->iov_base) + args->written;
  int remaining = copied - args->written;
  int to_copy = remaining > DATA_CHANNEL_CHUNK_MAX ? DATA_CHANNEL_CHUNK_MAX : remaining;

  write_to_tcp_stream(ctx, pconn, ST_RECV, data, to_copy, tcp_recv_stream_handler);

  args->written += to_copy;

  if (to_copy == remaining || args->depth == TCP_TAIL_CALL_MAX_DEPTH) {
    DELETE_ARGS(tcp_recvmsg);
  } else {
    args->depth++;
    tail_calls.call(ctx, TAIL_CALL_CONTINUE_TCP_RECVMSG);
  }

  return 0;
}
