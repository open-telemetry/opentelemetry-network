/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_FASTPASS_UTIL_PERF_RING_CPP_H_
#define INCLUDE_FASTPASS_UTIL_PERF_RING_CPP_H_

#include <linux/perf_event.h>
#include <linux/unistd.h>
#include <memory>
#include <platform/platform.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <util/perf_ring.h>
#include <uv.h>

class PerfRing;

/**
 * All the shared data for a memory-contiguous element queue
 */
class PerfRingStorage {
public:
  virtual ~PerfRingStorage() {}
  char *data() { return data_; }
  u32 n_data_pages() { return n_data_pages_; }
  u64 page_size() { return page_size_; }

  typedef void CALLBACK(void *ctx);
  virtual void set_callback(uv_loop_t &loop, void *ctx, CALLBACK cb) = 0;

protected:
  char *data_;
  u32 n_data_pages_;
  u64 page_size_;
};

/**
 * All the shared data for a memory-contiguous element queue
 */
class MmapPerfRingStorage : public PerfRingStorage {
public:
  /**
   * C'tor
   * @param n_pages: the number of allocated pages for the data queue
   * @param n_watermark_bytes: the number of bytes to wait to arrive before waking up the fd for reading when events are
   * available. special case is 0 = never wake up fd (poll only)
   */
  MmapPerfRingStorage(int cpu, u32 n_pages, u32 n_watermark_bytes);
  /**
   * Move c'tor, for resizing vector holding SinglePerfReader
   */
  MmapPerfRingStorage(MmapPerfRingStorage &&other);

  virtual ~MmapPerfRingStorage();

  virtual void set_callback(uv_loop_t &loop, void *ctx, CALLBACK cb);

  /**
   * Returns the file descriptor of the perf buffer
   */
  int fd() { return fd_; }

private:
  /* disallow copy and assignment */
  MmapPerfRingStorage(const MmapPerfRingStorage &) = delete;
  void operator=(const MmapPerfRingStorage &) = delete;

  int fd_;
  size_t mmap_size_;
  uv_poll_t mmap_poll_;
  void *callback_ctx_;
  CALLBACK *callback_;
  u32 n_watermark_bytes_;
};

class PerfRing : public perf_ring {
public:
  PerfRing(std::shared_ptr<PerfRingStorage> storage);

  /* @see pr_start_write_batch */
  void start_write_batch();

  /**
   * Writes the given string to the element queue
   * @throws range_error if queue is full,
   *    invalid_error if string is empty or trying to write more than eq size
   *
   * @see pr_write for more info
   */
  void write(std::string_view elem, u32 type);

  /* @see pr_finish_write_batch */
  void finish_write_batch();

  /* @see pr_start_read_batch */
  void start_read_batch();

  /* @see pr_peek_size */
  int peek_size() const;

  /* @see pr_peek_type */
  int peek_type() const;

  /* @see pr_peek_aligned_u64 */
  u64 peek_aligned_u64(u16 offset) const;

  /* @see pr_peek_aligned_u32 */
  u32 peek_aligned_u32(u16 offset) const;

  /* @see pr_peek_aligned_u32 */
  u16 peek_aligned_u16(u16 offset) const;

  /* @see pr_peek_copy */
  void peek_copy(char *buf, u16 offset, u16 len) const;

  /* @see pr_bytes_remaining */
  u32 bytes_remaining(u32 *total_bytes) const;

  std::pair<std::string_view, std::string_view> peek() const;

  /**
   * Reads an element from the element queue.
   * @return an element on success. throws otherwise
   *
   * @see pr_read for more info
   */
  std::string read();

  /**
   * Discards the next event in the queue
   */
  void pop();

  /* @see pr_finish_read_batch */
  void finish_read_batch();

  /**
   * Set a callback to execute when events show up in the perf ring
   * Used if you are not polling for high-speed access
   * Must be called only after all perf rings are added.
   */
  typedef void CALLBACK(void *ctx);
  void set_callback(uv_loop_t &loop, void *ctx, CALLBACK cb);

protected:
  /* the underlying storage backing the element queue */
  std::shared_ptr<PerfRingStorage> storage_;
};

/*****************
 * IMPLEMENTATION
 *****************/

inline MmapPerfRingStorage::MmapPerfRingStorage(int cpu, u32 n_bytes, u32 n_watermark_bytes)
    : callback_ctx_(nullptr), callback_(nullptr), n_watermark_bytes_(n_watermark_bytes)
{
  // calculate the number of pages required to store n_bytes
  page_size_ = getpagesize();
  u32 n_pages = ((n_bytes + page_size_ - 1) / page_size_);

  // create perf_event_open attributes
  struct perf_event_attr attr = {};
  attr.type = PERF_TYPE_SOFTWARE;
  attr.size = sizeof(struct perf_event_attr);
  attr.config = PERF_COUNT_SW_BPF_OUTPUT;
  attr.sample_period = 1;
  attr.sample_type = PERF_SAMPLE_RAW;
  attr.watermark = (n_watermark_bytes > 0) ? 1 : 0;
  attr.wakeup_watermark = n_watermark_bytes;

  fd_ = syscall(__NR_perf_event_open, &attr, -1, cpu, -1, PERF_FLAG_FD_CLOEXEC);
  if (fd_ == -1) {
    std::stringstream msg;
    msg << "perf_event_open on cpu " << cpu << " failed with errno " << errno << ", error: '" << strerror(errno) << "'";
    throw std::runtime_error(msg.str());
  }

  /* unistd.h to get page size */
  n_data_pages_ = n_pages;
  mmap_size_ = page_size_ * (1 + n_pages);
  data_ = (char *)mmap(NULL, mmap_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (data_ == MAP_FAILED) {
    close(fd_);

    std::stringstream msg;
    msg << "mmap on cpu " << cpu << " failed with errno " << errno << ", error: '" << strerror(errno) << "'";
    throw std::runtime_error(msg.str());
  }

  int res = ioctl(fd_, PERF_EVENT_IOC_ENABLE, 0);
  if (res != 0) {
    munmap(data_, mmap_size_);
    close(fd_);

    std::stringstream msg;
    msg << "ioctl PERF_EVENT_IOC_ENABLE on cpu " << cpu << " failed with return " << res << ", errno " << errno << ", error: '"
        << strerror(errno) << "'";
    throw std::runtime_error(msg.str());
  }
}

inline MmapPerfRingStorage::MmapPerfRingStorage(MmapPerfRingStorage &&other) : fd_(other.fd_)
{
  callback_ = other.callback_;
  other.callback_ = nullptr;
  callback_ctx_ = other.callback_ctx_;
  other.callback_ctx_ = nullptr;
  data_ = other.data_;
  other.data_ = NULL;
  mmap_size_ = other.mmap_size_;
  other.mmap_size_ = 0;
  n_data_pages_ = other.n_data_pages_;
  other.n_data_pages_ = 0;
  page_size_ = other.page_size_;
  other.page_size_ = 0;
}

inline MmapPerfRingStorage::~MmapPerfRingStorage()
{
  if (data_ != NULL) {
    munmap(data_, mmap_size_);
    close(fd_);
  }
  if (callback_) {
    uv_poll_stop(&mmap_poll_);
  }
}

inline void MmapPerfRingStorage::set_callback(uv_loop_t &loop, void *ctx, CALLBACK cb)
{
  callback_ = cb;
  callback_ctx_ = ctx;

  int res = uv_poll_init(&loop, &mmap_poll_, fd_);
  if (res != 0) {
    throw std::runtime_error("Could not init mmap_poll_");
  }

  uv_handle_set_data((uv_handle_t *)&mmap_poll_, this);

  res = uv_poll_start(&mmap_poll_, UV_READABLE, [](uv_poll_t *handle, int status, int events) {
    MmapPerfRingStorage *obj = (MmapPerfRingStorage *)uv_handle_get_data((uv_handle_t *)handle);
    (obj->callback_)(obj->callback_ctx_);
  });
  if (res != 0) {
    throw std::runtime_error("Could not start watching mmap_poll_");
  }
}

inline PerfRing::PerfRing(std::shared_ptr<PerfRingStorage> storage) : storage_(storage)
{
  int res = pr_init_contig(this, storage->data(), storage->n_data_pages(), storage->page_size());
  if (res != 0) {
    std::stringstream msg;
    msg << "pr_init_contig failed, ret=" << res << " storage->data()=" << (u64)storage->data();
    throw std::runtime_error(msg.str());
  }
}

inline void PerfRing::start_write_batch()
{
  pr_start_write_batch(this);
}

inline void PerfRing::write(std::string_view elem, u32 type)
{
  int offset = pr_write(this, elem.length(), type);
  if (offset == -EINVAL)
    throw std::invalid_argument("pr_write returned -EINVAL");
  if (offset == -ENOSPC)
    throw std::range_error("not enough space in element queue");
  if (offset < 0)
    throw std::runtime_error("unexpected return value");

  /* if we reached here, can write the element to offset */
  if (elem.length() == 0)
    return;

  int end = (offset + elem.length() - 1) & buf_mask;
  if ((end - offset) < 0) {
    /* wraps around */
    int len_to_ring_end = buf_mask + 1 - offset;
    memcpy(data + offset, elem.data(), len_to_ring_end);
    memcpy(data, elem.data() + len_to_ring_end, end + 1);
  } else {
    /* simple case */
    memcpy(data + offset, elem.data(), elem.length());
  }
}

inline void PerfRing::finish_write_batch()
{
  pr_finish_write_batch(this);
}

inline void PerfRing::start_read_batch()
{
  pr_start_read_batch(this);
}

inline int PerfRing::peek_size() const
{
  return pr_peek_size(this);
}

inline int PerfRing::peek_type() const
{
  return pr_peek_type(this);
}

inline u32 PerfRing::bytes_remaining(u32 *total_bytes) const
{
  return pr_bytes_remaining(this, total_bytes);
}

inline u64 PerfRing::peek_aligned_u64(u16 offset) const
{
  return pr_peek_aligned_u64(this, offset);
}

inline u32 PerfRing::peek_aligned_u32(u16 offset) const
{
  return pr_peek_aligned_u32(this, offset);
}

inline u16 PerfRing::peek_aligned_u16(u16 offset) const
{
  return pr_peek_aligned_u16(this, offset);
}

inline void PerfRing::peek_copy(char *buf, u16 offset, u16 len) const
{
  pr_peek_copy(this, buf, offset, len);
}

inline std::pair<std::string_view, std::string_view> PerfRing::peek() const
{
  auto const view = pr_peek(this);
  return std::make_pair(std::string_view(view.first, view.first_len), std::string_view(view.second, view.second_len));
}

inline std::string PerfRing::read()
{
  int size = peek_size();
  if (size == -ENOENT)
    throw std::out_of_range("queue empty");

  std::string ret(size, 0);
  peek_copy((char *)ret.data(), 0, size);

  pop();

  return ret;
}

inline void PerfRing::pop()
{
  u16 len;
  [[maybe_unused]] int offset = pr_read(this, &len);
  assert(offset >= 0);
}

inline void PerfRing::finish_read_batch()
{
  pr_finish_read_batch(this);
}

inline void PerfRing::set_callback(uv_loop_t &loop, void *ctx, CALLBACK cb)
{
  storage_->set_callback(loop, ctx, cb);
}

#endif /* INCLUDE_FASTPASS_UTIL_PERF_RING_CPP_H_ */
