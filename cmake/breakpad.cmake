# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

pkg_check_modules(GOOGLE_BREAKPAD REQUIRED breakpad-client)
add_library(breakpad_client INTERFACE)
target_compile_options(breakpad_client INTERFACE "${GOOGLE_BREAKPAD_CFLAGS}")
target_link_libraries(breakpad_client INTERFACE "${GOOGLE_BREAKPAD_LDFLAGS}")
