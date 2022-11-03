# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

find_path(LIBMAXMINDDB_INCLUDE_DIR NAMES maxminddb.h)
find_library(LIBMAXMINDDB_LIBRARIES_C NAMES "libmaxminddb.a")
find_package_handle_standard_args(LIBMAXMINDDB DEFAULT_MSG LIBMAXMINDDB_LIBRARIES_C LIBMAXMINDDB_INCLUDE_DIR)
if(NOT LIBMAXMINDDB_FOUND)
    message(FATAL_ERROR "Could not find libmaxminddb. Build container should already have that set up")
endif()
message(STATUS "libmaxminddb INCLUDE_DIR: ${LIBMAXMINDDB_INCLUDE_DIR}")
message(STATUS "libmaxminddb LIBRARY: ${LIBMAXMINDDB_LIBRARIES_C}")
add_library(libmaxminddb INTERFACE)
target_include_directories(libmaxminddb INTERFACE "${LIBMAXMINDDB_INCLUDE_DIR}")
target_link_libraries(libmaxminddb INTERFACE "${LIBMAXMINDDB_LIBRARIES_C}")
