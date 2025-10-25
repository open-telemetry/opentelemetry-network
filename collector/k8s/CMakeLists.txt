# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

setup_go_module(collector "ebpf.net" VENDORED)

build_protobuf(
  kubernetes_info
  CPP
  GO collector
)

build_protobuf(
  collector
  CPP GRPC
  GO collector
  DEPENDS
    kubernetes_info-protobuf
)

#############
# k8s-relay #
#############

add_library(
  kubernetes_owner_kind
  STATIC
    kubernetes_owner_kind.cc
)
target_link_libraries(
  kubernetes_owner_kind
    render_ebpf_net_artifacts
)

add_library(
  resync_channel
  STATIC
    resync_channel.cc
)
target_link_libraries(
  resync_channel
    render_ebpf_net_artifacts
)

add_library(
  kubernetes_rpc_server
  STATIC
    kubernetes_rpc_server.cc
)
target_link_libraries(
  kubernetes_rpc_server
    collector-cpp-protobuf
    kubernetes_info-cpp-protobuf
    yamlcpp
    fastpass_util
    render_ebpf_net_artifacts
)

add_library(
  resync_queue
  STATIC
    resync_queue.cc
)
target_link_libraries(
  resync_queue
    render_ebpf_net_artifacts
)

add_library(
  resync_processor
  STATIC
    resync_processor.cc
)
target_link_libraries(
  resync_processor
    reconnecting_channel
    libuv-interface
    spdlog
    fastpass_util
    render_ebpf_net_artifacts
)

add_library(
  k8s_entrylib
  STATIC
    entrypoint.cc
)
target_link_libraries(
  k8s_entrylib
  PUBLIC
    signal_handler
    kubernetes_rpc_server
)
target_include_directories(
  k8s_entrylib
  PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(K8S_RELAY_LINK_LIBRARIES
    k8s_entrylib
    resync_processor
    resync_queue
    resync_channel
    kubernetes_owner_kind
    connection_caretaker
    resource_usage_reporter
    render_ebpf_net_artifacts
    render_ebpf_net_ingest_writer
    config_file
    libuv-static
    args_parser
    system_ops
    spdlog
    static-executable
    gRPC::grpc++_unsecure
    gRPC::grpc_unsecure
    versions
)

lint_shell_script_bundle(
  k8s-relay-scripts
  SOURCES
    entrypoint.sh
)

include(rust_main)
add_rust_main(
  TARGET           k8s-relay
  STRIPPED_TARGET  k8s-relay-stripped
  PACKAGE          k8s-relay-bin
  BIN_NAME         k8s-relay
  LINK_LIBS        ${K8S_RELAY_LINK_LIBRARIES}
)

add_dependencies(collectors k8s-relay)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  build_custom_docker_image(
    k8s-relay
    OUT_DIR srv
    OUTPUT_OF
      k8s-relay-scripts
      k8s-relay-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
  )
else()
  build_custom_docker_image(
    k8s-relay
    OUT_DIR srv
    OUTPUT_OF
      k8s-relay-scripts
      k8s-relay-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
  )
endif()

add_subdirectory(k8s-watcher)
