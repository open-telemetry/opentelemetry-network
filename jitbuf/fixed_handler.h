/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_JITBUF_FIXED_HANDLER_H_
#define INCLUDE_JITBUF_FIXED_HANDLER_H_

#include <jitbuf/jb.h>
#include <platform/platform.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*jbc_handler_func)(void *context, void *buf);

struct jitbuf_handler {
  uint32_t rpc_id;
  jbc_handler_func f;
  void *context;
  const char *descriptor;
  int descriptor_len;
  int buffer_len;
};

struct jitbuf_fixed {
  struct jitbuf_handler *handlers;
  uint32_t n_handlers;
  uint32_t (*hash_fn)(uint32_t rpc_id);
};

/**
 * Initializes the jitbuf client
 *
 * @param client: the client to initialize
 * @param handler_buf: an array of struct jitbuf_handlers for storing handlers
 * @param n_handlers: the number of handlers in handler_buf
 * @param hash_fn: a hash function that maps rpc ID to [0,n_handlers)
 *
 * @returns -EINVAL if pointers are NULL or 0 on success.
 */
int jbf_init(
    struct jitbuf_fixed *client, struct jitbuf_handler *handler_buf, uint32_t n_handlers, uint32_t (*hash_fn)(uint32_t rpc_id));

/**
 * Adds an RPC handler to the client
 *
 * @param client: the client to add to
 * @param rpc: the RPC descriptor (from *.jb.h) that describes the RPC.
 *   must remain alive while the jitbuf_client is alive.
 * @param f: the handler function to call
 * @param context: the context to pass to the handler
 *
 * @returns -EINVAL if pointers are NULL or sanity check of handler fields fails
 *   -ENOSPC if hashing the added RPC ID yields an occupied slot, 0 on success.
 *
 * @note rpc_id == (u32)-1 is reserved.
 */
int jbf_add(struct jitbuf_fixed *client, const struct jb_rpc *rpc, jbc_handler_func f, void *context);

/**
 * Encodes an aggregate descriptor holding all descriptors for handled rpc_ids
 *
 * @param client: the client to create a descriptor for
 * @param buf: where to write the output descriptor
 * @param len: the size of buf
 *
 * @returns -ENOSPC if buf is too short, the length of the descriptor on success
 */
int jbf_make_descriptor(struct jitbuf_fixed *client, char *buf, int len);

/**
 * Handles an incoming buffer.
 *
 * @param client: the client holding the different handlers
 * @param buf: the buffer containing an entry to be handled
 * @param len: the length of the buffer. It could be larger than the entry.
 *
 * @returns: -EINVAL if len is too small for the rpc_id,
 *   -ENOENT if buffer's rpc_id has not been added to the client
 *   or the number of consumed bytes on success.
 */
int jbf_handle(struct jitbuf_fixed *client, char *buf, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDE_JITBUF_FIXED_HANDLER_H_ */
