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
