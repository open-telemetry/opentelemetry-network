/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/platform.h>

#include <lz4frame.h>

#include <cstdint>
#include <tuple>

class Lz4Decompressor {
public:
  explicit Lz4Decompressor(size_t buf_capacity);
  ~Lz4Decompressor();

  const u8 *output_buf() const;
  size_t output_buf_size() const;

  // Decompresses |data| of size |data_len|.
  // The # of bytes that have been decompressed is stored in |consumed_len|.
  //
  // Returns LZ4 error, if an error happened, 0 otherwise.
  size_t process(const u8 *data, size_t data_len, size_t *consumed_len);

  // Discards |len| bytes of data in output_buf.
  void discard(size_t len);

private:
  const size_t output_buf_capacity_;
  size_t tail_loc_;

  LZ4F_dctx *ctx_;
  u8 *output_buf_;
};
