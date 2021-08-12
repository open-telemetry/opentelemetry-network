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
