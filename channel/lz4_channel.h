/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <channel/channel.h>
#include <platform/types.h>
#include <util/raii.h>

#include <lz4frame.h>

#include <vector>

namespace channel {

// Lz4Channel serves as an adapter between upstream data source and downstream
// channel.
//
// When the compression is disabled, the Lz4Channel will pass any incoming
// data packets to downstream channel directly.
//
// When the compression is enabled, the Lz4Channel will compress the incoming
// data packets before relay then.
class Lz4Channel : public Channel {
public:
  // |channel|: the downstream channel which will actually send out the data.
  // |max_data_length|: max number of bytes of any incoming data packet sent
  //                    via send() function. Note that it's caller's
  //                    responsibility to honor this constraint.
  Lz4Channel(Channel &channel, u32 max_data_length);

  std::error_code send(const u8 *data, int data_len) override;

  void set_compression(bool enabled);

  void close() override;
  std::error_code flush() override;

  bool is_open() const override { return channel_.is_open(); }

private:
  bool compression_enabled_;

  Channel &channel_;
  std::vector<u8> buffer_;

  pod_unique_ptr<LZ4F_cctx, LZ4F_errorCode_t, LZ4F_freeCompressionContext> lz4_ctx_;
};

} // namespace channel
