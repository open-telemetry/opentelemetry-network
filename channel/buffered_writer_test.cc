// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <channel/buffered_writer.h>

#include <memory>

#include <channel/mock_channel.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {
using ::testing::_;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::Test;

static constexpr u32 default_buffer_size = 32;

class BufferedWriterTest : public Test {
protected:
  void SetUp() override { writer_.reset(new channel::BufferedWriter(mock_channel_, default_buffer_size)); }

  ::channel::MockChannel mock_channel_;
  std::unique_ptr<channel::BufferedWriter> writer_;
};

TEST_F(BufferedWriterTest, empty_writer)
{
  EXPECT_CALL(mock_channel_, send(_, _)).Times(0);

  EXPECT_EQ(writer_->buf_size(), default_buffer_size);
}

TEST_F(BufferedWriterTest, one_flush)
{
  ON_CALL(mock_channel_, is_open()).WillByDefault(Return(true));
  EXPECT_CALL(mock_channel_, send(_, 24)).Times(1);

  EXPECT_THAT(writer_->start_write(24), IsTrue());
  EXPECT_THAT(*writer_->start_write(24), NotNull());
  writer_->finish_write();

  EXPECT_THAT(writer_->flush(), IsFalse());
}

} // namespace
