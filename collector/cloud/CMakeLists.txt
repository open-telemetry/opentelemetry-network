# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_library(
  cloud_agentlib
  STATIC
    entrypoint.cc
    collector.cc
    enumerator.cc
    ingest_connection.cc
)
target_include_directories(
  cloud_agentlib
  PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(
  cloud_agentlib
  PUBLIC
    signal_handler 
    render_ebpf_net_cloud_collector
)

set(CLOUD_COLLECTOR_LINK_LIBRARIES
    cloud_agentlib
    render_ebpf_net_cloud_collector
    render_ebpf_net_ingest_writer
    aws-sdk-cpp
    reconnecting_channel
    connection_caretaker
    resource_usage_reporter
    config_file
    ip_address
    scheduling
    libuv-static
    args_parser
    system_ops
    aws_instance_metadata
    spdlog
    static-executable
    protobuf::libprotobuf
    ZLIB::ZLIB
    versions
)


lint_shell_script_bundle(
  cloud-collector-scripts
  SOURCES
    entrypoint.sh
)

include(rust_main)
add_rust_main(
  TARGET           cloud-collector
  STRIPPED_TARGET  cloud-collector-stripped
  PACKAGE          cloud-collector-bin
  BIN_NAME         cloud-collector
  LINK_LIBS        ${CLOUD_COLLECTOR_LINK_LIBRARIES}
)

add_dependencies(collectors cloud-collector cloud-collector-stripped)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  build_custom_docker_image(
    cloud-collector
    OUT_DIR srv
    OUTPUT_OF
      cloud-collector-scripts
      cloud-collector-stripped
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
    cloud-collector
    OUT_DIR srv
    OUTPUT_OF
      cloud-collector-scripts
      cloud-collector-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
  )
endif()
