/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_JITBUF_JLOG_OUTPUT_FACTORY_H_
#define INCLUDE_JITBUF_JLOG_OUTPUT_FACTORY_H_

#include <platform/spin_lock.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct element_queue;

/**
 * Holds information on how to handle verbose logging
 */
struct jlog_output_factory {
  /**
   * Configures a new element queue with verbose log outputs
   * @param factory: this factory
   * @param eq: the element queue to configure
   * @param write_lock: the write lock to use for verbose logging
   *
   * @returns: 0 on success, negative value on error
   */
  int (*create)(struct jlog_output_factory *factory, struct element_queue **eq_ptr, fp_spinlock_t **write_lock_ptr);

  /**
   * Frees the jlog output associated with the given context
   * @param eq: the element_queue returned by create
   */
  void (*destroy)(struct element_queue *eq, fp_spinlock_t *write_lock);
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDE_JITBUF_JLOG_OUTPUT_FACTORY_H_ */
