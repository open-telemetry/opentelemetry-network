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
