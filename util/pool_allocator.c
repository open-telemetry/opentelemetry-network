/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <util/pool_allocator.h>

int pool_allocator_init(struct pool_allocator *map, void *pool, u32 elem_size, u32 n_elems)
{
  if ((map == NULL) || (pool == NULL) || (n_elems == 0) || (elem_size < sizeof(u32)))
    return -EINVAL;

  map->pool = pool;
  map->elem_size = elem_size;
  map->n_elems = n_elems;

  map->free_list = 0;
  map->alloc_end = 0;

  return 0;
}
