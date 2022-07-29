/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
