// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "uid_key.h"

#include <util/lookup3.h>

#include <cstring>

namespace reducer {

u64 uid_to_u64(const std::string_view uid)
{
  uint32_t pc = 0;
  uint32_t pb = 0;
  lookup3_hashlittle2((void *)uid.data(), uid.length(), &pc, &pb);
  return (u64)pc + (((u64)pb) << 32);
}

void uid_suffix(std::string_view uid, u8 *dest, std::size_t dest_size)
{
  if (uid.length() >= dest_size) {
    // uid too long, take the suffix
    memcpy(dest, uid.data() + uid.length() - dest_size, dest_size);
  } else {
    // uid can fit in dest, set to the entire string, zeroing the rest
    memset(dest, 0, dest_size);
    memcpy(dest, uid.data(), uid.length());
  }
}

u64 make_uid_key(std::string_view uid, u8 *out_suffix, std::size_t suffix_size)
{
  uid_suffix(uid, out_suffix, suffix_size);
  return uid_to_u64(uid);
}

} // namespace reducer
