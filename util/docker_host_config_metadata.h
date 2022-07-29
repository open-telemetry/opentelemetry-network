/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>

/**
 * Reads metadata posted by Kubernetes to the Docker engine.
 */
class DockerHostConfigMetadata {
public:
  // `host_config` is the `.HostConfig` section from the equivalent of `docker inspect`
  explicit DockerHostConfigMetadata(nlohmann::json const &host_config);

  auto cpu_shares() const { return cpu_shares_; }
  auto cpu_period() const { return cpu_period_; }
  auto cpu_quota() const { return cpu_quota_; }

  auto memory_swappiness() const { return memory_swappiness_; }
  auto memory_limit() const { return memory_limit_; }
  auto memory_soft_limit() const { return memory_soft_limit_; }
  auto total_memory_limit() const { return total_memory_limit_; }

  explicit operator bool() const;

  template <typename Out> friend Out &&operator<<(Out &&out, DockerHostConfigMetadata const &what);

private:
  std::uint16_t cpu_shares_;
  std::int32_t cpu_period_;
  std::int32_t cpu_quota_;

  std::uint8_t memory_swappiness_;
  std::uint64_t memory_limit_;
  std::uint64_t memory_soft_limit_;
  std::int64_t total_memory_limit_; // including swap
};

#include <util/docker_host_config_metadata.inl>
