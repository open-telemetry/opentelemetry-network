# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

###########################
# static compilation target
add_library(static-executable INTERFACE)
target_link_libraries(
  static-executable
  INTERFACE
    address_sanitizer-static
    -static-libstdc++
)

###########################
# shared compilation target
add_library(shared-executable INTERFACE)
target_link_libraries(
  shared-executable
  INTERFACE
    address_sanitizer-shared
)
