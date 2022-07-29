// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "channel/lz4_channel.h"
#include <stdexcept>

namespace channel {

Lz4Channel::Lz4Channel(Channel &channel, u32 max_data_length)
    : compression_enabled_(false), channel_(channel), buffer_(LZ4F_compressBound(max_data_length, NULL) + LZ4F_HEADER_SIZE_MAX)
{
  if (LZ4F_cctx *lz4_context = nullptr; LZ4F_isError(LZ4F_createCompressionContext(&lz4_context, LZ4F_VERSION))) {
    throw std::runtime_error("Lz4Channel: Failed to create LZ4 context.");
  } else {
    lz4_ctx_.reset(lz4_context);
  }
}

void Lz4Channel::set_compression(bool enabled)
{
  compression_enabled_ = enabled;
}

#define _CHECK_LZ4_ERROR(code)                                                                                                 \
  if (LZ4F_isError(code)) {                                                                                                    \
    throw std::runtime_error(std::string("Lz4Channel: compression failed: ") + std::string(LZ4F_getErrorName(code)));          \
  }

std::error_code Lz4Channel::send(const u8 *data, int data_len)
{
  if (!compression_enabled_) {
    return channel_.send(data, data_len);
  }

  // Reference: https://github.com/lz4/lz4/blob/dev/lib/lz4frame.h#L248
  size_t tail = 0;
  size_t res = LZ4F_compressBegin(lz4_ctx_.get(), (void *)buffer_.data(), buffer_.size(), NULL);
  _CHECK_LZ4_ERROR(res);
  tail += res;

  res =
      LZ4F_compressUpdate(lz4_ctx_.get(), (void *)(buffer_.data() + tail), buffer_.size() - tail, (void *)data, data_len, NULL);
  _CHECK_LZ4_ERROR(res);
  tail += res;

  res = LZ4F_compressEnd(lz4_ctx_.get(), (void *)(buffer_.data() + tail), buffer_.size() - tail, NULL);
  _CHECK_LZ4_ERROR(res);
  tail += res;

  return channel_.send(buffer_.data(), tail);
}

void Lz4Channel::close()
{
  channel_.close();
}

std::error_code Lz4Channel::flush()
{
  return channel_.flush();
}

} // namespace channel
