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

add_library(
  jitbuf_llvm
  STATIC
    descriptor_reader.cc
    transformer.cc
    transform_builder.cc
    handler.cc
)
target_link_libraries(
  jitbuf_llvm
  PUBLIC
    llvm-interface
    jitbuf
)
