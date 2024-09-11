# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

set(CIVETWEB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/ext/civetweb/include)

add_library(
  civetweb
  OBJECT
    ext/civetweb/include/CivetServer.h
    ext/civetweb/include/civetweb.h
    ext/civetweb/src/CivetServer.cpp
    ext/civetweb/src/civetweb.c
    ext/civetweb/src/handle_form.inl
    ext/civetweb/src/md5.inl
)

target_compile_definitions(
  civetweb
  PRIVATE
    USE_IPV6
    NDEBUG
    NO_CGI
    NO_CACHING
    NO_SSL
    NO_FILES
)

target_include_directories(civetweb PUBLIC ${CIVETWEB_INCLUDE_DIR})

find_library(LIBDL dl)
target_link_libraries(civetweb PRIVATE ${LIBDL})

add_library(civetweb-interface INTERFACE)
target_include_directories(civetweb-interface INTERFACE ${CIVETWEB_INCLUDE_DIR})
