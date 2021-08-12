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
    LOG::error("error while writing {} bytes into file channel: {}", size, error);
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
    LOG::error("error while flushing data for file channel: {}", error);
  }
  return error;
}

} // namespace channel
