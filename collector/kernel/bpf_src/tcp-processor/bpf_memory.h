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

/*
 * Safe prefix compare for potentially untrusted pointers (kernel or user).
 * Copies up to 16 bytes from s1 via helper before comparing to s2 literal.
 * Intended for 5.4-era verifier quirks and mixed user/kernel buffers.
 */
static __always_inline int string_starts_with(const char *s1, const size_t s1_len, const char *s2, const size_t s2_len)
{
  if (s2_len == 0) {
    return 1;
  }
  if (s2_len > 16) {
    return 0;
  }
  if (s1_len < s2_len) {
    return 0;
  }

  char local[16] = {};
  if (bpf_probe_read(local, (u32)s2_len, s1) != 0) {
    return 0;
  }

  int n = (int)s2_len; // s2_len <= 16 guaranteed
  for (int i = 0; i < n; i++) {
    if (local[i] != s2[i]) {
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
