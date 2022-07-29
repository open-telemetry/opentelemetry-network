/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// Protocols
#define TCP_PROTOCOL_TYPE u32
#define TCP_PROTOCOL_COUNT 1

#define TCPPROTO_UNKNOWN 0
#define TCPPROTO_HTTP 1

#define TCP_PROTOCOL_BIT(X) (1 << ((X)-1))
#define TCP_PROTOCOL_MASK ((1 << TCP_PROTOCOL_COUNT) - 1)

#define TCP_TAIL_CALL_MAX_DEPTH 8

// Stream types
enum STREAM_TYPE { ST_SEND = 0, ST_RECV = 1 };

#ifndef _PROCESSING_BPF

inline const char *stream_type_to_string(enum STREAM_TYPE stream_type)
{
  switch (stream_type) {
  case ST_SEND:
    return "SEND";
  case ST_RECV:
    return "RECV";
  }
  throw std::runtime_error("invalid STREAM_TYPE value");
}

#endif

// TCP Data sent to userland
#define TCP_DATA_SIZE 256

// TCP Control channel map
struct tcp_control_key_t {
  u64 sk; // the socket to control
};

struct tcp_control_stream_t {
  u64 enable; // 0 = disable this side of the stream, 1 = enable
  u64 start;  // start offset of stream to start watching
};

struct tcp_control_value_t {
  struct tcp_control_stream_t streams[2]; // control for send and receive streams
};

// this header could probably be removed if we are
// ever allowed to perf_submit directly from kernel memory
struct data_channel_header_t {
  u64 length;
};

// Protocol handling
enum TCP_PROTOCOL_DETECT_RESULT { TPD_FAILED = -1, TPD_UNKNOWN = 0, TPD_SUCCESS = 1 };

#include "common/client_server_type.h"
