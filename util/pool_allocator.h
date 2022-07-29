/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_FASTPASS_UTIL_POOL_ALLOCATOR_H_
#define INCLUDE_FASTPASS_UTIL_POOL_ALLOCATOR_H_

#include <platform/platform.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define POOL_ALLOCATOR_NULL_ELEM ((u32)~0U)

/**
 * @pool: The pool of elements. If the element is free, the first u32 is the
 *   index of the next free element (or ~0 if last free element). Otherwise,
 *   contains user data (not owned by the allocator).
 * @free_list: the index of the free list's head element, or ~0 otherwise.
 * @elem_size: the length of each element in bytes (i.e., sizeof an elem)
 */
struct pool_allocator {
  void *pool;
  u32 free_list;
  u32 alloc_end;
  u32 elem_size;
  u32 n_elems;
};

/**
 * Internal function to compute a pointer to an elem, cast as u32*
 */
static inline u32 *__pool_allocator_elem(struct pool_allocator *map, u32 index)
{
  char *charp = ((char *)map->pool) + index * map->elem_size;
  return (u32 *)charp;
}

/**
 * Allocates a new element from the pool.
 *
 * @param map: the pool_allocator to allocate from
 *
 * @returns the index of a new element on success, ~0 if no free entries
 */
static inline u32 pool_allocator_alloc(struct pool_allocator *map)
{
  u32 id;
  u32 next;

  assert(map != NULL);
  assert(map->free_list <= map->alloc_end);

  if (map->free_list == POOL_ALLOCATOR_NULL_ELEM)
    return ~0;

  id = map->free_list;

  if (id == map->alloc_end) {
    // this element was never allocated
    next = id < (map->n_elems - 1) ? id + 1 : POOL_ALLOCATOR_NULL_ELEM;
    map->alloc_end = next;
  } else {
    // this element was previously returned to the free list
    next = *__pool_allocator_elem(map, id);
  }

  map->free_list = next;

  // zero memory before returning
  memset(__pool_allocator_elem(map, id), 0, map->elem_size);

  return id;
}

/**
 * Frees the unique ID for further allocation
 */
static inline void pool_allocator_free(struct pool_allocator *map, u32 id)
{
  assert((map != NULL) && (id < map->n_elems));

  *__pool_allocator_elem(map, id) = map->free_list;
  map->free_list = id;
}

/**
 * Initializes the ID map
 *
 * @param map: the map to initialize
 * @param pool: pointer to the pool of elements allocating from
 * @param elem_size: bytes
 * @param n_elems: the maximum number of elements supported
 *
 * @returns: 0 on success, -EINVAL on (n_elems == 0), NULL map or pool, or
 *   when elem_size is too small to hold a u32.
 */
int pool_allocator_init(struct pool_allocator *map, void *pool, u32 elem_size, u32 n_elems);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDE_FASTPASS_UTIL_POOL_ALLOCATOR_H_ */
