# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_path(YAMLCPP_INCLUDE_DIR yaml-cpp/yaml.h)
find_library(YAMLCPP_LIBRARY NAMES "libyaml-cpp.a")
find_package_handle_standard_args(YAMLCPP DEFAULT_MSG YAMLCPP_LIBRARY YAMLCPP_INCLUDE_DIR)
if(NOT YAMLCPP_FOUND)
  message(FATAL_ERROR "Could not find yaml-cpp. Build container should already have that set up")
endif()
message(STATUS "yaml-cpp INCLUDE_DIR: ${YAMLCPP_INCLUDE_DIR}")
message(STATUS "yamp-cpp LIBRARY: ${YAMLCPP_LIBRARY}")
add_library(yamlcpp INTERFACE)
target_include_directories(yamlcpp INTERFACE "${YAMLCPP_INCLUDE_DIR}")
target_link_libraries(yamlcpp INTERFACE "${YAMLCPP_LIBRARY}")
