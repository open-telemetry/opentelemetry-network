/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

namespace buffer {

/**
 * A writable view of a contiguous chunk of memory.
 *
 * This class is similar to `std::string_view`, but allows data to be written
 * to the buffer.
 *
 * TODO: support ring buffers where we can have 1 or 2 chunks of contiguous
 * buffers - a generalization would allow N chunks.
 */
class WritableBufferView {
public:
  WritableBufferView(char *begin, char *end) : begin_(begin), end_(end) {}

  /**
   * Returns a read-only view of this writable view.
   */
  std::string_view view() const { return {begin_, size()}; }

  // STL containers compatibility

  char const *data() const { return begin_; }
  char *data() { return begin_; }

  char const *cbegin() const { return begin_; }
  char const *begin() const { return begin_; }
  char *begin() { return begin_; }
  char const *cend() const { return end_; }
  char const *end() const { return end_; }
  char *end() { return end_; }

  std::size_t size() const { return end_ - begin_; }

private:
  char *begin_;
  char *end_;
};

/**
 * A buffer class that supports chunk read/write operations.
 *
 * Internally, the buffer is a contiguous block of memory with the following layout:
 *
 *   array:   [ consumed / available / free ]
 *            ^          ^           ^      ^
 * indices:   0          read_       write_ Size
 *
 * - consumed: data that has already been read from the buffer;
 * - available: data that has been written into the buffer, available to
 *   read operations;
 * - free: free space in the buffer, available for write operations.
 */
template <std::size_t Size> class Buffer {
public:
  /**
   * Constructs an empty buffer of size `Size`.
   */
  Buffer() : read_(buffer_), write_(buffer_) {}

  Buffer(Buffer const &) = delete;
  Buffer(Buffer &&) = delete;

  /**
   * Returns a read-only view of the `consumed` data section.
   */
  std::string_view consumed() const { return {buffer_, static_cast<std::size_t>(read_ - buffer_)}; }

  /**
   * Returns a read-only view of the `available` data section.
   */
  std::string_view available() const { return {read_, static_cast<std::size_t>(write_ - read_)}; }

  /**
   * Returns a read-only view of the `consumed` and `available` data sections
   * in a single chunk.
   */
  std::string_view view() const { return {buffer_, static_cast<std::size_t>(write_ - buffer_)}; }

  /**
   * Marks the first `count` bytes of the `available` section as `consumed`.
   * This must be called after data is read from the buffer so more data can be
   * made available to read operations.
   *
   * @assumes: `count` <= `available.size()`
   */
  Buffer &consume(std::size_t count)
  {
    assert(count <= available().size());
    read_ += count;
    return *this;
  }

  /**
   * Returns a writable view of the `free` data section.
   *
   * See `commit()` for how to make written data available to read operations.
   */
  WritableBufferView writable()
  {
    assert(buffer_ <= write_);
    assert(buffer_ + Size >= write_);
    return {write_, buffer_ + Size};
  }

  /**
   * Marks the first `count` bytes of the `free` section as `available`. This
   * must be called after data is written into the buffer so it can be made
   * available to read operations.
   *
   * @assumes: `count` <= `free()`
   */
  Buffer &commit(std::size_t count)
  {
    assert(count <= free());
    write_ += count;
    return *this;
  }

  /**
   * Drop all written data that hasn't been consumed by a read operation.
   */
  Buffer &drop()
  {
    write_ = read_;
    return *this;
  }

  /**
   * Drop the last `count` written bytes that haven't been consumed by a read operation.
   *
   * @assumes: `count` <= `available().size()`
   */
  Buffer &drop(std::size_t count)
  {
    assert(count <= available().size());
    write_ = read_;
    return *this;
  }

  /**
   * Rewinds the read cursor to the beginning of the buffer, so that data
   * that has already been consumed can be read again.
   */
  Buffer &rewind()
  {
    read_ = 0;
    return *this;
  }

  /**
   * Rewinds the read cursor back `count` bytes so that the last `count`
   * bytes read can be read again.
   *
   * @assumes: `count` <= `consumed().size()`
   */
  Buffer &rewind(std::size_t count)
  {
    assert(count <= consumed().size());
    read_ -= count;
    return *this;
  }

  /**
   * Resets the buffer to its initial state, where there's no data available
   * for read operations and the whole capacity is available for write
   * operations.
   *
   * No data in the internal buffer is overwritten.
   */
  Buffer &clear()
  {
    read_ = write_ = buffer_;
    return *this;
  }

  /**
   * Frees up space in the buffer by pruning the `consumed` section and
   * moving the `available` section to the beginning of the buffer.
   */
  Buffer &compact()
  {
    std::memmove(buffer_, read_, available().size());
    write_ -= consumed().size();
    read_ = buffer_;
    return *this;
  }

  std::size_t free() const { return buffer_ + Size - write_; }
  static constexpr std::size_t size() { return Size; }

  /**
   * Tells whether the buffer is full and has no more space for writes.
   */
  bool full() const { return write_ == buffer_ + Size; }

  /**
   * Tells whether the buffer is empty and has no data written into it.
   */
  bool empty() const { return write_ == buffer_; }

private:
  char buffer_[Size];
  char *read_;
  char *write_;
};

} // namespace buffer
