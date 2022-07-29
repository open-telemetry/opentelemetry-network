// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "resync_queue.h"

#include <string.h>
#include <unistd.h>
#include <vector>

#include "resync_channel.h"
#include "util/log.h"

namespace collector {

ResyncQueue::ResyncQueue()
    : element_queue_storage_(new MemElementQueueStorage(queue_num_elements_, queue_buffer_size_)),
      read_queue_(element_queue_storage_),
      write_queue_(element_queue_storage_),
      last_resync_(0)
{}

ResyncQueue::~ResyncQueue() {}

u64 ResyncQueue::consumer_get_last_resync() const
{
  std::lock_guard<std::recursive_mutex> lock(mutex_);

  return last_resync_;
}

ElementQueue *ResyncQueue::consumer_get_queue()
{
  return &read_queue_;
}

void ResyncQueue::consumer_reset()
{
  // Note that this function runs on libuv loop. Fork a new thread to
  // do the reset instead if performance become an issue.
  {
    std::vector<ResyncChannel *> chs;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto *ch : channels_) {
      chs.push_back(ch);
    }

    for (auto *ch : chs) {
      ch->reset();
    }
  }
}

std::unique_ptr<ResyncChannel> ResyncQueue::new_channel(std::function<void(void)> &reset_callback)
{
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  last_resync_ += 1;

  auto *ch = new ResyncChannel(last_resync_, this, reset_callback);
  channels_.insert(ch);

  return std::unique_ptr<ResyncChannel>(ch);
}

void ResyncQueue::producer_unregister(ResyncChannel *channel)
{
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  channels_.erase(channel);
}

std::error_code ResyncQueue::producer_send(const u8 *data, int data_len, const u64 resync)
{
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  write_queue_.start_write_batch();
  int offset = eq_write(&write_queue_, data_len + 8);
  if (offset < 0) {
    LOG::warn("element queue is full\n");
    write_queue_.finish_write_batch();
    return std::make_error_code(std::errc::no_buffer_space);
  }

  memcpy((void *)(write_queue_.data + offset), &resync, 8);
  memcpy((void *)(write_queue_.data + offset + 8), data, data_len);
  write_queue_.finish_write_batch();
  return {};
}

}; // namespace collector
