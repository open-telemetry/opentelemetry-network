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

#include <util/meta.h>

#include <gtest/gtest.h>

#include <generated/flowmill/ingest/meta.h>

#include <jitbuf/jb.h>
#include <spdlog/fmt/fmt.h>

#include <type_traits>

#include <cstring>

namespace flowmill::ingest {

TEST(flowmill_metadata, message_metadata_rpc_id)
{
  EXPECT_EQ(301, pid_info_message_metadata::rpc_id);
  EXPECT_EQ(396, container_metadata_message_metadata::rpc_id);
}

TEST(flowmill_metadata, field_type)
{
  EXPECT_TRUE((std::is_same_v<std::uint32_t, pid_info_message_metadata::field_pid::type>));
  EXPECT_TRUE((std::is_same_v<std::uint8_t[16], pid_info_message_metadata::field_comm::type>));
}

TEST(flowmill_metadata, field_name)
{
  EXPECT_EQ("pid", pid_info_message_metadata::field_pid::name);
  EXPECT_EQ("comm", pid_info_message_metadata::field_comm::name);
}

TEST(flowmill_metadata, field_get)
{
  jsrv_ingest__pid_info message;
  message.pid = 12345;
  std::strncpy(reinterpret_cast<char *>(message.comm), "abcdef123456789", 16);

  EXPECT_EQ(12345u, pid_info_message_metadata::field_pid::get(&message));
  EXPECT_EQ(
      0,
      std::strncmp(
          "abcdef123456789", reinterpret_cast<char const *>(pid_info_message_metadata::field_comm::get(&message)), 16));
}

TEST(flowmill_metadata, message_metadata_for)
{
  EXPECT_TRUE((std::is_same_v<pid_info_message_metadata, ingest_metadata::message_metadata_for<jb_ingest__pid_info>>));
  EXPECT_TRUE((std::is_same_v<pid_info_message_metadata, ingest_metadata::message_metadata_for<jsrv_ingest__pid_info>>));
  EXPECT_TRUE((std::is_same_v<
               container_metadata_message_metadata,
               ingest_metadata::message_metadata_for<jb_ingest__container_metadata>>));
  EXPECT_TRUE((std::is_same_v<
               container_metadata_message_metadata,
               ingest_metadata::message_metadata_for<jsrv_ingest__container_metadata>>));
}

} // namespace flowmill::ingest

template <typename... T> void test_foreach()
{
  using list = meta::list<T...>;

  std::vector<std::string> const expected{std::initializer_list<std::string>{typeid(T).name()...}};

  std::vector<std::string> actual;
  meta::foreach<list>([&](auto tag) {
    using type = decltype(meta::tag_type(tag));
    actual.emplace_back(typeid(type).name());
  });

  EXPECT_EQ(sizeof...(T), actual.size());
  EXPECT_EQ(expected, actual);
}

TEST(meta, foreach)
{
  test_foreach<>();
  test_foreach<int>();
  test_foreach<std::string, int>();
  test_foreach<std::string, int, double>();
}
