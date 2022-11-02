/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/element_queue_cpp.h>
#include <util/element_queue_writer.h>

#include <cassert>
#include <memory>
#include <vector>

namespace reducer {

// This class is used to help with creation of queues that are used for
// message-passing between to apps.
//
class RpcQueueMatrix {
public:
  // Default number of elements in a queue shared buffer.
  static constexpr u32 default_queue_n_elems = (1 << 18);
  // Default number of bytes in a queue shared buffer.
  static constexpr u32 default_queue_buf_len = (1 << 23);

  // Constructs the object for |num_senders| senders and |num_receivers|
  // receivers.
  RpcQueueMatrix(
      size_t num_senders,
      size_t num_receivers,
      u32 queue_n_elems = default_queue_n_elems,
      u32 queue_buf_len = default_queue_buf_len)
      : num_senders_(num_senders), num_receivers_(num_receivers)
  {
    size_t const num_entries = num_receivers * num_senders;

    entries_.reserve(num_entries);

    for (size_t i = 0; i < num_entries; ++i) {
      entries_.emplace_back(make_storage(queue_n_elems, queue_buf_len));
    }
  }

  // Creates a vector of element queue objects for the specified receiver.
  //
  // This vector is normally used in construction of receiver core instance.
  //
  template <typename Reader = ElementQueue, typename... Args> std::vector<Reader> make_readers(size_t receiver, Args &&...args)
  {
    assert(receiver < num_receivers_);

    std::vector<Reader> readers;

    size_t const offset = receiver * num_senders_;
    size_t const stride = 1;            // we're using receiver-major ordering
    size_t const length = num_senders_; // a reader for each sender

    readers.reserve(length);

    for (size_t i = 0; i < length; ++i) {
      size_t const pos = offset + (i * stride);
      readers.emplace_back(entries_[pos].storage, std::forward<Args>(args)...);
    }

    return readers;
  }

  // Creates a vector of writer objects for the specified sender.
  //
  // This vector is normally used in construction of sender core instances,
  // where it is passed to the constuctor of the app's Index class.
  //
  template <typename Writer, typename... Args> std::vector<Writer> make_writers(size_t sender, Args &&...args)
  {
    assert(sender < num_senders_);

    std::vector<Writer> writers;

    size_t const offset = sender;
    size_t const stride = num_senders_;   // we're using receiver-major ordering
    size_t const length = num_receivers_; // a writer to each receiver

    writers.reserve(length);

    for (size_t i = 0; i < length; ++i) {
      size_t const pos = offset + (i * stride);
      writers.emplace_back(entries_[pos].queue_writer, std::forward<Args>(args)...);
    }

    return writers;
  }

  // Returns the number of senders this matrix is constructed for.
  size_t num_senders() const { return num_senders_; }
  // Returns the number of receivers this matrix is constructed for.
  size_t num_receivers() const { return num_receivers_; }

private:
  // Holds everything needed for messaging between one sender and one receiver.
  struct Entry {
    ElementQueueStoragePtr storage;
    ElementQueue writer_queue;
    ElementQueueWriter queue_writer;

    Entry(ElementQueueStoragePtr s) : storage(s), writer_queue(storage), queue_writer(writer_queue) {}
  };

  size_t num_senders_;
  size_t num_receivers_;

  std::vector<Entry> entries_;

  static ElementQueueStoragePtr make_storage(u32 num_elems, u32 buf_len)
  {
    return std::make_shared<MemElementQueueStorage>(num_elems, buf_len);
  }
};

} // namespace reducer
