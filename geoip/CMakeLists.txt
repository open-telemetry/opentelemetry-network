# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_library(
  geoip_wrapper
  STATIC
    geoip.cc
)
target_link_libraries(
  geoip_wrapper
    libmaxminddb
)
target_compile_options(
  geoip_wrapper
    PRIVATE
      -Werror
      -Wall
      -Wextra
)
