# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


add_library(
  jitbuf
  STATIC
    fixed_handler.c
)
target_link_libraries(
  jitbuf
    element_queue
)
