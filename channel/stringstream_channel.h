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

#include <channel/channel.h>

#include <sstream>

namespace channel {

/**
 * A simple channel for reading and writing data to a stringstream, intended for use by unit tests.
 */
class StringStreamChannel : public Channel {
public:
  StringStreamChannel();

  std::error_code send(const u8 *data, int size) override;

  void close() override;
  std::error_code flush() override;

  bool valid() const { return true; }

  explicit operator bool() const { return valid(); }
  bool operator!() const { return !valid(); }

  bool is_open() const override { return true; }

  // For now copy the string.  Stringstreams are not required to store their data in a single contiguous array, but string_view
  // is a view into a contiguous character array.  Look into C++20 for move support into/out of stringstreams.  See
  // https://stackoverflow.com/questions/47115334/any-way-to-get-an-stdstring-view-from-an-stdostringstream-without-copying
  std::string get();

  std::stringstream ss_;
};

} // namespace channel
