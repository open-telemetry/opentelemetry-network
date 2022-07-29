/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <bitset>
#include <queue>
#include <utility>
#include <vector>

#include <platform/platform.h>

#include <collector/kernel/bpf_src/render_bpf.h>
#include <util/perf_ring_cpp.h>

/**
 * Container for multiple CPU readers.
 *
 * Accessing values is done through PerfReader. PerfReader starts and finishes
 *   read batches, and maintains the entries_ heap.
 *
 * If a reader is non-empty, PerfReader will populate an entry in entries_:
 *   * if the next entry is PERF_LOST_RECORD, the timestamp will be ~0ull
 *   * if the next entry is PERF_RECORD_SAMPLE, the timestamp will be the one
 *     encoded in the sample. This code assume that a sample starts with
 *      [ perf_event_header + u32 size + u32 unpadded_size + u64 timestamp ]
 */
class PerfContainer {
public:
  /**
   * C'tor
   * @param cpus: the CPUs whose rings we should map
   * @param n_pages: the number of data pages for in each ring.
   */
  PerfContainer();

  /* disallow copy and assignment */
  PerfContainer(const PerfContainer &) = delete;
  void operator=(const PerfContainer &) = delete;

  /**
   * Add the control channel ring to the collection
   *
   * Throws if trying to add more than 64 elements
   */
  void add_ring(PerfRing &pr);

  /**
   * Add the data channel ring to the collection
   *
   * Throws if trying to add more than 64 elements
   */
  void add_data_ring(PerfRing &pr);

  /**
   * Set a callback to execute when events show up in the
   * control channel perf ring
   * Used if you are not polling for high-speed access
   * Must be called only after all perf rings are added.
   */
  typedef void CALLBACK(void *ctx);
  void set_callback(uv_loop_t &loop, void *ctx, CALLBACK cb);

  /**
   *  Debugging routine to inspect the contents of the perf container
   */
  std::string inspect(void);

  // returns the number of perf rings in this container
  std::size_t size() const { return readers_.size(); }

  // returns a reference to the i-th perf ring
  PerfRing const &operator[](std::size_t i) const { return readers_[i]; }
  PerfRing const &data_ring(std::size_t i) const { return data_readers_[i]; }
  PerfRing &data_ring(std::size_t i) { return data_readers_[i]; }

private:
  friend class PerfReader;

  /* Readers for each live CPU */
  std::vector<PerfRing> readers_;
  std::vector<PerfRing> data_readers_;

  /* (timestamp, reader_index) pairs */
  struct PerfEntry {
    u64 timestamp;
    size_t reader_index;
    inline bool operator<(const PerfEntry &other) const
    {
      // Reverse the sense of the PerfEntry sort order
      // this way, our entries_ heap puts the earliest timestamps first,
      // making this a -min heap- instead of a -max heap-
      return timestamp > other.timestamp;
    }
  };

  PerfEntry entries_[BPF_MAX_CPUS];
  size_t n_entries_;

  /* bitmask: which readers from readers_ are already in entries_ */
  std::bitset<BPF_MAX_CPUS> readers_in_entries_;
};

/**
 * Read sorted values from multiple queues.
 */
class PerfReader {
public:
  /**
   * C'tor
   *
   * Starts a sorted read operation.
   */
  PerfReader(PerfContainer &container, u64 max_timestamp);

  /* D'tor */
  ~PerfReader();

  /**
   * Returns true if there are no more events to read safely while keeping
   *   sort.
   */
  bool empty();

  /**
   * Returns the number of bytes left to read of of the perf ring, and
   * optionally the total size of the ring
   */
  inline u32 bytes_remaining(u32 *total_bytes);

  /**
   * Returns the type of the next value
   *
   * Assumes reader is not empty (i.e., !empty())
   */
  inline u32 peek_type() const { return top().peek_type(); }

  /**
   * Returns the total size of the next perf event
   *
   * Assumes reader is not empty (i.e., !empty())
   */
  inline u32 peek_size() { return top().peek_size(); }

  /**
   * Returns the length of the payload of the next value
   *
   * Assumes reader is not empty (i.e., !empty()) and type==PERF_RECORD_SAMPLE
   */
  inline u16 peek_unpadded_length() { return top().peek_aligned_u32(sizeof(u32)); }

  /**
   * Returns the length of the payload of the next value
   *
   * Assumes reader is not empty (i.e., !empty()) and type==PERF_RECORD_SAMPLE
   */
  inline u16 peek_rpc_id() { return top().peek_aligned_u16(2 * sizeof(u64)); }

  /**
   * Returns the number of lost samples, if type is PERF_RECORD_LOST
   */
  inline u64 peek_n_lost() { return top().peek_aligned_u64(sizeof(u64)); }

  /**
   * Returns a view into the sample's contents, without the perf event header.
   *
   * The messages are stored in a ring buffer, so if they're located at the end
   * of the ring and wrap-around to the beginning, then the view will be spread
   * over two chunks, in the order specified by `first` and `second` members of
   * the returned pair. Otherwise, the view will have exactly one chunk
   * represented by `first` in the returned pair and `second` will be empty.
   *
   * @assumes: `peek_type()` == `PERF_RECORD_SAMPLE`.
   */
  inline std::pair<std::string_view, std::string_view> peek_message() const
  {
    auto const &ring = top();
    assert(ring.peek_type() == PERF_RECORD_SAMPLE);
    return ring.peek();
  }

  /**
   * Returns which cpu index we're reading from next
   */

  inline size_t peek_index() const
  {
    size_t idx = container_.entries_[0].reader_index;
    return idx;
  }

  /**
   * Copies the payload of a sample entry to the specified buffer.
   *
   * Assumes reader is not empty (i.e., !empty())
   * Assumes type is PERF_RECORD_SAMPLE
   * Assumes there is enough space in the destination (>= peek_sample_length())
   */
  void pop_and_copy_to(char *dest);

  /**
   * Copies the payload of a sample entry to the specified buffer.
   * Uses the native size of the payload message instead of a header to
   * determine the length Assumes there is no padding on the perf_submit side
   *
   * Assumes reader is not empty (i.e., !empty())
   * Assumes type is PERF_RECORD_SAMPLE
   * Assumes there is enough space in the destination (>= peek_sample_length())
   */
  void pop_unpadded_and_copy_to(char *dest);

  /**
   * Pops the entry, and updates data structures
   *
   * Assumes reader is not empty (i.e., !empty())
   */
  void pop();

  /**
   * Explicitly finish the batch
   */
  void stop();

private:
  /**
   * Returns the reader with the smallest
   */
  inline PerfRing &top()
  {
    size_t idx = container_.entries_[0].reader_index;
    return container_.readers_[idx];
  }

  inline PerfRing const &top() const
  {
    size_t idx = container_.entries_[0].reader_index;
    return container_.readers_[idx];
  }

  /**
   * Updates container_.entries_ and container_.readers_in_entries_
   */
  void update_after_pop();

  /**
   * Updates the reader's state given that it is not in container_.entries_
   */
  void update_when_not_in_entries(size_t idx);

  /* container to read from */
  PerfContainer &container_;

  /* the maximum timestamp we should accept */
  u64 max_timestamp_;

  /* is the reader active */
  bool active_;

  /* This class acts as a "Guard", so disallow copy and assignment */
  PerfReader(const PerfReader &) = delete;
  void operator=(const PerfReader &) = delete;
};
