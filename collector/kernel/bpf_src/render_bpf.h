/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
// This file is included both in BPF (render_bpf and tcp-processor) as well as by userland agent.

////////////////////////////////////////////////////////////////////////////
// Config

///////// render_bpf.c config

#define BPF_MAX_CPUS 1024              // Maximum number of CPUs to support
#define TABLE_SIZE__TGID_INFO MAX_PID // Task (TGID) information
#define TABLE_SIZE__SEEN_INODES                                                                                                \
  70000 // XXX: Is this even necessary? could this tracking be done in userland with non-limited tables?
#define TABLE_SIZE__TCP_OPEN_SOCKETS (256 * 1024) // Was 4096, but should be larger to accommodate high traffic systems
#define TABLE_SIZE__UDP_OPEN_SOCKETS (256 * 1024) // Was 4096, but should be larger to accommodate high traffic systems
#define TABLE_SIZE__UDP_GET_PORT_HASH 512         // Should be no more than the number of cores, in theory
#define TABLE_SIZE__SEEN_CONNTRACKS                                                                                            \
  (TABLE_SIZE__TCP_OPEN_SOCKETS +                                                                                              \
   TABLE_SIZE__UDP_OPEN_SOCKETS)         // Worst case scenario, we have a conntrack for every open socket
#define TABLE_SIZE__DEAD_GROUP_TASKS 512 // Should be no more than the number of cores, in theory
#define TABLE_SIZE__STACK_TRACES 16384   // Number of stack traces to keep in the table
#define TABLE_SIZE__NIC_INFO_TABLE 128   // Info per network interface

#define WATERMARK_STACK_TRACES                                                                                                 \
  (TABLE_SIZE__STACK_TRACES - 256) // When to clear the table (unfortunately non-atomic, but that's a lot of stack traces...)

// Maximum number of bytes in a tcp_data message block
#define DATA_CHANNEL_CHUNK_MAX 16383

// Shouldn't be necessary, be we aren't getting the lifetime of tcp
// sockets right in some edge case

// Forces socket lifetimes to not overlap due missing or out-of-order destroy events
#define TCP_LIFETIME_HACK 1
#define UDP_LIFETIME_HACK 1

// Adds sockets that were missed during our initial 'existing' walk
#define TCP_EXISTING_HACK 1
#define UDP_EXISTING_HACK 1

// When sockets are accepted, do we report the statistics on the child or parent socket?
#define TCP_STATS_ON_PARENT 1

// debug event codes
#define TCP_LIFETIME_HACK_CODE 1
#define UDP_LIFETIME_HACK_CODE 2

///////// bpf_tcp_processor.c config

#define TCP_CONNECTION_HASH_SIZE TABLE_SIZE__TCP_OPEN_SOCKETS // Same for now to ensure we can always handle all http requests

////////////////////////////////////////////////////////////////////////////
// Debugging flags

///////// render_bpf.c debugging switches

//#define TRACE_TCP_SOCKETS 1
//#define TRACE_UDP_SOCKETS 1

//#define DEBUG_TCP_SOCKET_ERRORS 1
//#define DEBUG_UDP_SOCKET_ERRORS 1
//#define DEBUG_OTHER_MAP_ERRORS 1

//#define DEBUG_ENABLE_STACKTRACE 1

///////// bpf_tcp_processor.c debugging switches

//#define TRACE_TCP_CONNECTION 1
//#define DEBUG_TCP_CONNECTION 1
//#define DEBUG_MEMORY 1
//#define TRACE_SOCKET_ACCEPT 1
//#define DEBUG_TCP_RECEIVE 1
//#define TRACE_TCP_RECEIVE 1
//#define DEBUG_TCP_SEND 1
//#define TRACE_TCP_SEND 1
//#define TRACE_HTTP_PROTOCOL 1
//#define DEBUG_TCP_DATA 1
//#define DEBUG_DATA_CHANNEL 1

////////////////////////////////////////////////////////////////////////////
// BPF error report codes

enum BPF_LOG_CODE {
  BPF_LOG_UNKNOWN = 0,
  BPF_LOG_TABLE_FULL = 1,             // arg0 = table id, arg1 = tgid, arg2 = item
  BPF_LOG_TABLE_BAD_INSERT = 2,       // arg0 = table id, arg1 = tgid, arg2 = return code
  BPF_LOG_TABLE_BAD_REMOVE = 3,       // arg0 = table id, arg1 = item being removed, arg2 = return code
  BPF_LOG_TABLE_MISSING_KEY = 4,      // arg0 = table id, arg1 = item looked up
  BPF_LOG_LIFETIME_HACK = 5,          // arg0 = table id, arg1 = item
  BPF_LOG_TABLE_DUPLICATE_INSERT = 6, // arg0 = table id, arg1 = tgid, arg2 = item
  BPF_LOG_UNEXPECTED_TYPE = 7,        // arg0 = key, arg1 = type
  BPF_LOG_DATA_TRUNCATED = 8,         // arg0 = total len, arg1 = truncated len
  BPF_LOG_INVALID_POINTER = 9,        // none
  BPF_LOG_UNSUPPORTED_IO = 10,        // arg0 = ST_SEND/ST_RECV, arg1 = sk, arg2 = type
  BPF_LOG_MISSING_ARGUMENTS = 11,     // none
  BPF_LOG_UNREACHABLE = 12,           // none
  BPF_LOG_THROTTLED = 13,             // arg0 = cpu number
  BPF_LOG_UNEXPECTED_VALUE = 14,      // arg0 = value
  BPF_LOG_INVALID_PID_TGID = 15,      // arg0 = pid_tgid
  BPF_LOG_EXISTING_HACK = 16,         // arg0 = table id, arg1 = item
  BPF_LOG_BPF_CALL_FAILED = 17,       // arg0 = ret val
};

enum BPF_TABLE_ID {
  BPF_TABLE_TGID_INFO = 0,
  BPF_TABLE_SEEN_INODES = 1,
  BPF_TABLE_TCP_OPEN_SOCKETS = 2,
  BPF_TABLE_TCP_OPENREQ_CHILD_HASH = 3,
  BPF_TABLE_UDP_OPEN_SOCKETS = 4,
  BPF_TABLE_UDP_GET_PORT_HASH = 5,
  BPF_TABLE_SEEN_CONNTRACKS = 6,
  BPF_TABLE_TCP_CONNECTIONS = 7,
  BPF_TABLE_DEAD_GROUP_TASKS = 8,
  BPF_TABLE_PID_INFO = 9,
};

// Log throttling, currently set to a max of 20 messages per second
#define BPF_LOG_THROTTLE_MAX_PER_PERIOD 20     // maximum number of log messages from bpf per period
#define BPF_LOG_THROTTLE_PERIOD_LENGTH_MS 1000 // length of the log throttle period in milliseconds

// Tail Calls
#define TAIL_CALL_ON_UDP_SEND_SKB__2 0
#define TAIL_CALL_ON_UDP_V6_SEND_SKB__2 1
#define TAIL_CALL_ON_IP_SEND_SKB__2 2
#define TAIL_CALL_ON_IP6_SEND_SKB__2 3
#define TAIL_CALL_HANDLE_RECEIVE_UDP_SKB 4
#define TAIL_CALL_HANDLE_RECEIVE_UDP_SKB__2 5
#define TAIL_CALL_CONTINUE_TCP_SENDMSG 6
#define TAIL_CALL_CONTINUE_TCP_RECVMSG 7
#define NUM_TAIL_CALLS 8

#if _PROCESSING_BPF
// Include this until we merge the tcp-processor code into render_bpf more closely
// Note, this is included by bpf and userland c++, so it -must- be an include with "" not <>
// as this distinction is how we determine in bpf which includes to inline at compile time vs process at runtime
#include "tcp-processor/tcp_processor.h"
#else
#include <collector/kernel/bpf_src/tcp-processor/tcp_processor.h>
#endif
