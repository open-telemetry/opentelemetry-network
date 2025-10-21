# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


get_property(
  RENDER_COMPILER
  TARGET
    render_compiler
  PROPERTY
    RENDER_COMPILER_PATH
)
render_compile(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PACKAGE
    ebpf_net
  APPS
    agent_internal
    kernel_collector
    cloud_collector
    ingest
    matching
    aggregation
    logging
  COMPILER
    ${RENDER_COMPILER}
  OUTPUT_DIR
    "${CMAKE_BINARY_DIR}/generated"
  DEPENDS
    render_compiler
)

add_library(
  render_pipeline
  INTERFACE
)
target_link_libraries(
  render_pipeline
  INTERFACE
    render_ebpf_net_ingest
    render_ebpf_net_matching
    render_ebpf_net_aggregation
    render_ebpf_net_logging
)
