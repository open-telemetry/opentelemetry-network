option(BUILD_WITH_OTLP "Build with OTLP support" OFF)
message(STATUS "otlp BUILD_WITH_OTLP is ${BUILD_WITH_OTLP}")

if (BUILD_WITH_OTLP)
  # TODO benv-final:v8 doesn't currently support building with OTLP - need to use a custom local benv build. This needs
  # to be cleaned up when the official benv is updated to support OTLP builds.

  #------------------------------------------------------------------------------------------
  # from https://github.com/grpc/grpc/blob/master/examples/cpp/cmake/common.cmake

  # This branch assumes that gRPC and all its dependencies are already installed
  # on this system, so they can be located by find_package().

  # Find Protobuf installation
  # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
  set(protobuf_MODULE_COMPATIBLE TRUE)
  find_package(Protobuf CONFIG REQUIRED)
  message(STATUS "Using protobuf ${Protobuf_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
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

  set(_GRPC_GRPCPP gRPC::grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  endif()
  #------------------------------------------------------------------------------------------

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
      ${_GRPC_GRPCPP}
      ${_PROTOBUF_LIBPROTOBUF})

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
      otlp_grpc_metrics_client.cc
  )
  target_link_libraries(
    otlp_grpc_metrics_client
      otlp_grpc_proto
      time
  )
endif()
