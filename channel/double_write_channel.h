/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <channel/channel.h>

namespace channel {

class DoubleWriteChannel : public Channel {
public:
  DoubleWriteChannel(Channel &first, Channel &second);

  std::error_code send(const u8 *data, int size) override;

  void close() override;
  std::error_code flush() override;

  bool is_open() const override { return first_.is_open() && second_.is_open(); }

private:
  Channel &first_;
  Channel &second_;
};

} // namespace channel
