/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/platform.h>
#include <util/fast_div.h>
#include <util/histogram.h>
#include <util/lazy_array.h>

#include <cstddef>
#include <limits>
#include <optional>

template <class Metric, std::size_t SIZE, std::size_t N_EPOCHS, class Allocator = std::allocator<Metric>> class MetricStore {
public:
  using metric_type = Metric;
  static constexpr std::size_t size = SIZE;
  static constexpr std::size_t n_epochs = N_EPOCHS;
  using index_type = typename std::conditional<(SIZE >= (1 << 16) - 2), u32, u16>::type;
  static constexpr index_type invalid = std::numeric_limits<index_type>::max();
  static constexpr index_type list_end = std::numeric_limits<index_type>::max() - 1;
  using epoch_type = u32;

  class queue_type {
  public:
    bool empty() { return head_ == list_end; }

    index_type peek()
    {
      assert(!empty());
      return head_;
    }

    void pop()
    {
      assert(!empty());

      /* save the dequeued entry */
      index_type dequeued = head_;
      /* find dequeued->next */
      index_type next = store_->arr_[dequeued].next[queue_index_];
      /* mark the dequeued entry as unqueued */
      store_->arr_[dequeued].next[queue_index_] = invalid;
      /* pop: set the head to the next entry */
      head_ = next;
    }

  private:
    friend class MetricStore;
    MetricStore *store_;
    std::size_t queue_index_;
    index_type head_;
  };

  MetricStore(const fast_div &t_to_timeslot) : t_to_timeslot_(t_to_timeslot)
  {
    static_assert((((N_EPOCHS) & ((N_EPOCHS)-1)) == 0), "N_EPOCHS must be a power of 2");

    for (u32 i = 0; i < n_epochs; i++) {
      auto &queue = queue_[i];
      queue.store_ = this;
      queue.queue_index_ = i;
      queue.head_ = list_end;
    }

    slot_duration_ = t_to_timeslot_.estimated_reciprocal();
  }

  /**
   * Looks up the statistics entry for the given index at time t, and enqueues
   *   the change if enqueue==true.
   *
   * @returns: a pair. first is true if metric was queued before the lookup.
   */
  std::pair<bool, metric_type &> lookup(u32 index, u64 t, bool enqueue)
  {
    epoch_type bin = histogram_bin(n_epochs, relative_timeslot(t));
    return lookup_relative(index, bin, enqueue);
  }

  /**
   * Looks up the statistics entry for socket @index at @bin slot ahead of
   *   the current queue. Enqueues the change if enqueue==true
   *
   * @assumes timeslot < N_EPOCHS.
   */
  std::pair<bool, metric_type &> lookup_relative(u32 index, epoch_type bin, bool enqueue)
  {
    assert(index < size);
    assert(bin < n_epochs);

    epoch_type epoch = (bin + current_queue_) & (n_epochs - 1);
    element_type &elem = arr_[index];
    bool was_queued = (elem.next[epoch] != invalid);
    if (enqueue && !was_queued) {
      elem.next[epoch] = queue_[epoch].head_;
      queue_[epoch].head_ = index;
    }
    return {was_queued, elem.m[epoch]};
  }

  /**
   * Gets the current slot's queue
   */
  queue_type &current_queue() { return queue_[current_queue_]; }

  /**
   * Advances the window of stat collection by one timeslot
   */
  void advance()
  {
    current_queue_ = (current_queue_ + 1) & (n_epochs - 1);

    if (current_timeslot_) {
      (*current_timeslot_)++;
    }
  }

  /**
   * returns the timeslot of @t relative to the current timeslot
   */
  s16 relative_timeslot(u64 t)
  {
    u16 timeslot = t / t_to_timeslot_;

    if (!current_timeslot_) {
      current_timeslot_ = timeslot;
    }

    return (s16)timeslot - *current_timeslot_;
  }

  /**
   * returns the number of time units that one slot takes up
   */
  double slot_duration() const { return slot_duration_; }

private:
  friend class queue_type;

  struct element_type {
    element_type()
    {
      for (u32 i = 0; i < n_epochs; i++)
        next[i] = invalid;
    }
    std::array<metric_type, n_epochs> m;
    std::array<index_type, n_epochs> next;
  };

  using element_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<element_type>;

  /* array of statistics */
  LazyArray<element_type, size, element_allocator_type> arr_;

  /* circular queues for changed entries */
  std::array<queue_type, n_epochs> queue_;

  /* converting t to timeslot */
  fast_div t_to_timeslot_;

  /* Timeslot assigned to the current queue, or nullopt if time-to-epoch
   * relationship is not yet established. Used to calculate the relative
   * timeslot (relative_timeslot() function).
   */
  std::optional<u16> current_timeslot_;

  /* index of the current queue */
  epoch_type current_queue_{0};

  /* slot duration, in time units */
  double slot_duration_;
};
