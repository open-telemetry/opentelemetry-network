/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <jitbuf/fixed_handler.h>

#include <platform/platform.h>

typedef uint32_t u32;
typedef uint16_t u16;

#define EMPTY_RPC_ID ((u32)-1)

int jbf_init(
    struct jitbuf_fixed *client, struct jitbuf_handler *handler_buf, uint32_t n_handlers, uint32_t (*hash_fn)(uint32_t rpc_id))
{
  uint32_t i;

  if ((client == NULL) || (handler_buf == NULL) || (hash_fn == NULL))
    return -EINVAL;

  /* initialize the buffer */
  for (i = 0; i < n_handlers; i++)
    handler_buf[i].rpc_id = EMPTY_RPC_ID;

  /* initialize the client struct */
  client->handlers = handler_buf;
  client->n_handlers = n_handlers;
  client->hash_fn = hash_fn;

  return 0;
}

int jbf_add(struct jitbuf_fixed *client, const struct jb_rpc *rpc, jbc_handler_func f, void *context)
{
  struct jitbuf_handler *h_elem;
  uint32_t slot;

  if ((client == NULL) || (rpc == NULL))
    return -EINVAL;
  if (rpc->rpc_id == EMPTY_RPC_ID)
    return -EINVAL;
  if ((rpc->descriptor == NULL) || (rpc->descriptor->size == 0))
    return -EINVAL;
  if (f == NULL)
    return -EINVAL;

  /* find the table element this handler should be in */
  slot = client->hash_fn(rpc->rpc_id);
  assert(slot < client->n_handlers);

  h_elem = &client->handlers[slot];
  if (h_elem->rpc_id != EMPTY_RPC_ID)
    return -ENOSPC;

  /* initialize fields in the table location */
  h_elem->rpc_id = rpc->rpc_id;
  h_elem->f = f;
  h_elem->context = context;
  h_elem->descriptor = rpc->descriptor->buf;
  h_elem->descriptor_len = rpc->descriptor->size;
  h_elem->buffer_len = rpc->size;

  return 0;
}

int jbf_make_descriptor(struct jitbuf_fixed *client, char *buf, int len)
{
  int i = 0;
  int used = 0;

  for (i = 0; i < client->n_handlers; i++) {
    int desc_len = client->handlers[i].descriptor_len;

    if (client->handlers[i].rpc_id == EMPTY_RPC_ID)
      continue;

    if (client->handlers[i].descriptor_len > len)
      return -ENOSPC;

    memcpy(buf, client->handlers[i].descriptor, desc_len);
    buf += desc_len;
    len -= desc_len;
    used += desc_len;
  }

  return used;
}

int jbf_handle(struct jitbuf_fixed *client, char *buf, int len)
{
  u32 rpc_id;
  uint32_t slot;
  struct jitbuf_handler *h_elem;

  /* if there's not even enough space for the rpc_id, fail. */
  if (len < 2)
    return -EINVAL;

  /* find the table entry for the buffer's rpc_id */
  rpc_id = *(u16 *)buf;
  slot = client->hash_fn(rpc_id);
  assert(slot < client->n_handlers);

  h_elem = &client->handlers[slot];

  /* does the table contain a handler for this rpc_id? */
  if (h_elem->rpc_id != rpc_id)
    return -ENOENT;

  /* is the buffer too short to contain the handled struct? */
  if (h_elem->buffer_len > len)
    return -EINVAL;

  /* okay, can run the handler */
  h_elem->f(h_elem->context, buf);

  /* return the number of bytes the struct consumed */
  return h_elem->buffer_len;
}
