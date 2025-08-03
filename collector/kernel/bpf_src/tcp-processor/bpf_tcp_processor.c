/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef STANDALONE_TCP_PROCESSOR

#define KBUILD_MODNAME "bpf_http"
#include "../render_bpf.h"
#include <vmlinux.h>

// Include this from render_bpf.c so we can use tail calls in tcp processor test code
struct {
  __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
  __uint(max_entries, NUM_TAIL_CALLS);
  __uint(key_size, sizeof(__u32));
  __uint(value_size, sizeof(__u32));
} tail_calls SEC(".maps");

#endif

// Verify kernel is >= 4.4
// #if LINUX_KERNEL_VERSION < KERNEL_VERSION(4, 4, 0)
// #error "Kernel version needs to be 4.4 or newer"
// #endif

////////////////////////////////////////////////////////////////////////////
// Utilities

#include "bpf_debug.h"
#include "bpf_memory.h"
#include "bpf_types.h"

////////////////////////////////////////////////////////////////////////////
// Subcomponents

#include "tcp_processor.h"

// TCP socket handling
#include "bpf_tcp_socket.h"

// Connection accept handling
#include "bpf_inet_csk_accept.h"

// TCP send/receive handling

struct tcp_handler_args {
  struct pt_regs *ctx;
  struct tcp_connection_t *pconn;
  struct tcp_control_value_t *pctrl;
  enum STREAM_TYPE streamtype;
  const void *data;
  size_t data_len;
};

static void tcp_client_handler_wrapper(struct tcp_handler_args *args);
static void tcp_server_handler_wrapper(struct tcp_handler_args *args);

#define TCP_CLIENT_HANDLER(CTX, CONN, CTRL, ST, DATA, LEN)                                                                     \
  do {                                                                                                                         \
    struct tcp_handler_args __args = {                                                                                         \
        .ctx = (CTX), .pconn = (CONN), .pctrl = (CTRL), .streamtype = (ST), .data = (DATA), .data_len = (LEN)};                \
    tcp_client_handler_wrapper(&__args);                                                                                       \
  } while (0)

#define TCP_SERVER_HANDLER(CTX, CONN, CTRL, ST, DATA, LEN)                                                                     \
  do {                                                                                                                         \
    struct tcp_handler_args __args = {                                                                                         \
        .ctx = (CTX), .pconn = (CONN), .pctrl = (CTRL), .streamtype = (ST), .data = (DATA), .data_len = (LEN)};                \
    tcp_server_handler_wrapper(&__args);                                                                                       \
  } while (0)

static inline void tcp_client_handler_impl(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    struct tcp_control_value_t *pctrl,
    enum STREAM_TYPE streamtype,
    const void *data,
    size_t data_len);
static inline void tcp_server_handler_impl(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    struct tcp_control_value_t *pctrl,
    enum STREAM_TYPE streamtype,
    const void *data,
    size_t data_len);

#include "bpf_tcp_send_recv.h"

// Data channel output
#include "bpf_data_channel.h"

// TCP events output
#include "bpf_tcp_events.h"

#ifndef ENABLE_TCP_DATA_STREAM
// HTTP protocol handling
#include "bpf_http_protocol.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void tcp_client_handler_impl(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    struct tcp_control_value_t *pctrl,
    enum STREAM_TYPE streamtype,
    const void *data,
    size_t data_len)
{

#pragma passthrough on
#if ENABLE_TCP_DATA_STREAM
#pragma passthrough off

  tcp_events_submit_tcp_data(ctx, pconn, streamtype, 0, data, data_len);

#pragma passthrough on
#else
#pragma passthrough off

  struct tcp_protocol_state_t *state = &(pconn->protocol_state);
  struct tcp_stream_info_t *strm = pconn->streams + (int)streamtype;

  // Perform protocol detection
  if (state->protocol == TCPPROTO_UNKNOWN) {
    // Detect HTTP
    if (state->candidates & TCP_PROTOCOL_BIT(TCPPROTO_HTTP)) {

      enum TCP_PROTOCOL_DETECT_RESULT res = http_detect(ctx, pconn, pctrl, streamtype, data, data_len);
      if (res == TPD_FAILED) {
        state->candidates &= ~TCP_PROTOCOL_BIT(TCPPROTO_HTTP);
      } else if (res == TPD_SUCCESS) {
        state->protocol = TCPPROTO_HTTP;
      }
    }

    // If no candidates left, no need to process anything else
    if (state->candidates == 0) {
      enable_tcp_connection(pctrl, -1, -1);
      return;
    }
  }

  switch (state->protocol) {
  case TCPPROTO_UNKNOWN:
    // If we don't know the protocol, don't process this at all
    return;
  case TCPPROTO_HTTP:
    http_process_request(ctx, pconn, pctrl, streamtype, data, data_len);
    break;
  default:
    bpf_log(ctx, BPF_LOG_UNREACHABLE, 0, 0, 0);
    break;
  }

#pragma passthrough on
#endif
#pragma passthrough off
}

static inline void tcp_server_handler_impl(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    struct tcp_control_value_t *pctrl,
    enum STREAM_TYPE streamtype,
    const void *data,
    size_t data_len)
{
#pragma passthrough on
#if ENABLE_TCP_DATA_STREAM
#pragma passthrough off

  tcp_events_submit_tcp_data(ctx, pconn, streamtype, 1, data, data_len);

#pragma passthrough on
#else
#pragma passthrough off

  struct tcp_protocol_state_t *state = &(pconn->protocol_state);

  // Parse the protocol
  switch (state->protocol) {
  case TCPPROTO_UNKNOWN:
    // If we don't know the protocol, don't process this at all
    return;
  case TCPPROTO_HTTP:
    http_process_response(ctx, pconn, pctrl, streamtype, data, data_len);
    break;
  default:
    bpf_log(ctx, BPF_LOG_UNREACHABLE, 0, 0, 0);
    break;
  }

#pragma passthrough on
#endif
#pragma passthrough off
}

static void tcp_client_handler_wrapper(struct tcp_handler_args *args)
{
  tcp_client_handler_impl(args->ctx, args->pconn, args->pctrl, args->streamtype, args->data, args->data_len);
}

static void tcp_server_handler_wrapper(struct tcp_handler_args *args)
{
  tcp_server_handler_impl(args->ctx, args->pconn, args->pctrl, args->streamtype, args->data, args->data_len);
}
