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

#include <util/element_queue.h>

/**
 * Internal helper, calculates where the next read/write should be based on
 *    the buffer pointer and buffer length.
 */
static inline u32 __eq_next_offset_by_len(u32 buf_offset, u32 buf_mask, u32 len)
{
  /* if writing the buffer will wraps around, we start at buffer's beginning */
  if ((int)(((buf_offset + len - 1) & ~buf_mask) - (buf_offset & ~buf_mask)) > 0)
    buf_offset = (buf_offset + len - 1) & ~buf_mask;

  return buf_offset;
}

void eq_init_shared(struct element_queue_shared *shared)
{
  shared->elem_head = 0;
  shared->elem_tail = 0;
  shared->buf_head = 0;
  shared->buf_tail = 0;
}

int eq_init_contig(struct element_queue *eq, u32 n_elems, u32 buf_len, void *data)
{
  if ((eq == NULL) || (data == NULL))
    return -EINVAL;
  if (n_elems & (n_elems - 1))
    return -EINVAL;
  if (buf_len & (buf_len - 1))
    return -EINVAL;

  eq->shared = (struct element_queue_shared *)data;
  eq->elems = (u32 *)&eq->shared[1];
  eq->data = (char *)&eq->elems[n_elems];

  eq->elem_mask = n_elems - 1;
  eq->buf_mask = buf_len - 1;

  eq->elem_head = eq->shared->elem_head;
  eq->elem_tail = eq->shared->elem_tail;
  eq->buf_head = eq->shared->buf_head;
  eq->buf_tail = eq->shared->buf_tail;

  return 0;
}

u32 eq_contig_size(u32 n_elems, u32 buf_len)
{
  struct element_queue_shared *shared = (struct element_queue_shared *)0;
  u32 *elems = (u32 *)&shared[1];
  char *data = (char *)&elems[n_elems];
  return (u32)(&data[buf_len] - (char *)0);
}

int eq_write(struct element_queue *eq, u32 len)
{
  u32 buf_tail;
  u32 buf_mask;
  u32 aligned_len = (len + 7) & ~7;

  /* check input parameters */
  if (eq == NULL)
    return -EINVAL;
  if (aligned_len > eq->buf_mask)
    return -EINVAL;

  /* is the element queue full? */
  if (eq->elem_tail - eq->elem_head >= eq->elem_mask + 1)
    return -ENOSPC;

  buf_mask = eq->buf_mask;
  buf_tail = __eq_next_offset_by_len(eq->buf_tail, buf_mask, aligned_len);

  /* is there enough space in the eq? */
  if (buf_tail + aligned_len - eq->buf_head > buf_mask + 1)
    return -ENOSPC;

  /* okay we're good to go */
  eq->buf_tail = buf_tail + aligned_len;
  eq->elems[(eq->elem_tail++) & eq->elem_mask] = len;
  return (buf_tail & buf_mask);
}

int eq_peek(struct element_queue *eq)
{
  /* is the element queue empty? */
  if (eq->elem_tail == eq->elem_head)
    return -ENOENT;

  return eq->elems[eq->elem_head & eq->elem_mask];
}

int eq_peek_offset(struct element_queue *eq, u32 *lenp)
{
  u32 len;
  u32 aligned_len;
  u32 offset;

  /* is the element queue empty? */
  if (eq->elem_tail == eq->elem_head) {
    return -ENOENT;
  }

  len = eq->elems[eq->elem_head & eq->elem_mask];
  aligned_len = (len + 7) & ~7;
  offset = __eq_next_offset_by_len(eq->buf_head, eq->buf_mask, aligned_len);

  if (lenp != NULL) {
    *lenp = len;
  }

  return (offset & eq->buf_mask);
}

int eq_read(struct element_queue *eq, u32 *lenp)
{
  u32 offset;
  u32 buf_mask;
  u32 aligned_len;

  /* check input parameters */
  if ((eq == NULL) || (lenp == NULL))
    return -EINVAL;

  /* is the element queue empty? */
  if (eq->elem_tail == eq->elem_head)
    return -EAGAIN;

  *lenp = eq->elems[(eq->elem_head++) & eq->elem_mask];
  aligned_len = (*lenp + 7) & ~7;
  buf_mask = eq->buf_mask;
  offset = __eq_next_offset_by_len(eq->buf_head, buf_mask, aligned_len);
  eq->buf_head = offset + aligned_len;

  return (offset & buf_mask);
}

int eq_move(struct element_queue *to, struct element_queue *from)
{
  int read_offset;
  int write_offset;
  u32 len = 0;
  int peek_len;

  /* how big is this message */
  peek_len = eq_peek(from);
  if (peek_len < 0)
    return -ENOENT;

  /* try to get a write buffer of that size */
  write_offset = eq_write(to, peek_len);
  if (write_offset == -ENOSPC)
    return -ENOSPC;
  else if (write_offset < 0) {
    assert(0);
    return -ENOSYS; /* unexpected error! */
  }

  /* make the read */
  read_offset = eq_read(from, &len);
  if ((read_offset < 0) || (len != peek_len)) {
    assert(0);
    return -ENOSYS; /* unexpected error! */
  }

  /* copy packet from transmission queue to retrans queue */
  memcpy(&to->data[write_offset], &from->data[read_offset], len);

  return len;
}
