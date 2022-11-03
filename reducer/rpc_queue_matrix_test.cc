// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/rpc_queue_matrix.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

namespace reducer {
namespace {

using ::testing::NotNull;

class Writer {
public:
  Writer(IBufferedWriter &buffered_writer) : buffered_writer_(buffered_writer) {}

  std::error_code write(char const *buf, size_t len)
  {
    auto buffer = buffered_writer_.start_write(len);
    if (!buffer) {
      return buffer.error();
    }

    EXPECT_THAT(*buffer, NotNull());
    memcpy(*buffer, buf, len);
    buffered_writer_.finish_write();
    return {};
  }

private:
  IBufferedWriter &buffered_writer_;
};

std::string make_msg(int x, int y)
{
  return std::to_string(x) + ":" + std::to_string(y);
}

TEST(RpcQueueMatrixTest, TestConstruction)
{
  size_t const num_senders = 3;
  size_t const num_receivers = 2;

  RpcQueueMatrix queues(num_senders, num_receivers);

  for (size_t r = 0; r < num_receivers; ++r) {
    std::vector<ElementQueue> readers = queues.make_readers(r);

    EXPECT_EQ(readers.size(), num_senders);

    for (auto &reader : readers) {
      reader.start_read_batch();

      EXPECT_EQ(reader.peek(), -ENOENT);

      reader.finish_read_batch();
    }
  }
}

TEST(RpcQueueMatrixTest, TestMessaging)
{
  size_t const num_senders = 3;
  size_t const num_receivers = 2;

  RpcQueueMatrix queues(num_senders, num_receivers);

  for (size_t s = 0; s < num_senders; ++s) {
    std::vector<Writer> writers = queues.make_writers<Writer>(s);

    EXPECT_EQ(writers.size(), num_receivers);

    for (size_t r = 0; r < num_receivers; ++r) {
      Writer &writer = writers[r];
      std::string msg = make_msg(s, r);
      writer.write(msg.c_str(), msg.size());
    }
  }

  for (size_t r = 0; r < num_receivers; ++r) {
    std::vector<ElementQueue> readers = queues.make_readers(r);

    EXPECT_EQ(readers.size(), num_senders);

    for (size_t s = 0; s < num_senders; ++s) {
      ElementQueue &reader = readers[s];
      reader.start_read_batch();

      EXPECT_TRUE(reader.peek() > 0);

      char *buf = nullptr;
      int len = reader.read(buf);

      EXPECT_THAT(buf, NotNull());

      std::string sent = make_msg(s, r);
      std::string received = std::string(buf, len);

      EXPECT_EQ(received, sent);

      reader.finish_read_batch();
    }
  }
}

} // namespace
} // namespace reducer
