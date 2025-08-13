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
static __always_inline int string_starts_with(const char *s1, const size_t s1_len, const char *s2)
{

  char s2_local[16] = {};
  size_t s2_len = bpf_probe_read_kernel_str(s2_local, sizeof(s2_local), s2);
  if (s2_len > 16)
    return 0; // help the verifier

  char s1_local[16] = {};
  size_t s1_local_len = bpf_probe_read_kernel_str(s1_local, sizeof(s1_local), s1);

  if (s1_local_len < s2_len) {
    return 0;
  }

  for (int i = 0; i < 16; i++) {
    if (i >= s2_len) {
      // s2 is shorter than 16 bytes, so we are done
      return 1;
    }
    if (s2_local[i] != s1_local[i]) {
      return 0;
    }
  }

  return 1;
}

static __always_inline int char_to_number(char x)
{
  if (x < '0' || x > '9')
    return -1;
  return (int)(x - '0');
}
