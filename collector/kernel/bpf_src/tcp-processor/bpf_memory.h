/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

//
// bpf_memory.h - BPF memory related utility functions
//

#pragma once

#include "bpf_debug.h"
#include "bpf_types.h"

// s2 can not be longer than 16 bytes due to older bpf inlining limitations
inline static int string_starts_with(const char *s1, const size_t s1_len, const char *s2)
{

  char localdata[16] = {};
  const size_t s2_len = bpf_probe_read_kernel_str(localdata, sizeof(localdata), s2);

  if (s1_len < s2_len) {
    return 0;
  }

#pragma passthrough on
#pragma unroll
#pragma passthrough off
  for (int i = 0; i < s2_len; i++) {
    if (localdata[i] != s2[i]) {
      return 0;
    }
  }

  return 1;
}

inline static int char_to_number(char x)
{
  if (x < '0' || x > '9')
    return -1;
  return (int)(x - '0');
}
