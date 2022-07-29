# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Protobuf and gRPC related cmake statements from https://github.com/grpc/grpc/blob/master/examples/cpp/cmake/common.cmake

# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

if (NOT gRPC_VERSION STREQUAL "1.45.0")
  message(FATAL_ERROR "gRPC must be a specific version (1.45.0). Build container should already have that set up.  Note that benv-final:v8 doesn't currently support building with OTLP - need to use a custom local benv build.")
endif()

include_directories(${CMAKE_INSTALL_PREFIX})

add_library(otlp_grpc_proto
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/resource/v1/resource.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/common/v1/common.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/logs/v1/logs_service.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/logs/v1/logs_service.grpc.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/logs/v1/logs.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/metrics/v1/metrics_service.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.cc"
  "${CMAKE_INSTALL_PREFIX}/opentelemetry/proto/metrics/v1/metrics.pb.cc")

target_link_libraries(
  otlp_grpc_proto
    gRPC::grpc++
    protobuf::libprotobuf
)

add_library(
  otlp_grpc_logs_client
  STATIC
    otlp_grpc_logs_client.cc
)
target_link_libraries(
  otlp_grpc_logs_client
    otlp_grpc_proto
)

add_library(
  otlp_grpc_metrics_client
  STATIC
    otlp_emitter.cc
    otlp_grpc_metrics_client.cc
    otlp_request_builder.cc
)
target_link_libraries(
  otlp_grpc_metrics_client
    otlp_grpc_proto
    time
)
