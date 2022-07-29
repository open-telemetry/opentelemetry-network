/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/types.h>

namespace channel {

class Callbacks {
public:
  /**
   * virtual d'tor
   */
  virtual ~Callbacks() {}

  /**
   * Callback with ready data.
   *
   * The default implementation ignores received data.
   *
   * @returns how many bytes were consumed
   */
  virtual u32 received_data(u8 const *data, int length) { return length; }

  u32 received_data(std::basic_string_view<u8> data) { return received_data(data.data(), data.size()); }

  u32 received_data(std::string_view data) { return received_data(reinterpret_cast<u8 const *>(data.data()), data.size()); }

  /**
   * An error occurred on the channel, or -ENOLINK on EOF
   */
  virtual void on_error(int error) {}

  /**
   * The link finished closing
   */
  virtual void on_closed() {}

  /**
   * Connected
   */
  virtual void on_connect() {}
};

} /* namespace channel */
