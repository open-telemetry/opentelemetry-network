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

#include <channel/stringstream_channel.h>

namespace channel {

StringStreamChannel::StringStreamChannel() {}

std::error_code StringStreamChannel::send(const u8 *data, int size)
{
  std::string_view const buffer{reinterpret_cast<char const *>(data), static_cast<std::string_view::size_type>(size)};

  ss_ << buffer;

  return {};
}

void StringStreamChannel::close() {}

std::error_code StringStreamChannel::flush()
{
  return {};
}

std::string StringStreamChannel::get()
{
  return ss_.str();
}

} // namespace channel
