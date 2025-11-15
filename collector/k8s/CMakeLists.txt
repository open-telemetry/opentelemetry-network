# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Build the Rust k8s-collector binary and package it into a container.

include(rust_main)

add_rust_main(
  TARGET           k8s-collector
  STRIPPED_TARGET  k8s-collector-stripped
  PACKAGE          k8s-collector-bin
  BIN_NAME         k8s-collector
  LINK_LIBS        versions
)

add_dependencies(collectors k8s-collector)

build_custom_docker_image(
  k8s-collector
  DOCKERFILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
  OUT_DIR srv
  OUTPUT_OF
    k8s-collector-stripped
  BINARIES
    debug-info.conf
  FILES
    ../../NOTICE.txt
    ../../LICENSE.txt
  DEPENDENCY_OF
    collectors
)
