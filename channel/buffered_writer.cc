// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/buffered_writer.h>
#include <channel/channel.h>
#include <util/log.h>

#include <stdexcept>

namespace channel {

BufferedWriter::BufferedWriter(Channel &channel, u32 buf_size)
    : buf_size_(buf_size), write_start_loc_(0), write_finish_loc_(0), channel_(channel)
{
  buf_ = (u8 *)malloc(buf_size * sizeof(u8));
  if (buf_ == NULL)
    throw std::runtime_error("BufferedWriter: failed to allocate memory\n");
}

BufferedWriter::~BufferedWriter()
{
  /* if we're in a consistent state, try flushing the buffer */
  if (write_start_loc_ == write_finish_loc_) {
    flush();
  }

  free(buf_);
}

Expected<u8 *, std::error_code> BufferedWriter::start_write(u32 length)
{
  /* if requesting more than buffer maximum size, bad request */
  if (length > buf_size_) {
    LOG::error(
        "BufferedWriter::start_write: requesting more than buffer maximum size"
        " (requested={}, buf_size={})",
        length,
        buf_size_);
    return {unexpected, std::make_error_code(std::errc::no_buffer_space)};
  }

  /* is there enough space in the current buffer? */
  if (buf_size_ - write_start_loc_ < length) {
    if (auto error = flush()) {
      LOG::error(
          "BufferedWriter::start_write: failed to flush the channel and there's"
          " not enough space in the current buffer to return - check for channel"
          " errors prior to this one (requested={}, buf_size={} offset={})",
          length,
          buf_size_,
          write_start_loc_);
      return {unexpected, error};
    }
  }
  assert(buf_size_ - write_start_loc_ >= length);

  /* mark the end of the write */
  write_finish_loc_ = write_start_loc_ + length;

  /* return a pointer to the start of the write */
  return &buf_[write_start_loc_];
}

void BufferedWriter::finish_write()
{
  write_start_loc_ = write_finish_loc_;
}

std::error_code BufferedWriter::flush()
{
  /* we shouldn't be in the middle of a write */
  assert(write_start_loc_ == write_finish_loc_);

  if (write_start_loc_ == 0) {
    return {};
  }

  // TODO: it should never throw
  try {
    if (is_writable()) {
      if (auto error = channel_.send(buf_, write_start_loc_)) {
        return error;
      }
    }
  } catch (...) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  write_start_loc_ = write_finish_loc_ = 0;
  return {};
}

void BufferedWriter::reset()
{
  write_start_loc_ = write_finish_loc_ = 0;
}

u32 BufferedWriter::buf_size() const
{
  return buf_size_;
}

bool BufferedWriter::is_writable() const
{
  return channel_.is_open();
}

} // namespace channel
