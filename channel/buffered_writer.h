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

#include <channel/ibuffered_writer.h>
#include <platform/platform.h>

namespace channel {

class Channel;

/**
 * A class that enables writing through a buffer so send() calls don't have to
 * happen for every message. When buffers are exhausted, they are sent into
 * the given Channel.
 */
class BufferedWriter : public ::IBufferedWriter {
public:
  /**
   * c'tor
   * throws if buff_ can't be malloc-ed
   * @param channel: the channel on which to send messages
   * @param buffsize: how many bytes used to batch the sent messages
   */
  BufferedWriter(Channel &channel, u32 buf_size);

  /**
   * d'tor
   */
  virtual ~BufferedWriter();

  /**
   * batches as many entries into buffer as possible before calling
   * send_buffer(). always flushes the buffer at the end.
   * @see PerfPoller::poll
   *
   * @returns: on success, where caller should write the data. nullptr when
   *   the requested length is larger than buf_size_, or if call to
   *   flush() fails.
   */
  Expected<u8 *, std::error_code> start_write(u32 length) override;

  /**
   * Finishes the current write
   */
  void finish_write() override;

  /**
   * Flushes the buffer to the channel if it's non-empty.
   *
   * @return -EINVAL if a send() fails, 0 if successful
   */
  std::error_code flush() override;

  /**
   * Abandons the current buffered data
   */
  void reset();

  /**
   * Returns the buffer size
   */
  u32 buf_size() const override;

  bool is_writable() const override;

private:
  u8 *buf_;
  const u32 buf_size_;

  /* where next or active write starts */
  u32 write_start_loc_;
  /* where active write will finish */
  u32 write_finish_loc_;

  Channel &channel_;
};

} // namespace channel
