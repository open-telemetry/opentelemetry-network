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

#include "nic_poller.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

constexpr std::string_view LOCAL_LOOPBACK_INTERFACE_NAME = "lo";
constexpr std::string_view LOCAL_CALI_INTERFACE_NAME = "cali";
constexpr double NIC_POLL_INTERVAL_SEC = 5;

NicPoller::NicPoller(::flowmill::ingest::Writer &writer, logging::Logger &log)
    : writer_(writer), log_(log), last_timestamp_(std::chrono::high_resolution_clock::now())
{}

void NicPoller::poll()
{
  auto timestamp = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> dur = timestamp - last_timestamp_;
  if (dur.count() < NIC_POLL_INTERVAL_SEC)
    return;
  last_timestamp_ = timestamp;

  for (auto &p : fs::directory_iterator("/sys/class/net")) {
    if (!p.is_directory())
      continue;

    std::string if_name = p.path().filename();
    if (if_name == LOCAL_LOOPBACK_INTERFACE_NAME) // skip loopback interface
      continue;
    if (if_name.substr(0, 4) == LOCAL_CALI_INTERFACE_NAME) // skip virtual calico interfaces
      continue;
    if (!fs::exists(p.path() / "statistics/tx_bytes") && !fs::exists(p.path() / "statistics/rx_bytes"))
      continue;

    u64 tx_bytes_total = 0;
    u64 rx_bytes_total = 0;
    u64 tx_bytes_rate = 0;
    u64 rx_bytes_rate = 0;
    u64 tx_errors = 0;
    u64 rx_errors = 0;

    std::fstream tx_bytes_fs(p.path() / "statistics/tx_bytes", std::ios::in);
    if (tx_bytes_fs)
      tx_bytes_fs >> tx_bytes_total;

    std::fstream rx_bytes_fs(p.path() / "statistics/rx_bytes", std::ios::in);
    if (rx_bytes_fs)
      rx_bytes_fs >> rx_bytes_total;

    std::fstream tx_errors_fs(p.path() / "statistics/tx_errors", std::ios::in);
    if (tx_errors_fs)
      tx_errors_fs >> tx_errors;

    std::fstream rx_errors_fs(p.path() / "statistics/rx_errors", std::ios::in);
    if (rx_errors_fs)
      rx_errors_fs >> rx_errors;

    auto it = if_map_.find(if_name);
    if (it == if_map_.end())
      it = if_map_.insert({if_name, {}}).first;

    tx_bytes_rate = (u64)((double)(tx_bytes_total - it->second.tx_bytes_total) / dur.count());
    rx_bytes_rate = (u64)((double)(rx_bytes_total - it->second.rx_bytes_total) / dur.count());
    it->second.tx_bytes_total = tx_bytes_total;
    it->second.rx_bytes_total = rx_bytes_total;

    auto dur_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur);
    long long int time_busy_ns = it->second.time_busy_ns;
    long long int time_free_ns = 0;
    if (dur_ns.count() > time_busy_ns)
      time_free_ns = dur_ns.count() - time_busy_ns;
    else
      time_busy_ns = dur_ns.count();
    it->second.time_busy_ns = 0;

#if DEBUG_NIC_STATS
    log_.info(
        "if_name {}, tx_total {}, rx_total {}, tx_rate {}, rx_rate {}, busy_ns {}, free_ns {}",
        if_name,
        tx_bytes_total,
        rx_bytes_total,
        tx_bytes_rate,
        rx_bytes_rate,
        time_busy_ns,
        time_free_ns);
#endif
    writer_.nic_stats(
        jb_blob{if_name},
        tx_bytes_total,
        rx_bytes_total,
        tx_bytes_rate,
        rx_bytes_rate,
        tx_errors,
        rx_errors,
        time_busy_ns,
        time_free_ns);
  }
}

void NicPoller::handle_nic_queue_state(u64 timestamp, jb_agent_internal__nic_queue_state *msg)
{
  std::string if_name{(char const *)msg->if_name, strnlen((char const *)msg->if_name, sizeof(msg->if_name))};

  if (if_name.empty() || if_name == LOCAL_LOOPBACK_INTERFACE_NAME) // skip loopback interface
    return;

  auto it = if_map_.find(if_name);
  if (it == if_map_.end())
    it = if_map_.insert({if_name, {}}).first;

  it->second.time_busy_ns += msg->busy_ns;
}
