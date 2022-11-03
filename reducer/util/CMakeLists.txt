# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


add_library(
  blob_collector
  STATIC
    blob_collector.cc
)
target_link_libraries(
  blob_collector
    logging
)

add_library(
  index_dumper
  STATIC
    index_dumper.cc
)
target_link_libraries(
  index_dumper
    logging
    spdlog
)

add_library(
  thread_ops
  STATIC
    thread_ops.cc
)
target_link_libraries(
  thread_ops
)

add_library(
  signal_handling
  STATIC
    signal_handler.cc
)
target_link_libraries(
  signal_handling
    thread_ops
    libuv-interface
)

add_library(
  time_tracker
  STATIC
    time_tracker.cc
)
target_link_libraries(
  time_tracker
    libuv-interface
)

add_library(
  virtual_clock
  STATIC
    virtual_clock.cc
)
add_unit_test(
  virtual_clock
  LIBS
    virtual_clock
)
