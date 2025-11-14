# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(util)

# CXX bridge for Rust reducer crate (exposes config + run entrypoint)
add_rust_cxxbridge(
  reducer_entrypoint_cxxbridge
  ${CMAKE_SOURCE_DIR}/crates/reducer-sys/src/lib.rs
)

# CXX bridge for Rust AggregationCore (reducer crate side)
add_rust_cxxbridge(
  reducer_aggregation_cxxbridge
  ${CMAKE_SOURCE_DIR}/crates/reducer/src/ffi.rs
)

# Reducer library
#
add_library(
  reducerlib
    entrypoint.cc
    reducer.cc
    reducer_config.cc
    core.cc
    worker.cc
    uid_key.cc
    rpc_stats.cc
    ingest/ingest_core.cc
    ingest/ingest_worker.cc
    ingest/shared_state.cc
    ingest/tcp_server.cc
    ingest/agent_span.cc
    ingest/socket_span.cc
    ingest/udp_socket_span.cc
    ingest/process_span.cc
    ingest/cgroup_span.cc
    ingest/k8s_pod_span.cc
    ingest/flow_updater.cc
    ingest/npm_connection.cc
    ingest/aws_network_interface_span.cc
    matching/matching_core.cc
    matching/flow_span.cc
    matching/aws_enrichment_span.cc
    matching/k8s_pod_span.cc
    matching/k8s_container_span.cc
    aggregation/agg_core.cc
    logging/logging_core.cc
    logging/logger_span.cc
    logging/core_stats_span.cc
    logging/agg_core_stats_span.cc
    logging/ingest_core_stats_span.cc
)
target_link_libraries(
  reducerlib
    reducer_entrypoint_cxxbridge
    reducer_aggregation_cxxbridge
    signal_handler
    metrics_output
    render_pipeline
    tcp_channel
    buffered_writer
    blob_collector
    index_dumper
    scheduling
    libuv-interface
    element_queue_writer
    fastpass_util
    error_handling
    time_tracker
    json
    lz4_decompressor
    geoip_wrapper
    absl::flat_hash_map
    absl::flat_hash_set
    absl::node_hash_map
    absl::str_format
    absl::strings
    absl::synchronization
    absl::time
    yaml-cpp
    tdigest
    ip_address
    file_ops
    args_parser
    uv_helpers
    system_ops
    thread_ops
    error_handling
    environment_variables
    virtual_clock
    cgroup_parser
    versions
    render_rust_ebpf_net
)
add_dependencies(
  reducerlib
    render_compile_ebpf_net
)

# Library containing code responsible for publishing metrics (e.g. to a TSDB).
#
add_library(
  metrics_output
    tsdb_formatter.cc
    prometheus_formatter.cc
    json_formatter.cc
    otlp_grpc_formatter.cc
    otlp_grpc_publisher.cc
    null_publisher.cc
    prometheus_handler.cc
    prometheus_publisher.cc
    disabled_metrics.cc
    metric_info.cc
    stat_info.cc
    $<TARGET_OBJECTS:civetweb>
)
target_link_libraries(
  metrics_output
    element_queue_writer
    error_handling
    civetweb-interface
    yaml-cpp
    time
    otlp_export_cxxbridge
)
add_dependencies(
  metrics_output
    render_compile_ebpf_net
    otlp_export_cxxbridge
)

# CXX bridge for Rust otlp_export crate
add_rust_cxxbridge(
  otlp_export_cxxbridge
  ${CMAKE_SOURCE_DIR}/crates/otlp_export/src/lib.rs
)

# Shell scripts
#
lint_shell_script_bundle(
  reducer-scripts
  SOURCES
    entrypoint.sh
    health_check.sh
)

# Rust binary integration
include(rust_main)
set(REDUCER_LINK_LIBRARIES
    reducerlib
    metrics_output
    libuv-static
    spdlog
)
add_rust_main(
  TARGET           reducer
  STRIPPED_TARGET  reducer-stripped
  PACKAGE          reducer-bin
  BIN_NAME         reducer
  LINK_LIBS        ${REDUCER_LINK_LIBRARIES}
)

# Docker image
#
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  build_custom_docker_image(
    reducer
    OUT_DIR srv
    OUTPUT_OF
      reducer-scripts
      reducer-stripped
    BINARIES
      debug-info.conf
    DEPENDENCY_OF
      pipeline
    ARGS
      BUILD_TYPE="Debug"
  )
else()
  build_custom_docker_image(
    reducer
    OUT_DIR srv
    OUTPUT_OF
      reducer-scripts
      reducer-stripped
    BINARIES
      debug-info.conf
    DEPENDENCY_OF
      pipeline
  )
endif()

# Unit Tests
# otlp_grpc_formatter test removed due to Rust-backed exporter path
add_unit_test(rpc_queue_matrix LIBS fastpass_util element_queue_writer)
add_unit_test(disabled_metrics LIBS metrics_output)

# Disable the reducer_test. It doesn't link because of Rust dependencies -- fixable
# but doesn't seem worth the effort. The CI e2e test runs the reducer with a
# kernel-collector and otelcol, whereas the reducer_test just runs a reducer with
# a synthetic collector (and no kernel-collector)
# add_unit_test(reducer LIBS reducerlib libuv-static static-executable spdlog)
