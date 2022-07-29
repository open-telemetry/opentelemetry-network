/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <string>

namespace config {

class ConfigFile {
public:
  struct YamlFormat {
  };
  enum class FailMode { silent, exception };

  ConfigFile(YamlFormat, std::string const &path, FailMode fail = FailMode::exception);

  using LabelsMap = std::map<std::string, std::string>;
  LabelsMap const &labels() const { return labels_; }
  LabelsMap &labels() { return labels_; }

private:
  LabelsMap labels_;
};

} // namespace config
