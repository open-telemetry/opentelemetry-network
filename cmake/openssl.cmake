# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

find_package(OpenSSL REQUIRED)
if (NOT OPENSSL_VERSION STREQUAL "1.1.1w")
  message(FATAL_ERROR "OpenSSL must be a specific version (1.1.1w). Build container should already have that set up")
endif()
