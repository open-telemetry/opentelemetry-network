# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_custom_target(collectors)
add_custom_target(collectors-docker)
add_custom_target(collectors-docker-registry)

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
  DOCKERFILE_NAME Dockerfile.k8s-collector
  OUT_DIR srv
  OUTPUT_OF
    k8s-collector-stripped
  BINARIES
    debug-info.conf
  FILES
    ../NOTICE.txt
    ../LICENSE.txt
  DEPENDENCY_OF
    collectors
)

add_subdirectory(cloud)
add_subdirectory(kernel)
