// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/file_channel.h>

#include <util/log.h>
#include <util/log_formatters.h>

#include <string_view>

#include <cassert>

namespace channel {

FileChannel::FileChannel(FileDescriptor fd) : fd_(std::move(fd)) {}

std::error_code FileChannel::send(const u8 *data, int size)
{
  std::string_view const buffer{reinterpret_cast<char const *>(data), static_cast<std::string_view::size_type>(size)};

  if (auto const error = fd_.write_all(buffer)) {
    const std::string error_message = error.message();
    LOG::error("error while writing {} bytes into file channel: {} ({})", size, error_message, error.value());
    return error;
  }

  return {};
}

void FileChannel::close()
{
  fd_.close();
}

std::error_code FileChannel::flush()
{
  auto const error = fd_.flush_data();
  if (error) {
    const std::string error_message = error.message();
    LOG::error("error while flushing data for file channel: {} ({})", error_message, error.value());
  }
  return error;
}

} // namespace channel
