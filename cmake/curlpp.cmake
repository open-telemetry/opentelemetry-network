# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

find_path(CURLPP_INCLUDE_DIR NAMES curlpp/cURLpp.hpp)
find_library(CURLPP_LIBRARY NAMES curlpp)


if (NOT CURLPP_INCLUDE_DIR OR NOT CURLPP_LIBRARY)
  message(FATAL_ERROR "cURLpp not found! Please install cURLpp development files.")
endif()

message(STATUS "curlpp library: ${CURLPP_LIBRARY}")
add_library(curl-cpp INTERFACE)
target_include_directories(
  curl-cpp
  INTERFACE
    "${CURLPP_INCLUDE_DIR}"
)
target_link_libraries(
  curl-cpp
  INTERFACE
    "${CURLPP_LIBRARY}"
    CURL::libcurl
)
