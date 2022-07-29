# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

pkg_check_modules(LIBELF REQUIRED libelf)
add_library(libelf INTERFACE)
target_compile_options(libelf INTERFACE "${LIBELF_CFLAGS}")
target_link_libraries(libelf INTERFACE "${LIBELF_LINK_LIBRARIES}")
message(STATUS "libelf CFLAGS: ${LIBELF_CFLAGS}")
message(STATUS "libelf LIBRARIES: ${LIBELF_LINK_LIBRARIES}")
