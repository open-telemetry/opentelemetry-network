/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

/**
 * Reads metadata posted by Nomad (https://www.nomadproject.io/).
 *
 * NOTE: this function reads environment variables so it's advisable to call it
 * before any thread is created, given that reading/writing to environment
 * variables is not thread safe.
 */
class NomadMetadata {
public:
  NomadMetadata();
  NomadMetadata(nlohmann::json const &environment);

  NomadMetadata(NomadMetadata &&) = default;

  std::string_view ns() const { return ns_; }
  std::string_view group_name() const { return group_name_; }
  std::string_view task_name() const { return task_name_; }
  std::string_view job_name() const { return job_name_; }

  void print() const;

  explicit operator bool() const;

private:
  std::string ns_;
  std::string group_name_;
  std::string task_name_;
  std::string job_name_;
};
