# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_tool_executable(
  otlp-to-prom
  SRCS
    main.cc
    metrics_server_impl.cc
  DEPS
    otlp_grpc_proto
    args_parser
)

build_custom_docker_image(
  otlp-to-prom
  OUT_DIR srv
  ARTIFACTS_OF
    otlp-to-prom
)
