/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

inline bool prefix_check(const void *haystack, const size_t haystack_len, const char *needle)
{
  size_t needle_len = strlen(needle);
  return (haystack_len >= needle_len && memcmp(haystack, needle, needle_len) == 0);
}

inline int char_to_number(char x)
{
  if (x < '0' || x > '9')
    return -1;
  return (int)(x - '0');
}

#define U32IDX(X, N) (absl::little_endian::FromHost32(((u32 *)(X))[(N)]))
#define _U32CH(X, N) (((u32)((X)[(N)])) << (8 * (N)))
#define U32CC(X) (_U32CH(X, 0) | _U32CH(X, 1) | _U32CH(X, 2) | _U32CH(X, 3))
