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

#ifndef INCLUDE_FASTPASS_UTIL_ELEMENT_QUEUE_CPP_H_
#define INCLUDE_FASTPASS_UTIL_ELEMENT_QUEUE_CPP_H_

#include <util/element_queue.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

class ElementQueue;

/**
 * All the shared data for a memory-contiguous element queue
 */
class ElementQueueStorage {
public:
  /**
   * C'tor
   * @param n_elems: the number of members in the circular queue holding
   *   element sizes. Must be a power of 2
   * @param buf_len: the number of bytes that can hold element data. Must be
   *   a power of 2.
   */
  ElementQueueStorage(u32 n_elems, u32 buf_len)
      : n_elems_(n_elems), buf_len_(buf_len), data_(NULL)
  {
  }

  virtual ~ElementQueueStorage() {}

  u32 n_elems() { return n_elems_; }
  u32 buf_len() { return buf_len_; }
  char *data() { return data_; }

protected:
  u32 n_elems_;
  u32 buf_len_;
  char *data_;
};

typedef std::shared_ptr<ElementQueueStorage> ElementQueueStoragePtr;

/**
 * All the shared data for a memory-contiguous element queue
 */
class MemElementQueueStorage : public ElementQueueStorage {
public:
  /**
   * C'tor
   * @param n_elems: the number of members in the circular queue holding
   *   element sizes. Must be a power of 2
   * @param buf_len: the number of bytes that can hold element data. Must be
   *   a power of 2.
   */
  MemElementQueueStorage(u32 n_elems, u32 buf_len);

  virtual ~MemElementQueueStorage();
};

class ElementQueue : public element_queue {
public:
  ElementQueue(const ElementQueueStoragePtr &storage);

  /* @see eq_start_write_batch */
  void start_write_batch();

  /**
   * Writes the given string to the element queue
   *
   * @returns 0 on success, negative on error
   * @see eq_write for more info
   */
  int write(const std::string &elem);

  /**
   * Writes the contents of the given stringstream to the element queue
   *
   * @returns 0 on success, negative on error
   * @see eq_write for more info
   */
  int write(std::stringstream &ss);

  /* @see eq_finish_write_batch */
  void finish_write_batch();

  /* @see eq_start_read_batch */
  void start_read_batch();

  /* @see eq_peek */
  int peek();

  /* @see eq_peek_offset */
  int peek(char *&output);

  /**
   * Peeks a value of the specified type.
   * @returns the size of that value on success;
   *   -ENOENT if no element exists;
   *   -EINVAL if there is not enough data for that type
   */
  template <typename T>
  int peek_value(T &output);

  /**
   * Reads an element from the element queue.
   * @return an element on success. throws otherwise
   *
   * @see eq_read for more info
   */
  std::string read();

  /**
   * Reads an element from the element queue
   *
   * @returns number of bytes to read on success, like eq_read on error
   * @param output: [out] a pointer to the read buffer.
   *
   * @assumes: length of element can fit in an int (<= 1 << 31). there is an
   *   assert for this, but not a runtime check (when NDEBUG is not set)
   */
  int read(char *&output);

  /* @see eq_finish_read_batch */
  void finish_read_batch();

  /**
   * Moves an element from @from to this
   * @see eq_move
   * @returns see eq_move
   */
  int move_from(ElementQueue *from);

  /**
   * Gets the number of elements in the queue.
   */
  u32 elem_count() const;

  /**
   * Gets the element capacity of the queue.
   */
  u32 elem_capacity() const;

  /**
   * Gets the number of bytes used in the buffer.
   */
  u32 buf_used() const;

  /**
   * Gets the buffer capacity.
   */
  u32 buf_capacity() const;

protected:
  /* the underlying storage backing the element queue */
  ElementQueueStoragePtr storage_;
};

/*****************
 * IMPLEMENTATION
 *****************/

inline MemElementQueueStorage::MemElementQueueStorage(u32 n_elems, u32 buf_len)
    : ElementQueueStorage(n_elems, buf_len)
{
  u32 size = eq_contig_size(n_elems, buf_len);

  /* allocate contig memory */
  data_ = (char *)malloc(size);
  if (data_ == NULL)
    throw std::runtime_error("Unable to allocate memory for element queue");

  /* zero it out */
  memset(data_, 0, size);

  /* Initialize shared structures */
  eq_init_shared((element_queue_shared *)data_);
}

inline MemElementQueueStorage::~MemElementQueueStorage()
{
  if (data_)
    free(data_);
}

inline ElementQueue::ElementQueue(const ElementQueueStoragePtr &storage)
    : storage_(storage)
{
  int res = eq_init_contig(this, storage->n_elems(), storage->buf_len(),
                           storage->data());
  if (res != 0)
    throw std::runtime_error("eq_init_contig failed");
}

inline void ElementQueue::start_write_batch()
{
  eq_start_write_batch(this);
}

inline int ElementQueue::write(const std::string &elem)
{
  int offset = eq_write(this, elem.length());
  if (offset < 0)
    return offset;

  /* if we reached here, can write the element to offset */
  memcpy(data + offset, elem.data(), elem.length());

  return 0;
}

inline int ElementQueue::write(std::stringstream &ss)
{
  u32 len = ss.tellp();
  int offset = eq_write(this, len);
  if (offset < 0)
    return offset;

  ss.read(data + offset, len);
  return 0;
}

inline void ElementQueue::finish_write_batch()
{
  eq_finish_write_batch(this);
}

inline void ElementQueue::start_read_batch()
{
  eq_start_read_batch(this);
}

inline int ElementQueue::peek()
{
  return eq_peek(this);
}

inline int ElementQueue::peek(char *&output)
{
  u32 len;
  int offset = eq_peek_offset(this, &len);

  if (offset < 0) {
    return offset;
  }

  output = data + offset;
  return len;
}

template <typename T>
int ElementQueue::peek_value(T &output)
{
  u32 len;
  int offset = eq_peek_offset(this, &len);

  if (offset < 0) {
    return offset;
  }

  if (len < sizeof(T)) {
    return -EINVAL;
  }

  output = *reinterpret_cast<T *>(data + offset);
  return len;
}

inline std::string ElementQueue::read()
{
  u32 len;

  int offset = eq_read(this, &len);
  if (offset == -EINVAL)
    throw std::invalid_argument("eq_read returned -EINVAL");
  if (offset == -EAGAIN)
    throw std::out_of_range("queue empty");
  if (offset < 0)
    throw std::runtime_error("unexpected return value");

  /* can copy data into string */
  return std::string(data + offset, len);
}

inline int ElementQueue::read(char *&output)
{
  u32 len;
  int offset = eq_read(this, &len);

  if (offset < 0)
    return offset;

  assert((u32)((int)len) == len);
  output = data + offset;
  return len;
}

inline void ElementQueue::finish_read_batch()
{
  eq_finish_read_batch(this);
}

inline int ElementQueue::move_from(ElementQueue *from)
{
  return eq_move(this, from);
}

inline u32 ElementQueue::elem_count() const
{
  return eq_elem_count(this);
}

inline u32 ElementQueue::elem_capacity() const
{
  return eq_elem_capacity(this);
}

inline u32 ElementQueue::buf_used() const
{
  return eq_buf_used(this);
}

inline u32 ElementQueue::buf_capacity() const
{
  return eq_buf_capacity(this);
}

#endif /* INCLUDE_FASTPASS_UTIL_ELEMENT_QUEUE_CPP_H_ */
