# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include(tool)

add_tool_executable(
  error_lookup
  SRCS
    error_lookup.cc
  DEPS
    libuv-shared
)

add_library(wire_msg_to_json INTERFACE)
target_link_libraries(
  wire_msg_to_json
  INTERFACE
    file_ops
    json
    logging
)

add_tool_executable(
  bpf_wire_to_json
  SRCS
    bpf_wire_to_json.cc
  DEPS
    render_ebpf_net_agent_internal
    wire_msg_to_json
)

add_tool_executable(
  intake_wire_to_json
  SRCS
    intake_wire_to_json.cc
  DEPS
    render_pipeline
    wire_msg_to_json
)

add_tool_executable(
  matching_wire_to_json
  SRCS
    matching_wire_to_json.cc
  DEPS
    render_pipeline
    wire_msg_to_json
)

add_tool_executable(
  aggregation_wire_to_json
  SRCS
    aggregation_wire_to_json.cc
  DEPS
    render_pipeline
    wire_msg_to_json
)

add_subdirectory(otlp-to-prom)
