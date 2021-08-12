//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
