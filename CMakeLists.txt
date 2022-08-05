# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.12)

project(
  network-explorer
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
if(NOT EBPF_NET_SUBPROJECT)
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
include(curl)
include(curlpp)
include(spdlog)
include(aws-sdk)
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
endif(NOT EBPF_NET_SUBPROJECT)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_INSTALL_PREFIX}
  ${CMAKE_INSTALL_PREFIX}/include
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(CONFIG_H_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(renderc)
add_subdirectory(render)
add_subdirectory(config)
add_subdirectory(channel)
add_subdirectory(platform)
add_subdirectory(scheduling)
add_subdirectory(util)
add_subdirectory(jitbuf)
add_subdirectory(collector)
add_subdirectory(otlp)

configure_file(config.h.cmake_in config.h)
