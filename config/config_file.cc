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

#include <config/config_file.h>

#include <util/log.h>
#include <yaml-cpp/yaml.h>

namespace config {

ConfigFile::ConfigFile(YamlFormat, std::string const &path, FailMode fail)
{
  if (path.empty()) {
    return;
  }

  try {
    YAML::Node yaml = YAML::LoadFile(path);

    if (auto labels = yaml["labels"]) {
      if (!labels.IsMap() && !labels.IsNull()) {
        LOG::warn("Ignoring 'labels' in config file: 'labels' should be a map.");
      } else {
        for (auto const &i : labels) {
          auto key = i.first.as<std::string>();
          auto value = i.second.as<std::string>();

          if ((key.length() > 20) || (value.length() > 40)) {
            LOG::warn(
                "Ignoring label '{}': '{}'. "
                "key and value lengths must be max 20, 40 chars.",
                key,
                value);
            continue;
          }

          LOG::info("Node label has been set in config: '{}':'{}'", key, value);
          labels_[std::move(key)] = std::move(value);
        }
      }
    } else {
      LOG::info("No \"labels\" were specified.");
    }
  } catch (std::exception const &e) {
    LOG::error("Config file at {} could not be loaded.", path);

    if (fail == FailMode::exception) {
      throw e;
    }
  }
}

} // namespace config
