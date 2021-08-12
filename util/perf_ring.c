#include <util/perf_ring.h>

int pr_init_contig(struct perf_ring *eq, void *data, u32 n_pages, u64 page_size)
{
  __u64 buf_len;

  if ((eq == NULL) || (data == NULL))
    return -EINVAL;

  eq->shared = (struct perf_event_mmap_page *)data;
  eq->data = (char *)data + page_size;

  buf_len = n_pages * page_size;
  if ((buf_len & (buf_len - 1)) || (buf_len < 8))
    return -EINVAL;

  eq->buf_mask = buf_len - 1;

  eq->buf_head = eq->shared->data_tail;
  eq->buf_tail = eq->shared->data_head;

  return 0;
}

int pr_write(struct perf_ring *eq, u16 len, u32 type)
{
  u32 buf_tail;
  u32 buf_mask;
  u32 aligned_len = ((u32)len + sizeof(struct perf_event_header) + 7) & ~7;
  struct perf_event_header *header;

  /* check input parameters */
  if (eq == NULL)
    return -EINVAL;
  if (aligned_len > eq->buf_mask)
    return -EINVAL;
  if (aligned_len >= (1 << 16))
    return -EINVAL;

  buf_mask = eq->buf_mask;
  buf_tail = eq->buf_tail;

  /* is there enough space in the eq? */
  if (buf_tail + aligned_len - eq->buf_head > buf_mask + 1)
    return -ENOSPC;

  /* okay we're good to go */
  header = (struct perf_event_header *)&eq->data[buf_tail & buf_mask];
  header->size = aligned_len;
  header->type = type;
  eq->buf_tail = buf_tail + aligned_len;
  return ((buf_tail + sizeof(struct perf_event_header)) & buf_mask);
}

int pr_peek_size(const struct perf_ring *eq)
{
  struct perf_event_header *header;

  /* is the queue empty? */
  if (eq->buf_tail == eq->buf_head)
    return -ENOENT;

  /* sanity check: there should be space for the header */
  assert((eq->buf_head & 7) == 0);

  header = (struct perf_event_header *)&eq->data[eq->buf_head & eq->buf_mask];
  return header->size - sizeof(struct perf_event_header);
}

u32 pr_peek_type(const struct perf_ring *eq)
{
  const struct perf_event_header *header;

  assert(eq->buf_tail != eq->buf_head);

  header = (const struct perf_event_header *)&eq->data[eq->buf_head & eq->buf_mask];
  return header->type;
}

u64 pr_peek_aligned_u64(const struct perf_ring *eq, u16 offset)
{
  u32 total_offset;

  assert((int)eq->buf_tail - (eq->buf_head + offset + 8) >= 0);
  assert(((eq->buf_head + offset) & 7) == 0);

  total_offset = eq->buf_head + sizeof(struct perf_event_header) + offset;

  return *(u64 *)&eq->data[total_offset & eq->buf_mask];
}

u32 pr_peek_aligned_u32(const struct perf_ring *eq, u16 offset)
{
  u32 total_offset;

  assert((int)eq->buf_tail - (eq->buf_head + offset + 4) >= 0);
  assert(((eq->buf_head + offset) & 3) == 0);

  total_offset = eq->buf_head + sizeof(struct perf_event_header) + offset;

  return *(u32 *)&eq->data[total_offset & eq->buf_mask];
}

u16 pr_peek_aligned_u16(const struct perf_ring *eq, u16 offset)
{
  u32 total_offset;

  assert((int)eq->buf_tail - (eq->buf_head + offset + 2) >= 0);
  assert(((eq->buf_head + offset) & 1) == 0);

  total_offset = eq->buf_head + sizeof(struct perf_event_header) + offset;

  return *(u16 *)&eq->data[total_offset & eq->buf_mask];
}

void pr_peek_copy(const struct perf_ring *eq, char *buf, u16 offset, u16 len)
{
  u32 begin_head;
  u32 begin;
  u32 end;
  u32 buf_mask = eq->buf_mask;

  if (len == 0)
    return;

  assert(pr_peek_size(eq) >= (int)offset + len);

  begin_head = eq->buf_head + sizeof(struct perf_event_header) + offset;
  begin = begin_head & buf_mask;
  end = (begin_head + len - 1) & buf_mask;

  if (end < begin) {
    /* wraps around */
    int len_to_ring_end = buf_mask + 1 - begin;
    memcpy(buf, &eq->data[begin], len_to_ring_end);
    memcpy(buf + len_to_ring_end, &eq->data[0], end + 1);
  } else {
    /* simple case */
    memcpy(buf, &eq->data[begin], len);
  }
}

struct pr_data_view pr_peek(const struct perf_ring *eq)
{
  struct pr_data_view result = {0};

  const u32 unpadded_len = pr_peek_aligned_u32(eq, sizeof(u32));
  const u32 offset = sizeof(u64);

  const u32 begin_head = eq->buf_head + sizeof(struct perf_event_header) + offset;
  const u32 begin = begin_head & eq->buf_mask;
  const u32 end = (begin_head + unpadded_len - 1) & eq->buf_mask;

  if (unpadded_len == 0) {
    return result;
  }

  assert(pr_peek_size(eq) >= (int)offset + unpadded_len);

  if (end < begin) {
    /* wraps around */
    int len_to_ring_end = eq->buf_mask + 1 - begin;
    result.first = &eq->data[begin];
    result.first_len = len_to_ring_end;
    result.second = &eq->data[0];
    result.second_len = end + 1;
  } else {
    /* simple case */
    result.first = &eq->data[begin];
    result.first_len = unpadded_len;
  }

  return result;
}

int pr_read(struct perf_ring *eq, u16 *lenp)
{
  struct perf_event_header *header;
  u32 buf_head;
  u32 buf_mask;

  /* check input parameters */
  if ((eq == NULL) || (lenp == NULL))
    return -EINVAL;

  /* is the queue empty? */
  if (eq->buf_tail == eq->buf_head)
    return -EAGAIN;

  /* sanity check: there should be space for the header */
  assert((eq->buf_head & 7) == 0);

  buf_mask = eq->buf_mask;
  buf_head = eq->buf_head;

  header = (struct perf_event_header *)&eq->data[buf_head & buf_mask];

  *lenp = header->size - sizeof(struct perf_event_header);
  eq->buf_head += header->size;

  return ((buf_head + sizeof(struct perf_event_header)) & buf_mask);
}

u32 pr_bytes_remaining(const struct perf_ring *eq, u32 *total_size)
{
  /* check input parameters */
  if (eq == NULL)
    return -EINVAL;

  u32 buf_size = eq->buf_mask + 1;
  if (total_size) {
    *total_size = buf_size;
  }
  u32 begin = eq->buf_head & eq->buf_mask;
  u32 end = eq->buf_tail & eq->buf_mask;

  if (end < begin) {
    return (buf_size - begin) + end;
  }

  return (end - begin);
}
