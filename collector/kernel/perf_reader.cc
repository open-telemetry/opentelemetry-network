// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/perf_reader.h>

#include <algorithm>
#include <linux/perf_event.h>
#include <linux/unistd.h>
#include <spdlog/fmt/fmt.h>
#include <sstream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <util/perf_ring.h>

#define __STRINGIZE(X) #X
#define _STRINGIZE(X) __STRINGIZE(X)

PerfContainer::PerfContainer() : n_entries_(0), readers_in_entries_(0) {}

void PerfContainer::add_ring(PerfRing &pr)
{
  if (readers_.size() >= BPF_MAX_CPUS)
    throw std::runtime_error("Only up to " _STRINGIZE(BPF_MAX_CPUS) " cpus are currently supported");

  readers_.push_back(pr);
}

void PerfContainer::add_data_ring(PerfRing &pr)
{

  if (data_readers_.size() >= BPF_MAX_CPUS)
    throw std::runtime_error("Only up to " _STRINGIZE(BPF_MAX_CPUS) " cpus are currently supported");

  data_readers_.push_back(pr);
}

void PerfContainer::set_callback(uv_loop_t &loop, void *ctx, CALLBACK cb)
{
  for (auto &reader : readers_) {
    reader.set_callback(loop, ctx, cb);
  }
  for (auto &data_reader : data_readers_) {
    data_reader.set_callback(loop, ctx, cb);
  }
}

std::string PerfContainer::inspect(void)
{
  std::string out;

  size_t num_readers = readers_.size();
  out += fmt::format("readers_: size={}\n", num_readers);
  for (size_t n = 0; n < readers_.size(); n++) {
    u32 total_bytes;
    u32 bytes = readers_[n].bytes_remaining(&total_bytes);
    out += fmt::format("  readers_[{}]: size={} ({}% full)\n", n, bytes, (((double)bytes) / (double)total_bytes) * 100.0);
  }
  out += fmt::format("entries_: n_entries_={}. readers_in_entries_={}\n", n_entries_, readers_in_entries_.to_string());
  for (u32 n = 0; n < n_entries_; n++) {
    out += fmt::format("  entries_[{}]={{timestamp {}, reader_index {}}}", n, entries_[n].timestamp, entries_[n].reader_index);
    n++;
  }
  return out;
}

PerfReader::PerfReader(PerfContainer &container, u64 max_timestamp)
    : container_(container), max_timestamp_(max_timestamp), active_(true)
{
  for (size_t i = 0; i < container.readers_.size(); i++) {
    auto &reader = container.readers_[i];
    reader.start_read_batch();

    auto &data_reader = container.data_readers_[i];
    data_reader.start_read_batch();

    /* if the reader is already in container.entries_, continue */
    if (container.readers_in_entries_.test(i))
      continue;

    /* might need to add to container_.entries_. do so if required */
    update_when_not_in_entries(i);
  }
}

PerfReader::~PerfReader()
{
  stop();
}

bool PerfReader::empty()
{
  return (container_.n_entries_ == 0) || (container_.entries_[0].timestamp > max_timestamp_);
}

void PerfReader::pop_unpadded_and_copy_to(char *dest)
{
  auto &reader = top();
  /* get length */
  u32 length = reader.peek_size();
  /* copy into buffer */
  reader.peek_copy(dest, 0, length);
  /* release the element */
  reader.pop();
  update_after_pop();
}

void PerfReader::pop_and_copy_to(char *dest)
{
  auto &reader = top();
  /* get unpadded length */
  u32 unpadded = reader.peek_aligned_u32(sizeof(u32));
  /* copy into buffer */
  reader.peek_copy(dest, sizeof(u64), unpadded);
  /* release the element */
  reader.pop();
  update_after_pop();
}

void PerfReader::pop()
{
  auto &reader = top();
  reader.pop();
  update_after_pop();
}

void PerfReader::stop()
{
  if (!active_)
    return;

  for (auto &reader : container_.readers_)
    reader.finish_read_batch();
  for (auto &data_reader : container_.data_readers_)
    data_reader.finish_read_batch();

  active_ = false;
}

void PerfReader::update_after_pop()
{
  /* pop the entry in container_.entries_ */
  size_t idx = container_.entries_[0].reader_index;
  std::pop_heap(&container_.entries_[0], &container_.entries_[container_.n_entries_]);
  container_.n_entries_--;
  container_.readers_in_entries_.reset(idx);

  update_when_not_in_entries(idx);
}

void PerfReader::update_when_not_in_entries(size_t idx)
{
  PerfRing &reader = container_.readers_[idx];

  /* if the reader is empty, no need to add it */
  int size = reader.peek_size();
  if (size == -ENOENT)
    return;

  u64 timestamp = 0ull; /* PERF_RECORD_LOST is inserted as the minimum timestamp so it comes out first */
  if (reader.peek_type() == PERF_RECORD_SAMPLE) {
    /* will have 8 bytes for sample record, 8 bytes timestamp */
    assert(size >= 16);
    timestamp = reader.peek_aligned_u64(sizeof(u64));
  }

  /* add the timestamp */
  container_.entries_[container_.n_entries_] = {.timestamp = timestamp, .reader_index = idx};
  container_.n_entries_++;
  std::push_heap(&container_.entries_[0], &container_.entries_[container_.n_entries_]);
  container_.readers_in_entries_.set(idx);
}
