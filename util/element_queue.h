/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_FASTPASS_UTIL_ELEMENT_QUEUE_H_
#define INCLUDE_FASTPASS_UTIL_ELEMENT_QUEUE_H_

#include <platform/platform.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct element_queue_shared {
  u32 elem_head;
  u32 buf_head;
  u32 elem_tail;
  u32 buf_tail;
};

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
 */
struct element_queue {
  u32 elem_mask;
  u32 buf_mask;

  u32 elem_head;
  u32 buf_head;
  u32 elem_tail;
  u32 buf_tail;

  struct element_queue_shared *shared;
  u32 *elems;
  char *data;
};

/**
 * Initializes a struct element_queue_shared
 */
void eq_init_shared(struct element_queue_shared *shared);

/**
 * Initializes the element queue using a contiguous memory area
 *
 * @param eq: the element_queue to initialize
 * @param n_elems: number of elements in shared buffer. Must be power of 2.
 * @param buf_len: number of bytes in shared buffer. Must be power of 2.
 * @param data: shared memory area
 *
 * Memory layout of eq data:
 *   element_queue_shared
 *   u32 elem[n_elems]
 *   char buf[buf_len]
 *
 * @returns 0 on success,
 *   -EINVAL on NULL pointers, non-power-of 2 sizes.
 */
int eq_init_contig(struct element_queue *eq, u32 n_elems, u32 buf_len, void *data);

/**
 * Returns the size, in bytes, of a contiguous element queue
 */
u32 eq_contig_size(u32 n_elems, u32 buf_len);

/**
 * Starts a write batch
 * @param eq: the eq to write to
 *
 * @important: the user must call finish_write after finishing the batch. The
 *   consumer can only see results after a batch is finished
 */
static inline void eq_start_write_batch(struct element_queue *eq)
{
  eq->elem_head = ACCESS_ONCE(eq->shared->elem_head);
  eq->buf_head = ACCESS_ONCE(eq->shared->buf_head);

  assert((int)eq->buf_tail - eq->buf_head >= 0);
  assert((int)eq->elem_tail - eq->elem_head >= 0);
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
int eq_write(struct element_queue *eq, u32 len);

/**
 * Finishes a batch write to the element_queue
 * @param eq: the element_queue written to
 *
 * @assumes a successful start_write_batch
 */
static inline void eq_finish_write_batch(struct element_queue *eq)
{
  /* make sure items have been committed before writing the tails */
  smp_wmb();

  assert((int)eq->buf_tail - eq->buf_head >= 0);
  assert((int)eq->elem_tail - eq->elem_head >= 0);

  eq->shared->buf_tail = eq->buf_tail;
  eq->shared->elem_tail = eq->elem_tail;
}

/**
 * Starts a read batch
 * @param eq: the element_queue to read from
 *
 * @important: after reading the data, the user must call finish_read_batch
 */
static inline void eq_start_read_batch(struct element_queue *eq)
{
  /* we don't need to read buf_tail because we trust that the sizes in the
   * element-size array do not overflow the element_queue */

  eq->elem_tail = ACCESS_ONCE(eq->shared->elem_tail);
  smp_rmb();

  assert((int)eq->elem_tail - eq->elem_head >= 0);
}

/**
 * Reads the next element's size, or -ENOENT if no element exists
 */
int eq_peek(struct element_queue *eq);

/**
 * Reads the next element's offset into data, or -ENOENT if no element exists
 * @param lenp: [out] if not NULL, where to store the element length
 */
int eq_peek_offset(struct element_queue *eq, u32 *lenp);

/**
 * Read data from a element_queue
 * @param eq: the element_queue to read from
 * @param lenp: [out] length of the buffer to read
 *
 * @return: on success, offset into the data where to read;
 *   -EINVAL if passed NULL pointers,
 *   -EAGAIN if eq is empty
 */
int eq_read(struct element_queue *eq, u32 *lenp);

/**
 * Finish the read batch, freeing space for producer
 * @param eq: the element_queue to read from
 */
static inline void eq_finish_read_batch(struct element_queue *eq)
{
  smp_mb();

  eq->shared->buf_head = eq->buf_head;
  eq->shared->elem_head = eq->elem_head;

  assert((int)eq->elem_tail - eq->elem_head >= 0);
}

/**
 * Moves one element from a source element queue to destination element queue.
 * @param to: the destination element queue
 * @param from: the source element queue
 *
 * @returns: number of bytes moved on success,
 *   -ENOENT if source is empty,
 *   -ENOSPC if destination does not have enough space,
 *   -ENOSYS on enexpected error.
 */
int eq_move(struct element_queue *to, struct element_queue *from);

static inline u32 eq_elem_count(const struct element_queue *eq)
{
  return eq->elem_tail - eq->elem_head;
}

static inline u32 eq_elem_capacity(const struct element_queue *eq)
{
  return eq->elem_mask + 1;
}

static inline u32 eq_buf_used(const struct element_queue *eq)
{
  return eq->buf_tail - eq->buf_head;
}

static inline u32 eq_buf_capacity(const struct element_queue *eq)
{
  return eq->buf_mask + 1;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDE_FASTPASS_UTIL_ELEMENT_QUEUE_H_ */
