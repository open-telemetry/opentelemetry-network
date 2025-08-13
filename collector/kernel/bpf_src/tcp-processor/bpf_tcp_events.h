/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef STANDALONE_TCP_PROCESSOR

// Only used by standalone debugging (tcp-processor.py)
///////////////////////////////////////////////////////

// TCP Event Types
#define TCP_EVENT_TYPE u32
#define TCP_EVENT_TYPE_HTTP_RESPONSE ((TCP_EVENT_TYPE)0)
#define TCP_EVENT_TYPE_TCP_DATA ((TCP_EVENT_TYPE)1)

// Format of tcp_events perf buffer messages
struct tcp_events_t {
  TCP_EVENT_TYPE type; // TCP_EVENT_TYPE
  TGID pid;            // Userland process id related to this event
  TIMESTAMP ts;        // Timestamp
  u64 sk;              // Socket pointer related to this event
  union {
    struct {        // HTTP Reponse events data
      u16 code;     // Response status code
      u8 dir;       // client/server direction (0=client)
      u8 __pad0[5]; // 64 bit align
      u64 latency;  // request/response latency in ns
    } http_response;
    struct {         // TCP Data events data
      u32 length;    // Length of chunk
      u8 streamtype; // STREAM_TYPE
      u8 is_server;  // CLIENT_SERVER_TYPE
      u16 __pad0;    // 64 bit align
      u64 offset;    // Position in the stream
    } tcp_data;
    struct {
      u64 __pad0; // Padding to ensure length is multiple of 8 bytes
      u64 __pad1; // Padding to ensure length is multiple of 8 bytes
    } __align;
  };
};

// The perf event array map for libbpf
struct {
  __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
  __uint(key_size, sizeof(__u32));
  __uint(value_size, sizeof(__u32));
} tcp_events SEC(".maps");

#endif

// Utility functions

static __always_inline void
tcp_events_submit_http_response(struct pt_regs *ctx, struct sock *sk, u16 code, u64 latency, enum CLIENT_SERVER_TYPE dir)
{
  GET_PID_TGID
  u64 now = get_timestamp();

#ifdef STANDALONE_TCP_PROCESSOR

  struct tcp_events_t event = {
      .type = TCP_EVENT_TYPE_HTTP_RESPONSE, .pid = _tgid, .ts = now, .sk = (u64)sk, .__align.__pad0 = 0, .__align.__pad1 = 0};
  event.http_response.code = code;
  event.http_response.dir = (u8)dir;
  event.http_response.latency = latency;

  bpf_perf_event_output(ctx, &tcp_events, BPF_F_CURRENT_CPU, &event, sizeof(event));

#else

  perf_submit_agent_internal__http_response(ctx, now, (u64)sk, _tgid, code, latency, dir);
#endif
}

static __always_inline void tcp_events_submit_tcp_data(
    struct pt_regs *ctx,
    struct tcp_connection_t *pconn,
    enum STREAM_TYPE streamtype,
    enum CLIENT_SERVER_TYPE is_server,
    const void *data,
    size_t data_len)
{

  // submit data to data channel
  size_t actual_len;
  data_channel_submit(ctx, pconn, data, data_len, &actual_len);

  // now send render message announcing its existence
  u64 now = get_timestamp();

#if DEBUG_TCP_DATA
  bpf_trace_printk("tcp_events_submit_tcp_data: tstamp=%u sk=%llx streamtype=%d\n", now, pconn->sk, streamtype);
  bpf_trace_printk("                  is_server=%d, data_len=%u\n", is_server, data_len);
#endif

  GET_PID_TGID;

#ifdef STANDALONE_TCP_PROCESSOR

  struct tcp_events_t event = {
      .type = TCP_EVENT_TYPE_TCP_DATA, .pid = _tgid, .ts = now, .sk = (u64)pconn->sk, .__align.__pad0 = 0, .__align.__pad1 = 0};
  event.tcp_data.length = actual_len;
  event.tcp_data.streamtype = (u8)streamtype;
  event.tcp_data.is_server = (u8)is_server;
  event.tcp_data.offset = pconn->streams[streamtype].total;

  bpf_perf_event_output(ctx, &tcp_events, BPF_F_CURRENT_CPU, &event, sizeof(event));

#else

  // Submit the control perf ring tcp data message
  perf_submit_agent_internal__tcp_data(
      ctx, now, (u64)pconn->sk, _tgid, actual_len, pconn->streams[streamtype].total, streamtype, is_server);

#endif
}
