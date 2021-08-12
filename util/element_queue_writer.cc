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

#include "element_queue_writer.h"

#include <util/log.h>

#include <unistd.h>

namespace {

static constexpr size_t RETRY_BACKOFF_RATIO = 2;
static constexpr size_t RETRY_BACKOFF_LIMIT = 512;
static constexpr useconds_t RETRY_INTERVAL = 1000; // in microseconds

} // namespace

ElementQueueWriter::ElementQueueWriter(ElementQueue &queue) : queue_(queue) {}

ElementQueueWriter::~ElementQueueWriter() {}

Expected<u8 *, std::error_code> ElementQueueWriter::start_write(u32 length)
{
  queue_.start_write_batch();

  size_t backoff = 1;
  int offset = -EINVAL;

  do {
    offset = eq_write(&queue_, length);

    if (offset == -ENOSPC) {
      // sleep util there's space to write
      queue_.finish_write_batch();
      usleep(backoff * RETRY_INTERVAL);
      queue_.start_write_batch();

      // increase backoff geometrically
      backoff *= RETRY_BACKOFF_RATIO;

      if (backoff > RETRY_BACKOFF_LIMIT) {
        // clamp to limit and log a warning
        backoff = RETRY_BACKOFF_LIMIT;
        LOG::warn("ElementQueueWriter: queue full, backing off");
      }

      ++num_write_stalls_;
    }
  } while (offset == -ENOSPC);

  if (offset < 0) {
    return {unexpected, -offset, std::generic_category()};
  }

  return reinterpret_cast<u8 *>(queue_.data + offset);
}

void ElementQueueWriter::finish_write()
{
  queue_.finish_write_batch();
}

std::error_code ElementQueueWriter::flush()
{
  // TODO
  return {};
}

u32 ElementQueueWriter::buf_size() const
{
  // TODO
  return 0;
}
