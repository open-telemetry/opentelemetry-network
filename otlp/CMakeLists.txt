# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_directories(${CMAKE_INSTALL_PREFIX})

add_library(otlp_grpc_proto
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/resource/v1/resource.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/common/v1/common.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/logs/v1/logs_service.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/logs/v1/logs.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/metrics/v1/metrics_service.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/metrics/v1/metrics.pb.cc"
)

target_link_libraries(
  otlp_grpc_proto
    PkgConfig::grpc++
    PkgConfig::protobuf
)

add_library(
  otlp_grpc_metrics_emitter
  STATIC
    otlp_emitter.cc
    otlp_request_builder.cc
)
target_link_libraries(
  otlp_grpc_metrics_emitter
    otlp_grpc_proto
    time
)

add_unit_test(otlp_grpc_client SRCS otlp_request_builder.cc LIBS logging otlp_grpc_proto time)
