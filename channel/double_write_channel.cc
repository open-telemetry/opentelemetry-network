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
