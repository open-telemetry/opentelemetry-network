/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <channel/channel.h>
#include <gmock/gmock.h>

namespace channel {

class MockChannel : public Channel {
public:
  MockChannel() = default;
  ~MockChannel() override = default;

  MOCK_METHOD2(send, std::error_code(const u8 *, int));
  MOCK_CONST_METHOD0(is_open, bool());
}; // class MockChannel
} // namespace channel
