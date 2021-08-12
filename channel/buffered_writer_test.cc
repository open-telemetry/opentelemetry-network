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
