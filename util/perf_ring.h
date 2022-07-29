/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_FASTPASS_UTIL_PERF_RING_H_
#define INCLUDE_FASTPASS_UTIL_PERF_RING_H_

#include <linux/perf_event.h>
#include <platform/platform.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * A queue structure conveying discrete elements of variable length.
 *
 * Constraints:
 *  - all readers must share the same struct
 *  - all writers must share the same struct
 *  - readers can use a separate struct from writers
 *  - all reads must be surrounded by start_read_batch and finish_read_batch
 *  - all writes must be surrounded by start_write_batch and finish_read_batch
 *
 * @param buf_head: index of the next unread char in the eq
 * @param buf_tail: index following the eq's last written char
 *
 * Format of events in the ring:
 *  BEGIN perf_event_header
 *  - u32: type
 *  - u16: misc
 *  - u16: perf event length (64 bits aligned)
 *  END perf_event_header
 *  BEGIN HEADER
 *  - u32: aligned length (64 bits aligned)
 *  - u32: unpadded length (<= aligned length - sizeof(HEADER))
 *  END HEADER
 *  BEGIN PERF EVENT CONTENTS
 *  - u8[aligned length - sizeof(HEADER)]: opaque perf event payload
 *  END PERF EVENT CONTENTS
 */
struct perf_ring {
  u32 buf_mask;

  u32 buf_head;
  u32 buf_tail;

  struct perf_event_mmap_page *shared;
  char *data;
};

/**
 * Initializes the perf ring using a contiguous memory area
 *
 * @param eq: the element_queue to initialize
 * @param data: shared memory area
 * @param n_pages: number of data pages in the perf ring
 * @param page_size: size of a page
 *
 * Memory layout of eq data:
 *   element_queue_shared
 *   u32 elem[n_elems]
 *   char buf[buf_len]
 *
 * @returns 0 on success,
 *   -EINVAL on NULL pointers, non-power-of 2 sizes.
 */
int pr_init_contig(struct perf_ring *eq, void *data, u32 n_pages, u64 page_size);

/**
 * Returns the size, in bytes, of a contiguous element queue
 */
u32 pr_contig_size(u32 n_elems, u32 buf_len);

/**
 * Starts a write batch
 * @param eq: the eq to write to
 *
 * @important: the user must call finish_write after finishing the batch. The
 *   consumer can only see results after a batch is finished
 */
static inline void pr_start_write_batch(struct perf_ring *eq)
{
  eq->buf_head = ACCESS_ONCE(eq->shared->data_tail);

  assert((int)eq->buf_tail - eq->buf_head >= 0);
}

/**
 * Get a buffer where data can be written to the eq
 * @param eq: the eq to write to
 * @param len: number of bytes we want to write
 *
 * @return: offset in eq where data should be written
 *   -EINVAL if trying to write more than the eq size or passed NULL pointer
 *   -ENOSPC if eq too full
 */
int pr_write(struct perf_ring *eq, u16 len, u32 type);

/**
 * Finishes a batch write to the element_queue
 * @param eq: the element_queue written to
 *
 * @assumes a successful start_write_batch
 */
static inline void pr_finish_write_batch(struct perf_ring *eq)
{
  /* make sure items have been committed before writing the tails */
  smp_wmb();

  assert((int)eq->buf_tail - eq->buf_head >= 0);

  eq->shared->data_head = eq->buf_tail;
}

/**
 * Starts a read batch
 * @param eq: the element_queue to read from
 *
 * @important: after reading the data, the user must call finish_read_batch
 */
static inline void pr_start_read_batch(struct perf_ring *eq)
{
  /* we don't need to read buf_tail because we trust that the sizes in the
   * element-size array do not overflow the element_queue */

  eq->buf_tail = ACCESS_ONCE(eq->shared->data_head);
  smp_rmb();

  assert((int)eq->buf_tail - eq->buf_head >= 0);
}

/**
 * Reads the next element's size, or -ENOENT if no element exists
 */
int pr_peek_size(const struct perf_ring *eq);

/**
 * Reads the next element's type.
 *
 * Assumes element exists
 */
u32 pr_peek_type(const struct perf_ring *eq);

/**
 * Reads an aligned u64 from the variable at given offset
 *
 * Assumes element exists
 */
u64 pr_peek_aligned_u64(const struct perf_ring *eq, u16 offset);

/**
 * Reads an aligned u32 from the variable at given offset
 *
 * Assumes element exists
 */
u32 pr_peek_aligned_u32(const struct perf_ring *eq, u16 offset);

/**
 * Reads an aligned u16 from the variable at given offset
 *
 * Assumes element exists
 */
u16 pr_peek_aligned_u16(const struct perf_ring *eq, u16 offset);

/**
 * Copies @len bytes from the ring's head at given @offset to @buf
 */
void pr_peek_copy(const struct perf_ring *eq, char *buf, u16 offset, u16 len);

/**
 * This struct represents a view of a logically contiguous chunk of data in the
 * perf ring.
 *
 * The perf ring is implemented as a ring buffer, so if events are located at
 * the end of the ring and wrap-around to the beginning, then the view will be
 * spread over two contiguous chunks, in the order specified by `first` and
 * `second` members. Otherwise, the view will have exactly one contiguous chunk
 * represented by `first`, and `second` will be empty.
 */
struct pr_data_view {
  char const *first;
  u16 first_len;
  char const *second;
  u16 second_len;
};

/**
 * Peek data from a element_queue
 * @param eq: the element_queue to read from
 *
 * @return: views into data of element, without perf_event_header.
 */
struct pr_data_view pr_peek(const struct perf_ring *eq);

/**
 * Read data from a element_queue
 * @param eq: the element_queue to read from
 * @param lenp: [out] length of the buffer to read
 *
 * @return: on success, offset into data of element, without perf_event_header;
 *   -EINVAL if passed NULL pointers,
 *   -EAGAIN if eq is empty
 */
int pr_read(struct perf_ring *eq, u16 *lenp);

/**
 * Return the number of bytes left to read in the perf ring
 * @param eq: the eq to read from
 * @param total_size: option parameter to return the total size of the ring
 *
 * @return: number of bytes remaining to read in the perf ring
 *   -EINVAL if a NULL pointer is passed
 */

u32 pr_bytes_remaining(const struct perf_ring *eq, u32 *total_size);

/**
 * Finish the read batch, freeing space for producer
 * @param eq: the element_queue to read from
 */
static inline void pr_finish_read_batch(struct perf_ring *eq)
{
  smp_mb();

  eq->shared->data_tail = eq->buf_head;

  assert((int)eq->buf_tail - eq->buf_head >= 0);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDE_FASTPASS_UTIL_PERF_RING_H_ */
