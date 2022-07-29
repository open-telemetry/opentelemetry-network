// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/double_write_channel.h>

namespace channel {

DoubleWriteChannel::DoubleWriteChannel(Channel &first, Channel &second) : first_(first), second_(second) {}

std::error_code DoubleWriteChannel::send(const u8 *data, int size)
{
  if (auto error = first_.send(data, size)) {
    return error;
  }

  if (second_.is_open()) {
    if (auto error = second_.send(data, size)) {
      return error;
    }
  }

  return {};
}

void DoubleWriteChannel::close()
{
  first_.close();
  second_.close();
}

std::error_code DoubleWriteChannel::flush()
{
  if (auto error = first_.flush()) {
    return error;
  }

  if (second_.is_open()) {
    if (auto error = second_.flush()) {
      return error;
    }
  }

  return {};
}

} // namespace channel
