/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <config/intake_config.h>

#include <map>
#include <string>

namespace config {

class ConfigFile {
public:
  struct YamlFormat {};
  enum class FailMode { silent, exception };

  ConfigFile(YamlFormat, std::string const &path, FailMode fail = FailMode::exception);

  using LabelsMap = std::map<std::string, std::string>;
  LabelsMap const &labels() const { return labels_; }
  LabelsMap &labels() { return labels_; }

  IntakeConfig const &intake_config() const { return intake_config_; }

private:
  LabelsMap labels_;
  IntakeConfig intake_config_;
};

} // namespace config
