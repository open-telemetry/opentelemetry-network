# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_library(
  libgeoip_wrapper
  STATIC
    geoip.cc
)
target_link_libraries(
  libgeoip_wrapper
    libmaxminddb
)
target_compile_options(
  libgeoip_wrapper
    PRIVATE
      -Werror
      -Wall
      -Wextra
)
