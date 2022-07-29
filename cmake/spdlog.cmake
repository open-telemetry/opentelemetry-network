# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE "${CMAKE_SOURCE_DIR}/ext/spdlog/include")
target_link_libraries(spdlog INTERFACE)
