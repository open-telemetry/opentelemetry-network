/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

//
// bpf_tcp_connection.h - TCP socket tracking
//
// Requires:
//   Define list of protocols prior to including this file as well and #define
//   index type to TCP_PROTOCOL_TYPE and #define the number of protocols to
//   TCP_PROTOCOL_COUNT, and TCP_PROTOCOL_MASK to (1<<TCP_PROTOCOL_COUNT)-1
//   Set the TCP socket buffer size, #define TCP_SOCKET_BUFFER_SIZE, which
//   must be a multiple of 8 bytes

#include <net/sock.h>

#include "bpf_debug.h"
#include "bpf_types.h"

#ifndef TCP_CONNECTION_HASH_SIZE
#error "Must define a hash size for tcp_connections table"
#endif

// Must be the maximum size required for any protocol decoding
#define TCP_SOCKET_PROTOCOL_STATE_SIZE 16

// Verify there is a TCP_PROTOCOL_TYPE type
#ifndef TCP_PROTOCOL_TYPE
#define TCP_PROTOCOL_TYPE u32
#endif

#ifndef TCP_PROTOCOL_COUNT
#error "Must define a number of potential TCP protocols to detect"
#endif

struct tcp_stream_info_t {
#ifndef ENABLE_TCP_DATA_STREAM
  u16 protocol_count; // # of protocols that care about this data (0 means
                      // we can stop parsing the data)
  u16 _pad0;
  u32 _pad1;
#endif
  u64 total; // # of bytes transmitted on this socket in this direction
};

struct tcp_protocol_state_t {
  TCP_PROTOCOL_TYPE protocol;              // detected protocol
  u32 candidates;                          // bit mask of candidate protocols (0 bit = disqualified
                                           // candidate, 1 bit=possible candidate)
  u8 data[TCP_SOCKET_PROTOCOL_STATE_SIZE]; // internal data used by protocol
};

// TCP Connection Data
struct tcp_connection_t {
  struct sock *sk;                     // the socket we're keyed to
  struct sock *parent_sk;              // parent listen socket if this an accepted socket
  TGID upid;                           // userland PID of this connection
  struct tcp_stream_info_t streams[2]; // state tracking for send and receive streams
#ifndef ENABLE_TCP_DATA_STREAM
  struct tcp_protocol_state_t protocol_state; // protocol detection state
#endif
};

BPF_HASH(_tcp_connections, struct sock *, struct tcp_connection_t, TCP_CONNECTION_HASH_SIZE);

BPF_HASH(_tcp_control, struct tcp_control_key_t, struct tcp_control_value_t, TCP_CONNECTION_HASH_SIZE);

//
// TCP Connection Lifecycle Management
//

static struct tcp_connection_t *lookup_tcp_connection(struct sock *sk)
{
  //#if TRACE_TCP_CONNECTION
  //  DEBUG_PRINTK("tcp_connections.lookup(%llx)\n", sk);
  //#endif

  struct tcp_connection_t *pconn = _tcp_connections.lookup(&sk);
  return pconn;
}

static struct tcp_connection_t *create_tcp_connection(struct pt_regs *ctx, struct sock *sk)
{
  struct tcp_connection_t *pconn = _tcp_connections.lookup(&sk);
  if (pconn) {
#if TCP_LIFETIME_HACK
#if DEBUG_TCP_CONNECTION
    DEBUG_PRINTK("create_tcp_connection: tcp_lifetime_hack\n");
#endif
    // xxx: disable this for now because we know it happens all the time and it's too chatty
    // bpf_log(ctx, BPF_LOG_LIFETIME_HACK, BPF_TABLE_TCP_CONNECTIONS, (u64)sk, 0);
    _tcp_connections.delete(&sk);
#else
    DEBUG_PRINTK("create_tcp_connection: socket already exists sk=%llx\n", sk);
    return pconn;
#endif
  }

#if TRACE_TCP_CONNECTION
  DEBUG_PRINTK("create_tcp_connection(%llx)\n", sk);
#endif

  GET_PID_TGID;

  struct tcp_connection_t zero = {};
  zero.sk = sk;
  zero.parent_sk = NULL;
  zero.upid = _tgid;
#ifndef ENABLE_TCP_DATA_STREAM
  zero.protocol_state.candidates = TCP_PROTOCOL_MASK;
  zero.streams[0].protocol_count = TCP_PROTOCOL_COUNT;
  zero.streams[1].protocol_count = TCP_PROTOCOL_COUNT;
#endif

  struct tcp_control_key_t key = {.sk = (u64)sk};
  struct tcp_control_value_t value = {
      .streams[ST_SEND].enable = 1, .streams[ST_SEND].start = 0, .streams[ST_RECV].enable = 1, .streams[ST_RECV].start = 0};

  pconn = _tcp_connections.lookup_or_init(&sk, &zero);
  _tcp_control.insert(&key, &value);

  return pconn;
}

static struct tcp_control_value_t *get_tcp_control(struct tcp_connection_t *pconn)
{
  struct tcp_control_key_t key = {.sk = (u64)pconn->sk};
  struct tcp_control_value_t *pvalue = _tcp_control.lookup(&key);
  return pvalue;
}

// Call this when we don't want to bother processing a tcp
// connection any longer to minimize overhead
// recv/send = -1 (ignore), 0 (unchanged), 1 (don't ignore)
static void enable_tcp_connection(struct tcp_control_value_t *pctrl, int recv, int send)
{
  // DEBUG_PRINTK("enable_tcp_connection: recv=%d, send=%d\n", recv, send);
  if (send != 0) {
    pctrl->streams[ST_SEND].enable = send < 0 ? 0 : 1;
  }
  if (recv != 0) {
    pctrl->streams[ST_RECV].enable = recv < 0 ? 0 : 1;
  }
}

// Call this when we're completely done with a tcp connection and want to
// release the socket map
static void delete_tcp_connection(struct pt_regs *ctx, struct tcp_connection_t *pconn, struct sock *sk)
{
#if TRACE_TCP_CONNECTION
  DEBUG_PRINTK("delete_tcp_connection(%llx)\n", sk);
#endif

  // remove from kernel data structures
  struct tcp_control_key_t key = {.sk = (u64)pconn->sk};

  _tcp_control.delete(&key);

  int ret = _tcp_connections.delete(&sk);
  if (ret != 0) {
#if DEBUG_TCP_CONNECTION
    DEBUG_PRINTK("delete_tcp_connection: delete on non-existent socket sk=%llx\n", sk);
#endif
    bpf_log(ctx, BPF_LOG_TABLE_BAD_REMOVE, BPF_TABLE_TCP_CONNECTIONS, (u64)sk, 0);
  }
}

static void write_to_tcp_stream(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    enum STREAM_TYPE streamtype,
    const void *src_data,
    size_t src_bytes,
    void (*tcp_stream_handler)(
        struct pt_regs *ctx, struct tcp_connection_t *, struct tcp_control_value_t *, enum STREAM_TYPE, const void *, size_t))
{
  struct tcp_stream_info_t *strm = pconn->streams + (int)streamtype;
  struct tcp_control_value_t *pctrl = get_tcp_control(pconn);
  if (!pctrl) {
    bpf_log(ctx, BPF_LOG_UNREACHABLE, 0, 0, 0);
    return;
  }
  u64 start = pctrl->streams[streamtype].start;

  // If the entirety of what we are going to write is
  // before the window then we can skip this data
  if ((strm->total + src_bytes) <= start) {
    // just add the stream total without calling the callback
    strm->total += src_bytes;
    return;
  }

  u64 data_len = src_bytes;
  u8 *data = (u8 *)src_data;

  // If what we are about to write starts before the window, clip to the front
  // end of the window
  if (strm->total < start) {
    size_t trim_len = (start - strm->total);
    data_len -= trim_len;
    data += trim_len;
    strm->total = start;
  }

  // Inspect the buffer
  tcp_stream_handler(ctx, pconn, pctrl, streamtype, data, data_len);

  // Advance total
  strm->total += data_len;
}

//
// BPF Probes for intercepting the creation and destruction of TCP sockets
//

// --- tcp_init_sock ----------------------------------------------------
// Where the start of TCP socket lifetimes is for IPv4 and IPv6
int handle_kprobe__tcp_init_sock(struct pt_regs *ctx, struct sock *sk)
{
  struct tcp_connection_t *pconn;
  pconn = create_tcp_connection(ctx, sk);
  if (!pconn) {
#if DEBUG_TCP_CONNECTION
    DEBUG_PRINTK("tcp_init_sock: couldn't create tcp_connection");
#endif
    GET_PID_TGID;

    bpf_log(ctx, BPF_LOG_TABLE_BAD_INSERT, BPF_TABLE_TCP_CONNECTIONS, _tgid, (u64)sk);
    return 0;
  }
  return 0;
}

// --- security_sk_free ----------------------------------------------
// This is where final socket destruction happens for all socket types
int handle_kprobe__security_sk_free(struct pt_regs *ctx, struct sock *sk)
{
  struct tcp_connection_t *pconn = lookup_tcp_connection(sk);
  if (pconn) {
    delete_tcp_connection(ctx, pconn, sk);
  }
  return 0;
}
