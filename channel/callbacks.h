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

namespace channel {

class Callbacks {
public:
  /**
   * virtual d'tor
   */
  virtual ~Callbacks() {}

  /**
   * Callback with ready data.
   *
   * The default implementation ignores received data.
   *
   * @returns how many bytes were consumed
   */
  virtual u32 received_data(u8 const *data, int length) { return length; }

  u32 received_data(std::basic_string_view<u8> data) { return received_data(data.data(), data.size()); }

  u32 received_data(std::string_view data) { return received_data(reinterpret_cast<u8 const *>(data.data()), data.size()); }

  /**
   * An error occurred on the channel, or -ENOLINK on EOF
   */
  virtual void on_error(int error) {}

  /**
   * The link finished closing
   */
  virtual void on_closed() {}

  /**
   * Connected
   */
  virtual void on_connect() {}
};

} /* namespace channel */
