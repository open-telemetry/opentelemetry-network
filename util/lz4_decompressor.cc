// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "lz4_decompressor.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

Lz4Decompressor::Lz4Decompressor(size_t capacity) : output_buf_capacity_(capacity), tail_loc_(0)
{
  output_buf_ = (u8 *)malloc(capacity * sizeof(u8));
  if (output_buf_ == NULL) {
    throw std::runtime_error("Lz4Decompressor: failed to allocate memory.");
  }

  LZ4F_errorCode_t r = LZ4F_createDecompressionContext(&ctx_, LZ4F_VERSION);
  if (LZ4F_isError(r)) {
    throw std::runtime_error("Lz4Decompressor: failed to create a CTX object.");
  }
}

Lz4Decompressor::~Lz4Decompressor()
{
  free(output_buf_);
  LZ4F_freeDecompressionContext(ctx_);
}

size_t Lz4Decompressor::process(const u8 *data, size_t data_len, size_t *consumed_len)
{
  size_t res = 0;
  size_t src_size = 0;
  *consumed_len = 0;

  do {
    src_size = data_len;
    size_t dst_size = output_buf_capacity_ - tail_loc_;

    res = LZ4F_decompress(ctx_, (void *)(output_buf_ + tail_loc_), &dst_size, (void *)data, &src_size, NULL);

    *consumed_len += src_size;
    tail_loc_ += dst_size;
    data += src_size;
    data_len -= src_size;

    // continue to decompress while LZ4 is making progress
  } while ((!LZ4F_isError(res)) && (src_size > 0));

  return LZ4F_isError(res) ? res : 0;
}

const u8 *Lz4Decompressor::output_buf() const
{
  return output_buf_;
}

size_t Lz4Decompressor::output_buf_size() const
{
  return tail_loc_;
}

void Lz4Decompressor::discard(size_t len)
{
  assert(tail_loc_ >= len);

  if (tail_loc_ == len) {
    tail_loc_ = 0;
    return;
  }

  memmove((void *)output_buf_, (const void *)(output_buf_ + len), tail_loc_ - len);
  tail_loc_ -= len;
}
