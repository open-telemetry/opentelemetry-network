/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

struct DockerImageMetadata {
  static constexpr char VERSION_DELIMITER{':'};
  static constexpr char IMAGE_DELIMITER{'/'};
  static constexpr char CHECKSUM_DELIMITER{'@'};

  explicit constexpr DockerImageMetadata(std::string_view image);

  constexpr std::string_view registry() const { return registry_; }
  constexpr std::string_view name() const { return name_; }
  constexpr std::string_view version() const { return version_; }

private:
  std::string_view registry_;
  std::string_view name_;
  std::string_view version_;
};

#include <reducer/util/docker_image.inl>
