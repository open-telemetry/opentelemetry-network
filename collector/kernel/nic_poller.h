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

#pragma once

#include <absl/container/flat_hash_map.h>
#include <chrono>

#include <generated/flowmill/agent_internal.wire_message.h>
#include <generated/flowmill/ingest/writer.h>
#include <util/logger.h>

//#define DEBUG_NIC_STATS 1

class NicPoller {
public:
  NicPoller(::flowmill::ingest::Writer &writer, logging::Logger &log);

  void handle_nic_queue_state(u64 timestamp, jb_agent_internal__nic_queue_state *msg);

  void poll();

private:
  struct if_map_value {
    u64 tx_bytes_total = 0;
    u64 rx_bytes_total = 0;
    u64 time_busy_ns = 0;
  };

  ::flowmill::ingest::Writer &writer_;
  logging::Logger &log_;
  std::chrono::time_point<std::chrono::high_resolution_clock> last_timestamp_;
  absl::flat_hash_map<std::string, if_map_value> if_map_;
};
