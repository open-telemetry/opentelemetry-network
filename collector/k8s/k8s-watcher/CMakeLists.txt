# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_custom_target(k8s-watcher)

build_go_package(
  k8s-watcher
  collector-go-module
  BINARY
  DEPENDS
    collector-go-protobuf
    kubernetes_info-go-protobuf
  DEPENDENCY_OF
    k8s-watcher
    collectors
)

build_custom_docker_image(
  k8s-watcher
  OUT_DIR srv
  OUTPUT_OF
    k8s-watcher-go
  DEPENDENCY_OF
    collectors
)
