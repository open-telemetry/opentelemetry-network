# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_library(
  scheduling
  STATIC
    interval_scheduler.cc
    timer.cc
)
target_compile_options(
  scheduling
  PRIVATE
    ${CXX_ERROR_LIMIT_FLAG}=1
)
target_link_libraries(
  scheduling
    libuv-interface
    spdlog
)
