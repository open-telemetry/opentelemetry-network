# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_path(CURLPP_INCLUDE_DIR NAMES curlpp/cURLpp.hpp)
find_library(CURLPP_STATIC_LIBRARY NAMES libcurlpp.a)
message(STATUS "curlpp static library: ${CURLPP_STATIC_LIBRARY}")
add_library(curl-cpp INTERFACE)
target_include_directories(
  curl-cpp
  INTERFACE
    "${CURLPP_INCLUDE_DIR}"
)
target_link_libraries(
  curl-cpp
  INTERFACE
    "${CURLPP_STATIC_LIBRARY}"
    curl-static
)
