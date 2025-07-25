# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

find_package(CURL)
if(NOT CURL_FOUND)
  message(FATAL_ERROR "cURL not found! Please install cURL development files.")
endif()

message(STATUS "Found cURL: ${CURL_LIBRARIES}")
