// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

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
