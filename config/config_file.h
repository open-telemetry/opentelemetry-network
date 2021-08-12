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
