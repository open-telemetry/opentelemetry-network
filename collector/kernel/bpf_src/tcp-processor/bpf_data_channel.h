/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// TCP Data sent to userland
BPF_PERF_OUTPUT(data_channel);

// Slightly more compact fore than COPY_BIT(2) + COPY_BIT(1)
#define COPY_LAST_BITS                                                                                                         \
  const int last_bits = len & 3;                                                                                               \
  msg.hdr.length = last_bits;                                                                                                  \
  switch (last_bits) {                                                                                                         \
  default:                                                                                                                     \
    break;                                                                                                                     \
  case 1:                                                                                                                      \
    bpf_probe_read(&msg.data, 1, in);                                                                                          \
    data_channel.perf_submit(ctx, &msg, sizeof(struct data_channel_header_t) + 1);                                             \
    break;                                                                                                                     \
  case 2:                                                                                                                      \
    bpf_probe_read(&msg.data, 2, in);                                                                                          \
    data_channel.perf_submit(ctx, &msg, sizeof(struct data_channel_header_t) + 2);                                             \
    break;                                                                                                                     \
  case 3:                                                                                                                      \
    bpf_probe_read(&msg.data, 3, in);                                                                                          \
    data_channel.perf_submit(ctx, &msg, sizeof(struct data_channel_header_t) + 3);                                             \
    break;                                                                                                                     \
  }

#define COPY_BIT(B)                                                                                                            \
  if (len & B) {                                                                                                               \
    msg.hdr.length = B;                                                                                                        \
    bpf_probe_read(&msg.data, B, in);                                                                                          \
    data_channel.perf_submit(ctx, &msg, sizeof(struct data_channel_header_t) + B);                                             \
    in += B;                                                                                                                   \
  }

#define COPY_CHUNK_256                                                                                                         \
  bpf_probe_read(&msg.data, 256, in);                                                                                          \
  data_channel.perf_submit(ctx, &msg, sizeof(struct data_channel_header_t) + 256);                                             \
  in += 256;

#define COPY_BIT_256                                                                                                           \
  if (len & 256) {                                                                                                             \
    COPY_CHUNK_256;                                                                                                            \
  }

#define COPY_BIT_512                                                                                                           \
  if (len & 512) {                                                                                                             \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
  }

#define COPY_BIT_1024                                                                                                          \
  if (len & 1024) {                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
  }

#define COPY_BIT_2048                                                                                                          \
  if (len & 2048) {                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
  }

#define COPY_BIT_4096                                                                                                          \
  if (len & 4096) {                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
  }

#define COPY_BIT_8192                                                                                                          \
  if (len & 8192) {                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
    COPY_CHUNK_256;                                                                                                            \
  }

// Submit contents of TCP data stream to the perf ring to userland
static void
data_channel_submit(struct pt_regs *ctx, struct tcp_connection_t *pconn, const void *data, size_t data_len, size_t *actual_len)
{
  // Clip the length to the most we can copy
  unsigned int len = data_len;
  if (len > DATA_CHANNEL_CHUNK_MAX) {
#if DEBUG_DATA_CHANNEL
    bpf_log(ctx, BPF_LOG_DATA_TRUNCATED, (u64)len, (u64)DATA_CHANNEL_CHUNK_MAX, 0);
#endif
    len = DATA_CHANNEL_CHUNK_MAX;
  }
  *actual_len = len;

#if DEBUG_DATA_CHANNEL
  bpf_trace_printk("tcp_events_submit_tcp_data: sk=%llx, len=%u, data_len=%u\n", pconn->sk, len, data_len);
#endif

  // Copy the data to the data stream
  const u8 *in = (const u8 *)data;
  struct {
    struct data_channel_header_t hdr;
    char data[256];
  } msg = {.hdr.length = 256, .data = {}};

  // Copy 256 byte chunks, because we can't have a very large on-stack buffer
  // and early ebpf can't bpf_probe_read to anywhere but the stack, and also
  // early ebpf can not perf_submit from anything other than the stack
#if DATA_CHANNEL_CHUNK_MAX >= 8192
  COPY_BIT_8192;
#endif
#if DATA_CHANNEL_CHUNK_MAX >= 4096
  COPY_BIT_4096;
#endif
#if DATA_CHANNEL_CHUNK_MAX >= 2048
  COPY_BIT_2048;
#endif
#if DATA_CHANNEL_CHUNK_MAX >= 1024
  COPY_BIT_1024;
#endif
#if DATA_CHANNEL_CHUNK_MAX >= 512
  COPY_BIT_512;
#endif
#if DATA_CHANNEL_CHUNK_MAX >= 256
  COPY_BIT_256;
#endif

#if DATA_CHANNEL_CHUNK_MAX < 256
#error "Invalid data channel chunk size"
#endif

  // Copy  < 256 byte variable length chunks
  COPY_BIT(128);
  COPY_BIT(64);
  COPY_BIT(32);
  COPY_BIT(16);
  COPY_BIT(8);
  COPY_BIT(4);

  // Copy last bits using a slightly more compact form so this fits in the 4k instruction limit
  COPY_LAST_BITS;

  // COPY_BIT(2);
  // COPY_BIT(1);
}
