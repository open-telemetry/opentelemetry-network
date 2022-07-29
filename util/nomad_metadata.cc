// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/nomad_metadata.h>

#include <collector/agent_log.h>
#include <util/environment_variables.h>
#include <util/json.h>
#include <util/log.h>
#include <util/log_formatters.h>

#include <absl/strings/match.h>

const std::string NOMAD_NAMESPACE_VAR = "NOMAD_NAMESPACE";
const std::string NOMAD_GROUP_NAME_VAR = "NOMAD_GROUP_NAME";
const std::string NOMAD_TASK_NAME_VAR = "NOMAD_TASK_NAME";
const std::string NOMAD_JOB_NAME_VAR = "NOMAD_JOB_NAME";

const std::string NOMAD_NAMESPACE_VAR_PREFIX = "NOMAD_NAMESPACE=";
const std::string NOMAD_GROUP_NAME_VAR_PREFIX = "NOMAD_GROUP_NAME=";
const std::string NOMAD_TASK_NAME_VAR_PREFIX = "NOMAD_TASK_NAME=";
const std::string NOMAD_JOB_NAME_VAR_PREFIX = "NOMAD_JOB_NAME=";

NomadMetadata::NomadMetadata()
    : ns_(try_get_env_var(NOMAD_NAMESPACE_VAR.c_str())),
      group_name_(try_get_env_var(NOMAD_GROUP_NAME_VAR.c_str())),
      task_name_(try_get_env_var(NOMAD_TASK_NAME_VAR.c_str())),
      job_name_(try_get_env_var(NOMAD_JOB_NAME_VAR.c_str()))
{}

NomadMetadata::NomadMetadata(nlohmann::json const &environment)
{
  LOG::trace_in(AgentLogKind::NOMAD, "Container environment: {}", environment);

  for (auto const &variable : environment) {
    if (auto string = try_get_string(variable)) {
      if (absl::StartsWith(*string, NOMAD_NAMESPACE_VAR_PREFIX)) {
        ns_ = string->substr(NOMAD_NAMESPACE_VAR_PREFIX.size());
      } else if (absl::StartsWith(*string, NOMAD_GROUP_NAME_VAR_PREFIX)) {
        group_name_ = string->substr(NOMAD_GROUP_NAME_VAR_PREFIX.size());
      } else if (absl::StartsWith(*string, NOMAD_TASK_NAME_VAR_PREFIX)) {
        task_name_ = string->substr(NOMAD_TASK_NAME_VAR_PREFIX.size());
      } else if (absl::StartsWith(*string, NOMAD_JOB_NAME_VAR_PREFIX)) {
        job_name_ = string->substr(NOMAD_JOB_NAME_VAR_PREFIX.size());
      }
    }
  }

  LOG::trace_in(
      AgentLogKind::NOMAD, "Nomad metadata: ns='{}' group='{}' job='{}' task='{}'", ns_, group_name_, job_name_, task_name_);
}

void NomadMetadata::print() const
{
  LOG::debug("Nomad metadata:");
  LOG::debug("- namespace: {}", ns_);
  LOG::debug("- group name: {}", group_name_);
  LOG::debug("- task name: {}", task_name_);
  LOG::debug("- job name: {}", job_name_);
}

NomadMetadata::operator bool() const
{
  return !ns_.empty() || !group_name_.empty() || !task_name_.empty() || !job_name_.empty();
}
