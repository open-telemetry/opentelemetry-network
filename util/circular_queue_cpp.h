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

#include <platform/platform.h>
#include <stdexcept>

template <typename T> class CircularQueue {
public:
  using value_type = T;

  /**
   * Creates a new circular queue with capacity @num_elems.
   * @important: @num_elems must be a power of two
   * @param elems: a buffer large enough to hold num_elems elements of type T
   * @throws runtime_error if num_elems is not a power of two
   */
  CircularQueue(value_type *elems, uint32_t num_elems) : head_(0), tail_(0), mask_(num_elems - 1), elems_(elems)
  {
    if ((num_elems & (num_elems - 1)) != 0)
      throw std::runtime_error("num_elems must be a power of two");
  }

  /**
   * Enqueues @x at the end of @q.
   * @assumes there is enough space in q. (user can check with full())
   */
  void enqueue(value_type x)
  {
    uint32_t tail = tail_;
    elems_[tail & mask_] = x;
    barrier();
    ACCESS_ONCE(tail_) = tail + 1;
  }

  /**
   * Dequeues an element from @q.
   * @assumes q is non-empty (user can check with empty())
   */
  value_type dequeue()
  {
    uint32_t head = head_;
    value_type retval;

    retval = elems_[head & mask_];
    barrier();
    ACCESS_ONCE(head_) = head + 1;
    return retval;
  }

  /**
   * Returns the head element from @q without dequeuing it.
   * @assumes q is non-empty (user can check with empty())
   */
  value_type peek() { return elems_[head_ & mask_]; }

  /**
   * Returns true if @q is empty, false otherwise.
   */
  bool empty() { return (tail_ == head_); }

  /**
   * Returns the current occupancy of the queue.
   * (to be used by the consumer)
   */
  uint32_t occupancy() { return tail_ - head_; }

  /**
   * Returns the number of free slots in the queue
   * (to be used by the producer)
   */
  uint32_t space() { return mask_ + 1 - (tail_ - head_); }

  /**
   * Returns true if @q is full, false otherwise.
   */
  int full() { return (occupancy() >= mask_ + 1); }

  /**
   * Discards 'n_elem' elements from the head of the queue
   */
  void discard(uint32_t n_elems)
  {
    assert(n_elems <= occupancy());

    ACCESS_ONCE(head_) = head_ + n_elems;
  }

private:
  /* index of the first item in the queue */
  uint32_t head_;
  /* index following the last item in the queue */
  uint32_t tail_;
  /* the number of allowed elements (which is a power of two) minus 1 */
  const uint32_t mask_;
  /* pointer to element buffer */
  value_type *elems_;
};
