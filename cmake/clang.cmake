# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_package(CLANG REQUIRED CONFIG NAMES Clang)
message(STATUS "Found Clang ${CLANG_VERSION}")
message(STATUS "Using ClangConfig.cmake in: ${CLANG_CONFIG}")
