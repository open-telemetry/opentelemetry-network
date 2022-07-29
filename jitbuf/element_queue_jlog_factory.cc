// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <jitbuf/element_queue_jlog_factory.h>

#include <algorithm>
#include <ccan/container_of/container_of.h>

FactoryElementQueue::FactoryElementQueue(
    std::shared_ptr<ElementQueueStorage> storage, struct element_queue_jlog_output_factory *factory)
    : producer(storage), consumer(storage), factory_(factory)
{
  int ret;

  ret = fp_spin_init(&write_lock);
  if (ret != 0)
    throw std::runtime_error("FactoryElementQueue: cannot initialize lock");

  factory->queues.push_back(this);
}

FactoryElementQueue::~FactoryElementQueue()
{
  auto iter = std::find(factory_->queues.begin(), factory_->queues.end(), this);

  if (iter == factory_->queues.end())
    throw std::runtime_error("could not find element_queue in factory's queues when destroying");

  factory_->queues.erase(iter);

  fp_spin_destroy(&write_lock);
}

/* create method from jlog_output_factory */
static int output_create(struct jlog_output_factory *factory, struct element_queue **eq_ptr, fp_spinlock_t **write_lock_ptr)
{
  struct element_queue_jlog_output_factory *eq_factory = container_of(factory, typeof(*eq_factory), jlog_factory);

  try {
    std::shared_ptr<MemElementQueueStorage> storage(new MemElementQueueStorage(eq_factory->n_elems_, eq_factory->buf_len_));

    FactoryElementQueue *queue = new FactoryElementQueue(storage, eq_factory);
    if (queue == NULL)
      return -ENOMEM;

    *eq_ptr = &queue->producer;
    *write_lock_ptr = &queue->write_lock;

    return 0;
  } catch (...) {
    return -ENOMEM;
  }
}

/* destroy method from jlog_output_factory */
void output_destroy(struct element_queue *eq, fp_spinlock_t *write_lock)
{
  FactoryElementQueue *eq_cpp = container_of(eq, FactoryElementQueue, producer);

  delete eq_cpp;
}

ElementQueueJlogFactory::ElementQueueJlogFactory(u32 n_elems, u32 buf_len)
{
  n_elems_ = n_elems;
  buf_len_ = buf_len;
  jlog_factory.create = output_create;
  jlog_factory.destroy = output_destroy;
}
