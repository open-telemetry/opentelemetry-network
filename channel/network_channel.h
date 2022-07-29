/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <channel/callbacks.h>
#include <channel/channel.h>
#include <platform/platform.h>

namespace channel {

/**
 * An interface that allows reading and writing data to a pipe/socket/etc
 */
class NetworkChannel : public Channel {
public:
  /**
   * Connects to an endpoint and starts negotiating
   * @param callbacks: the callbacks to use during this connection
   */
  virtual void connect(Callbacks &callbacks) = 0;

  /**
   * Returns the address (in binary format) that this channel is connected to,
   * if available. `nullptr` otherwise.
   */
  virtual in_addr_t const *connected_address() const = 0;
};

} /* namespace channel */
