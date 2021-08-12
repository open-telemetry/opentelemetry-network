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

/*
 * circular_queue.h
 */

#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <platform/platform.h>

/**
 * @param head: index of the first item in the queue
 * @param tail: index following the last item in the queue
 * @param mask: the number of allowed elements (which is a power of two) minus 1
 * @param elems: pointer to element buffer
 */
struct circular_queue {
  uint32_t head;
  uint32_t tail;
  uint32_t mask;
  void *elems;
};

/**
 * Creates a new packet queue with capacity @num_elems.
 * @important: @num_elems must be a power of two
 * @param elems: a buffer large enough to hold num_elems elements
 * @returns: 0 on success,
 *           -EINVAL if num_elems is not a power of two
 */
static inline int cq_init(struct circular_queue *q, void *elems, uint32_t num_elems)
{
  assert(q != NULL);

  if ((num_elems & (num_elems - 1)) != 0)
    return -EINVAL;

  q->head = 0;
  q->tail = 0;
  q->mask = num_elems - 1;
  q->elems = elems;

  return 0;
}

/**
 * Enqueues @x at the end of @q.
 * @assumes there is enough space in q. (user can check with cq_full())
 */
#define cq_enqueue(name, type)                                                                                                 \
  static inline void cq_##name##_enqueue(struct circular_queue *q, type x)                                                     \
  {                                                                                                                            \
    uint32_t tail;                                                                                                             \
                                                                                                                               \
    assert(q != NULL);                                                                                                         \
                                                                                                                               \
    tail = q->tail;                                                                                                            \
    ((type *)q->elems)[tail & q->mask] = x;                                                                                    \
    barrier();                                                                                                                 \
    ACCESS_ONCE(q->tail) = tail + 1;                                                                                           \
  }

/**
 * Dequeues an element from @q.
 * @assumes q is non-empty (user can check with cq_empty())
 */
#define cq_dequeue(name, type)                                                                                                 \
  static inline type cq_##name##_dequeue(struct circular_queue *q)                                                             \
  {                                                                                                                            \
    uint32_t head;                                                                                                             \
    type retval;                                                                                                               \
                                                                                                                               \
    assert(q != NULL);                                                                                                         \
                                                                                                                               \
    head = q->head;                                                                                                            \
    retval = ((type *)q->elems)[head & q->mask];                                                                               \
    barrier();                                                                                                                 \
    ACCESS_ONCE(q->head) = head + 1;                                                                                           \
    return retval;                                                                                                             \
  }

/**
 * Returns the head element from @q without dequeuing it.
 * @assumes q is non-empty (user can check with cq_empty())
 */
#define cq_peek(name, type)                                                                                                    \
  static inline type cq_##name##_peek(struct circular_queue *q)                                                                \
  {                                                                                                                            \
    assert(q != NULL);                                                                                                         \
    return ((type *)q->elems)[q->head & q->mask];                                                                              \
  }

/**
 * Returns 1 if @q is empty, 0 otherwise.
 */
static inline int cq_empty(struct circular_queue *q)
{
  assert(q != NULL);
  return (q->tail == q->head);
}

/**
 * Returns the current occupancy of the queue.
 * (to be used by the consumer)
 */
static inline uint32_t cq_occupancy(struct circular_queue *q)
{
  assert(q != NULL);
  return q->tail - q->head;
}

/**
 * Returns the number of free slots in the queue
 * (to be used by the producer)
 */
static inline uint32_t cq_space(struct circular_queue *q)
{
  assert(q != NULL);
  return q->mask + 1 - (q->tail - q->head);
}

/**
 * Returns 1 if @q is full, 0 otherwise.
 */
static inline int cq_full(struct circular_queue *q)
{
  return (cq_occupancy(q) >= q->mask + 1);
}

/**
 * Discards 'n_elem' elements from the head of the queue
 */
static inline void cq_discard(struct circular_queue *q, uint32_t n_elems)
{
  assert(q != NULL);
  assert(n_elems <= cq_occupancy(q));

  ACCESS_ONCE(q->head) = q->head + n_elems;
}

/**
 * Macro to define functions for different types.
 *   For example,
 *      using_circular_queue(u32, uint32_t);
 *   will define functions
 *      void cq_u32_enqueue(struct circular_queue *q, uint32_t x);
 *      uint32_t cq_u32_dequeue(struct circular_queue *q);
 *   etc.
 */
#define using_circular_queue(name, type)                                                                                       \
  cq_enqueue(name, type);                                                                                                      \
  cq_dequeue(name, type);                                                                                                      \
  cq_peek(name, type);

using_circular_queue(u16, u16);
using_circular_queue(u32, u32);
using_circular_queue(u64, u64);

#endif /* CIRCULAR_QUEUE_H */
