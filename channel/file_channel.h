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
#include <util/file_ops.h>

namespace channel {

class FileChannel : public Channel {
public:
  FileChannel(FileDescriptor fd);

  std::error_code send(const u8 *data, int size) override;

  void close() override;
  std::error_code flush() override;

  bool valid() const { return fd_.valid(); }

  explicit operator bool() const { return valid(); }
  bool operator!() const { return !valid(); }

  bool is_open() const override { return fd_.valid(); }

private:
  FileDescriptor fd_;
};

} // namespace channel
