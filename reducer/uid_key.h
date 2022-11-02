/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>

#include <string_view>

namespace reducer {

// Creates a hash to use as a 64 bit collision reducer for pod and containers
// UIDs.
u64 uid_to_u64(std::string_view uid);

// Copies at most `dest_size` bytes of `uid` into `dest`, zeroing the rest.
void uid_suffix(std::string_view uid, u8 *dest, std::size_t dest_size);

// Constructs an "UID key" from the supplied UID.
//
// UID key is a uid_suffix/uid_hash pair generated from the original UID:
// - uid_suffix is a copy of last `suffix_size` charactes, padded with zeroes if
//   `uid` length is less than `suffix_size`;
// - uid_hash is a hash of the whole UID.
//
// Returns the uid_hash, places the uid_suffix into the memory location pointed
// to by `out_suffix`.
//
u64 make_uid_key(std::string_view uid, u8 *out_suffix, std::size_t suffix_size);

template <typename T> T make_uid_key(std::string_view uid)
{
  T key;
  key.uid_hash = make_uid_key(uid, key.uid_suffix.data(), key.uid_suffix.max_size());
  return key;
}

} // namespace reducer
