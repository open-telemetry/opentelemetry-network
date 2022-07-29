/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <platform/platform.h>
#include <string.h>
#include <util/circular_queue_cpp.h>
#include <util/fixed_hash.h>
#include <util/histogram.h>

/**
 * Struct used to keep connection statistics to be sent periodically
 */
struct tcp_statistics {
  u64 diff_bytes_acked = 0;
  u32 diff_delivered = 0;
  u32 diff_retrans = 0;
  u32 max_srtt = 0;
  u32 diff_rcv_holes = 0;
  u64 diff_bytes_received = 0;
  u32 diff_rcv_delivered = 0;
  u32 max_rcv_rtt = 0;

  /* is this statistic valid? since we cannot dequeue ourselves, this enables
   * code to invalidate this entry so it will be ignored
   */
  bool valid = false;
};

struct tcp_socket_entry {
  /* last state observed */
  u64 bytes_acked = 0;
  u32 packets_delivered = 0;
  u32 packets_retrans = 0;
  u64 bytes_received = 0;
  u32 rcv_holes = 0;
  u32 rcv_delivered = 0;
};

struct udp_statistics {
  bool valid = false;
  u32 packets = 0;
  u64 bytes = 0;
  u32 drops = 0;
};

struct udp_remote_endpoint {
  std::array<u32, 4> addr = {0, 0, 0, 0};
  u16 port = 0;

  /** Address Family if address changed since last report, 0 otherwise */
  u8 changed_af = 0;
};

struct udp_socket_entry {
  /* local address info */
  std::array<u32, 4> laddr = {0, 0, 0, 0};
  u16 lport = 0;

  bool reported = false; /* did we send to backend */
  u32 pid = 0;
  u64 sk = 0;
  struct udp_remote_endpoint addrs[2] = {{}}; /* 0 for TX, 1 for RX */
};
