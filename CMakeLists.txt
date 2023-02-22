# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.12)

project(
  opentelemetry-ebpf
    VERSION 0.9.0
)

list(
  APPEND
  CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/cmake
)

include(FindPkgConfig)

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Building as a subproject (submodule).
  set(EBPF_NET_SUBPROJECT TRUE)
endif()

# Custom modules
#
include(cpp-compiler)
include(ccache)
include(docker-utils)
include(sanitizer)
include(executable)
include(xxd)
include(shell)
include(debug)
include(lz4)
include(openssl)
include(civetweb)
include(curl)
include(curlpp)
include(spdlog)
include(aws-sdk)
include(geoip)
include(golang)
include(protobuf)
include(llvm)
include(clang)
include(libelf)
include(bcc)
include(test)
include(uv)
include(breakpad)
include(abseil)
include(yamlcpp)
include(render)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_INSTALL_PREFIX}
  ${CMAKE_INSTALL_PREFIX}/include
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(CONFIG_H_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(pipeline)
add_custom_target(pipeline-docker)
add_custom_target(pipeline-docker-registry)

add_subdirectory(renderc)
add_subdirectory(render)
add_subdirectory(config)
add_subdirectory(channel)
add_subdirectory(platform)
add_subdirectory(scheduling)
add_subdirectory(util)
add_subdirectory(geoip)
add_subdirectory(jitbuf)
add_subdirectory(collector)
add_subdirectory(reducer)
add_subdirectory(otlp)
add_subdirectory(dev)
add_subdirectory(tools)

configure_file(config.h.cmake_in config.h)

add_dependencies(pipeline collectors reducer)
add_dependencies(pipeline-docker collectors-docker)
add_dependencies(pipeline-docker-registry collectors-docker-registry)
