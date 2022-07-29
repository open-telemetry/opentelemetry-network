// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
