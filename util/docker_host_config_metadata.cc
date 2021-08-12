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

#include <util/docker_host_config_metadata.h>

#include <util/json.h>

namespace {
static std::string const CPU_SHARES = "CpuShares";
static std::string const CPU_PERIOD = "CpuPeriod";
static std::string const CPU_QUOTA = "CpuQuota";

static std::string const MEMORY_LIMIT = "Memory";
static std::string const MEMORY_SOFT_LIMIT = "MemoryReservation";
static std::string const TOTAL_MEMORY_LIMIT = "MemorySwap";
static std::string const MEMORY_SWAPPINESS = "MemorySwappiness";
} // namespace

DockerHostConfigMetadata::DockerHostConfigMetadata(nlohmann::json const &host_config)
    : cpu_shares_(try_get_int<decltype(cpu_shares_)>(host_config, CPU_SHARES).value_or(0)),
      cpu_period_(try_get_int<decltype(cpu_period_)>(host_config, CPU_PERIOD).value_or(0)),
      cpu_quota_(try_get_int<decltype(cpu_quota_)>(host_config, CPU_QUOTA).value_or(0)),
      memory_swappiness_(try_get_int<decltype(memory_swappiness_)>(host_config, MEMORY_LIMIT).value_or(0)),
      memory_limit_(try_get_int<decltype(memory_limit_)>(host_config, MEMORY_SOFT_LIMIT).value_or(0)),
      memory_soft_limit_(try_get_int<decltype(memory_soft_limit_)>(host_config, TOTAL_MEMORY_LIMIT).value_or(0)),
      total_memory_limit_(try_get_int<decltype(total_memory_limit_)>(host_config, MEMORY_SWAPPINESS).value_or(0))
{}

DockerHostConfigMetadata::operator bool() const
{
  return cpu_shares_ || cpu_period_ || cpu_quota_ || memory_swappiness_ || memory_limit_ || memory_soft_limit_ ||
         total_memory_limit_;
}
