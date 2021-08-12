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

#ifndef INCLUDE_JITBUF_ELEMENT_QUEUE_JLOG_FACTORY_H_
#define INCLUDE_JITBUF_ELEMENT_QUEUE_JLOG_FACTORY_H_

#include <jitbuf/jlog.h>
#include <platform/spin_lock.h>
#include <util/element_queue_cpp.h>
#include <vector>

class FactoryElementQueue {
public:
  FactoryElementQueue(std::shared_ptr<ElementQueueStorage> storage, struct element_queue_jlog_output_factory *factory);

  virtual ~FactoryElementQueue();

  ElementQueue producer;
  fp_spinlock_t write_lock;
  ElementQueue consumer;

private:
  struct element_queue_jlog_output_factory *factory_;
};

struct element_queue_jlog_output_factory {
  struct jlog_output_factory jlog_factory;
  std::vector<FactoryElementQueue *> queues;
  u32 n_elems_;
  u32 buf_len_;
};

class ElementQueueJlogFactory : public element_queue_jlog_output_factory {
public:
  /**
   * C'tor
   * @param n_elems: the number of elements the element queue will hold
   * @param buf_len: the size of buffer for data storage
   */
  ElementQueueJlogFactory(u32 n_elems, u32 buf_len);

  struct jlog_output_factory *get_factory() { return &jlog_factory; }

  int n_queues() { return queues.size(); }
  ElementQueue &consumer(int i) { return queues[i]->consumer; }
};

#endif /* INCLUDE_JITBUF_ELEMENT_QUEUE_JLOG_FACTORY_H_ */
