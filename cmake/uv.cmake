# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_path(LIBUV_INCLUDE_DIR uv.h)
find_library(LIBUV_LIBS NAMES uv libuv)
find_library(LIBUV_STATIC_LIBRARY NAMES libuv.a)
find_package_handle_standard_args(LIBUV DEFAULT_MSG
                                      LIBUV_LIBS
                                      LIBUV_INCLUDE_DIR
                                      LIBUV_STATIC_LIBRARY)
if((NOT LIBUV_INCLUDE_DIR) OR (NOT LIBUV_LIBS) OR (NOT LIBUV_STATIC_LIBRARY))
  message(FATAL_ERROR "Could not find libuv. Build container should already have that set up")
endif()

message(STATUS "libuv INCLUDE_DIR: ${LIBUV_INCLUDE_DIR}")
message(STATUS "libuv LIBS: ${LIBUV_LIBS}")
message(STATUS "libuv STATIC_LIBRARY: ${LIBUV_STATIC_LIBRARY}")

add_library(libuv-interface INTERFACE)
target_include_directories(
  libuv-interface
  INTERFACE
    "${LIBUV_INCLUDE_DIR}"
)

add_library(libuv-shared INTERFACE)
target_link_libraries(
  libuv-shared
  INTERFACE
    ${LIBUV_LIBS}
    libuv-interface
)

add_library(libuv-static INTERFACE)
target_link_libraries(
  libuv-static
  INTERFACE
    ${LIBUV_STATIC_LIBRARY}
)
target_include_directories(
  libuv-static
  INTERFACE
    "${LIBUV_INCLUDE_DIR}"
    libuv-interface
)
