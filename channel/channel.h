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

#pragma once

#include <platform/types.h>

#include <string_view>
#include <system_error>

namespace channel {

/**
 * An interface that allows reading and writing data to a pipe/socket/etc
 */
class Channel {
public:
  /**
   * Virtual d'tor
   */
  virtual ~Channel() {}

  /**
   * Sends data onto the channel.
   */
  virtual std::error_code send(const u8 *data, int data_len) = 0;

  inline std::error_code send(std::basic_string_view<u8> data) { return send(data.data(), data.size()); }

  inline std::error_code send(std::string_view data) { return send(reinterpret_cast<u8 const *>(data.data()), data.size()); }

  /**
   * Flushes any internal buffers.
   */
  virtual std::error_code flush() { return {}; }

  /**
   * Closes the channel.
   */
  virtual void close() {}

  virtual bool is_open() const = 0;
};

} /* namespace channel */
