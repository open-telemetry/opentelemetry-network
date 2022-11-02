// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

constexpr DockerImageMetadata::DockerImageMetadata(std::string_view image)
{
  if (auto const i = image.find_last_of(IMAGE_DELIMITER); i != std::string_view::npos) {
    registry_ = image.substr(0, i);
    image.remove_prefix(registry_.size() + 1);
  }

  if (auto const i = image.find_first_of(CHECKSUM_DELIMITER); i != std::string_view::npos) {
    version_ = image.substr(i + 1);
    name_ = image.substr(0, i);
    return;
  }

  if (auto const i = image.find_first_of(VERSION_DELIMITER); i != std::string_view::npos) {
    name_ = image.substr(0, i);

    if (name_ == "sha256") {
      name_ = std::string_view{};
      version_ = image;
    } else {
      version_ = image.substr(i + 1);
    }
  } else {
    name_ = image;
  }
}
