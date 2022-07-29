// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
