# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_path(LZ4_INCLUDE_DIR lz4.h)
find_library(LZ4_LIBRARY NAMES "liblz4.a")
find_package_handle_standard_args(LZ4 DEFAULT_MSG LZ4_LIBRARY LZ4_INCLUDE_DIR)
if(NOT LZ4_FOUND)
  message(FATAL_ERROR "Could not find lz4. Build container should already have that set up")
endif()
message(STATUS "lz4 INCLUDE_DIR: ${LZ4_INCLUDE_DIR}")
message(STATUS "lz4 LIBRARY: ${LZ4_LIBRARY}")
add_library(lz4 INTERFACE)
target_include_directories(lz4 INTERFACE "${LZ4_INCLUDE_DIR}")
target_link_libraries(lz4 INTERFACE "${LZ4_LIBRARY}")
