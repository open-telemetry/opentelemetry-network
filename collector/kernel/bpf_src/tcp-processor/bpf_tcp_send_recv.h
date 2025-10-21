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

////////////////////////////////////////////////////////////////////////////
// Helpers: CO-RE safe iov_iter checks and resolution

/*
 * Returns true if msg->msg_iter designates ITER_IOVEC or ITER_KVEC across
 * kernel versions. Uses enum CO-RE relocation to avoid numeric constants.
 * Optionally returns the iter type value for logging (if type_out!=NULL).
 */
static __always_inline bool msg_iter_is_iov_or_kvec(const struct msghdr *msg, unsigned int *type_out)
{
  // Default iter type value for logging
  unsigned int iter_type = 0;

  // Require enum values to exist; otherwise, return false (non-permissive)
  bool has_iov = bpf_core_enum_value_exists(enum iter_type, ITER_IOVEC);
  bool has_kvec = bpf_core_enum_value_exists(enum iter_type, ITER_KVEC);
  if (!(has_iov && has_kvec)) {
    if (type_out)
      *type_out = iter_type;
    return false;
  }
  __u64 ev_iov = bpf_core_enum_value(enum iter_type, ITER_IOVEC);
  __u64 ev_kvec = bpf_core_enum_value(enum iter_type, ITER_KVEC);

  // Prefer ordinal-era field (>= 5.14): iter_type is an enum-ordinal (u8)
  if (bpf_core_field_exists(((struct msghdr *)0)->msg_iter.iter_type)) {
    __u8 it = BPF_CORE_READ(msg, msg_iter.iter_type);
    iter_type = it;

    if (type_out)
      *type_out = iter_type;
    return it == ev_iov || it == ev_kvec;
  }

  // Bitmask-era (<= 5.13): use compat msghdr to read iov_iter.type
  struct msghdr___5_13_19 *msg_compat = (void *)msg;
  if (msg_compat) {
    if (bpf_probe_read_kernel(&iter_type, sizeof(iter_type), &msg_compat->msg_iter.type) == 0) {
      if (type_out)
        *type_out = iter_type;
      unsigned int mask = (unsigned int)(ev_iov | ev_kvec);
      return (iter_type & mask) != 0;
    }
  }

  // If all else fails, return false and pass through iter_type (0)
  if (type_out)
    *type_out = iter_type;
  return false;
}

// Resolve iovec pointer across kernel versions: use __iov if present (6.4+),
// else fall back to iov (<= 6.3). Layout matches kvec, so it's valid either way.
static __always_inline struct iovec *msg_iter_get_iov(struct msghdr *msg)
{
  if (bpf_core_field_exists(((struct msghdr *)0)->msg_iter.__iov)) {
    return (struct iovec *)BPF_CORE_READ(msg, msg_iter.__iov);
  }
  return (struct iovec *)BPF_CORE_READ((struct msghdr___5_13_19 *)msg, msg_iter.iov);
}

/*
 * Returns true if msg->msg_iter designates ITER_UBUF on kernels that support it.
 * Uses CO-RE field/enum relocation to avoid relying on numeric enum values.
 *
 * Behavior across eras:
 * - <=5.13: no iter_type field and no UBUF; returns false.
 * - 5.14–5.19: iter_type exists but UBUF not present; returns false.
 * - >=6.0: iter_type exists and ITER_UBUF may exist; compare ordinal safely.
 */
static __always_inline bool iter_is_ubuf(const struct msghdr *msg)
{
  if (!bpf_core_field_exists(((struct msghdr *)0)->msg_iter.iter_type))
    return false; // bitmask-era: no UBUF

  if (!bpf_core_enum_value_exists(enum iter_type, ITER_UBUF))
    return false; // target kernel lacks UBUF support

  __u8 it = BPF_CORE_READ(msg, msg_iter.iter_type);
  __u64 ev_ubuf = bpf_core_enum_value(enum iter_type, ITER_UBUF);
  return it == ev_ubuf;
}

/*
 * Resolve UBUF as an iovec-like (base,len) pair across kernel versions.
 * - 6.4+: prefer __ubuf_iovec overlay (base/len). iov_offset is outside overlay.
 * - 6.0–6.3: use ubuf pointer and count fields. Caller may apply iov_offset.
 */
static __always_inline void ubuf_as_iovec(const struct msghdr *msg, void **out_base, size_t *out_len)
{
  // 6.4+ overlay: exposes base/len as struct iovec
  if (bpf_core_field_exists(((struct msghdr *)0)->msg_iter.__ubuf_iovec)) {
    void *base = BPF_CORE_READ(msg, msg_iter.__ubuf_iovec.iov_base);
    size_t len = BPF_CORE_READ(msg, msg_iter.__ubuf_iovec.iov_len);
    *out_base = base;
    *out_len = len;
    return;
  }

  // 6.0–6.3: plain ubuf pointer + count
  void *ubuf = BPF_CORE_READ(msg, msg_iter.ubuf);
  size_t cnt = BPF_CORE_READ(msg, msg_iter.count);
  *out_base = ubuf;
  *out_len = cnt;
}

static __always_inline void tcp_send_stream_handler(
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

static __always_inline void tcp_recv_stream_handler(
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

SEC("kprobe/tcp_sendmsg")
__attribute__((noinline)) int handle_kprobe__tcp_sendmsg(struct pt_regs *ctx)
{
  // In post 4.1 kernels: struct sock *sk, struct msghdr *msg, size_t size
  struct sock *sk = (struct sock *)PT_REGS_PARM1(ctx);
  struct msghdr *msg = (struct msghdr *)PT_REGS_PARM2(ctx);
  size_t size = (size_t)PT_REGS_PARM3(ctx);

  GET_PID_TGID

  if (!msg || !sk) {
    return 0;
  }

  struct tcp_connection_t *pconn;
  pconn = lookup_tcp_connection(sk);
  if (!pconn) {
    // For now ignore sends on sockets we haven't seen the tcp_init_sock for
    // #if TRACE_TCP_SEND
    //    DEBUG_PRINTK("tcp_sendmsg found no tcp connection in kprobe\n");
    // #endif
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
  unsigned int iter_type = 0;
  void *iov_ptr = NULL;
  size_t iov_len = 0;
  int written = 0;
  int depth = 1;

  if (iter_is_ubuf(msg)) {
    // ITER_UBUF: treat as a single contiguous segment
    ubuf_as_iovec(msg, &iov_ptr, &iov_len);
  } else if (msg_iter_is_iov_or_kvec(msg, &iter_type)) {
    // IOVEC/KVEC: can access through iov since both share layout
    iov = (struct iovec *)msg_iter_get_iov(msg);
    if (iov) {
      iov_ptr = BPF_CORE_READ(iov, iov_base);
      iov_len = BPF_CORE_READ(iov, iov_len);
    }
  } else {
#if DEBUG_TCP_SEND
    DEBUG_PRINTK("unsupported iov type: %u\n", iter_type);
#endif
    bpf_log(ctx, BPF_LOG_UNSUPPORTED_IO, (u64)ST_SEND, (u64)sk, (u64)iter_type);
    return 0;
  }
  nr_segs = BPF_CORE_READ(msg, msg_iter.nr_segs);
  iov_offset = BPF_CORE_READ(msg, msg_iter.iov_offset);

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

SEC("kretprobe/tcp_sendmsg")
int handle_kretprobe__tcp_sendmsg(struct pt_regs *ctx)
{
  // This call recurses up to TCP_TAIL_CALL_MAX_DEPTH times,
  // writing up to DATA_CHANNEL_CHUNK_MAX each time
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_CONTINUE_TCP_SENDMSG);
  return 0;
}

SEC("kprobe")
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
      void *iov_ptr = args->iov_ptr;
      size_t iov_len = args->iov_len;
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
    bpf_tail_call(ctx, &tail_calls, TAIL_CALL_CONTINUE_TCP_SENDMSG);
  }

  return 0;
}

// --- tcp_recvmsg ----------------------------------------------------
// Called when data is to be received by a TCP socket

SEC("kprobe/tcp_recvmsg")
int handle_kprobe__tcp_recvmsg(struct pt_regs *ctx)
{
  // In kernels 4.1 onwards: struct sock *sk, struct msghdr *msg, size_t len, int nonblock, int flags, int *addr_len
  struct sock *sk = (struct sock *)PT_REGS_PARM1(ctx);
  struct msghdr *msg = (struct msghdr *)PT_REGS_PARM2(ctx);
  size_t len = (size_t)PT_REGS_PARM3(ctx);
  int nonblock = (int)PT_REGS_PARM4(ctx);
  int flags = (int)PT_REGS_PARM5(ctx);
  // int *addr_len = NULL;   -- unused

  GET_PID_TGID

  struct tcp_connection_t *pconn;
  pconn = lookup_tcp_connection(sk);
  if (!pconn) {
    // Ignore receives on sockets we haven't seen the tcp_init_sock for
    // #if TRACE_TCP_RECEIVE
    //    DEBUG_PRINTK("tcp_recvmsg found no tcp connection in kprobe\n");
    // #endif
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
  void *iov_base = NULL;
  size_t iov_len = 0;
  int written = 0;
  int depth = 1;
  unsigned int iter_type = 0;

  if (iter_is_ubuf(msg)) {
    // ITER_UBUF: read as iovec-like values
    ubuf_as_iovec(msg, &iov_base, &iov_len);
  } else if (msg_iter_is_iov_or_kvec(msg, &iter_type)) {
    // IOVEC/KVEC: can access through iov since both share layout
    iov = (struct iovec *)msg_iter_get_iov(msg);
    iov_base = BPF_CORE_READ(iov, iov_base);
    iov_len = BPF_CORE_READ(iov, iov_len);
  } else {
#if DEBUG_TCP_RECEIVE
    DEBUG_PRINTK("unsupported iov type: %u\n", iter_type);
#endif
    bpf_log(ctx, BPF_LOG_UNSUPPORTED_IO, (u64)ST_RECV, (u64)sk, (u64)iter_type);
    return 0;
  }

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

SEC("kretprobe/tcp_recvmsg")
int handle_kretprobe__tcp_recvmsg(struct pt_regs *ctx)
{
  // This call recurses up to TCP_TAIL_CALL_MAX_DEPTH times,
  // writing up to DATA_CHANNEL_CHUNK_MAX each time
  bpf_tail_call(ctx, &tail_calls, TAIL_CALL_CONTINUE_TCP_RECVMSG);
  return 0;
}

SEC("kprobe")
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
    bpf_tail_call(ctx, &tail_calls, TAIL_CALL_CONTINUE_TCP_RECVMSG);
  }

  return 0;
}
