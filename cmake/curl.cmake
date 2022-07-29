# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_library(CURL_STATIC_LIBRARY NAMES libcurl.a)
message(STATUS "curl static library: ${CURL_STATIC_LIBRARY}")
add_library(curl-static INTERFACE)
target_link_libraries(
  curl-static
  INTERFACE
    ${CURL_STATIC_LIBRARY}
    OpenSSL::SSL
    OpenSSL::Crypto
    z
)
