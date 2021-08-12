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

#include "proc_cmdline.h"

#include <util/file_ops.h>

#include <cerrno>
#include <cstdio>
#include <system_error>

// Max number of characters we are willing to read from /proc/PID/cmdline.
static constexpr size_t MAX_CMDLINE_READ_SIZE = 256;

Expected<std::string, std::error_code> read_proc_cmdline(u32 pid)
{
  char path[64] = {0};

  if (snprintf(path, sizeof(path), "/proc/%d/cmdline", pid) < 0) {
    return {unexpected, std::error_code()};
  }

  FileDescriptor fd;

  if (auto error = fd.open(path, FileDescriptor::Access::read_only, FileDescriptor::Positioning::beginning)) {
    return {unexpected, std::move(error)};
  }

  std::string buffer(MAX_CMDLINE_READ_SIZE, '\0');

  if (auto const result = fd.read_all(buffer.data(), buffer.size())) {
    buffer.resize(*result);
    return std::move(buffer);
  } else {
    return {unexpected, result.error()};
  }
}

Expected<std::string, std::error_code> try_read_proc_cmdline(u32 pid)
{
  auto r = read_proc_cmdline(pid);

  if (!r) {
    if ((r.error().category() == std::generic_category()) && ((r.error().value() == ENOENT) || (r.error().value() == ESRCH))) {
      // no such process
      return "";
    }
  }

  return r;
}
