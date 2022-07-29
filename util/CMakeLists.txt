# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

option(ENABLE_CODE_TIMING "Enable code timing" ON)
message(STATUS "time ENABLE_CODE_TIMING is ${ENABLE_CODE_TIMING}")

add_library(
  lz4_decompressor
  STATIC
    lz4_decompressor.cc
)
target_link_libraries(
  lz4_decompressor
    lz4
)

add_library(
  random
  STATIC
    random.cc
)
add_unit_test(random LIBS random)
add_unit_test(string_view)

add_library(
  time
  STATIC
    code_timing.cc
    time.cc
)
target_link_libraries(
  time
    stdc++fs
)
add_unit_test(time LIBS time)

add_library(
  proc_ops
  STATIC
    proc_ops.cc
)
target_link_libraries(
  proc_ops
    absl::flat_hash_map
)
add_unit_test(proc_ops LIBS proc_ops)

add_library(
  system_ops
  STATIC
    system_ops.cc
)

add_library(
  file_ops
  STATIC
    file_ops.cc
)
target_link_libraries(
  file_ops
    fastpass_util
    logging
    stdc++fs
)

add_library(
  url
  STATIC
    url.cc
)
target_link_libraries(
  url
    logging
)

add_library(
  base64
  STATIC
    base64.cc
)
target_link_libraries(
  base64
    OpenSSL::Crypto
)
add_unit_test(base64 LIBS base64)

add_library(
  environment_variables
  STATIC
    environment_variables.cc
)
target_link_libraries(
  environment_variables
    logging
)

add_library(
  signal_handler
  STATIC
    signal_handler.cc
)
target_link_libraries(
  signal_handler
    aws_instance_metadata
    gcp_instance_metadata
    args_parser
    logging
    breakpad_client
    file_ops
    system_ops
    environment_variables
    libuv-interface
    stdc++fs
    otlp_grpc_metrics_client
)

add_library(
  json
  INTERFACE
)
target_include_directories(
  json
  INTERFACE
    "${CMAKE_SOURCE_DIR}/ext/json/include"
)
add_unit_test(json LIBS json)

add_library(
  restful
  STATIC
    restful.cc
)
target_link_libraries(
  restful
    curl-cpp
    logging
)

add_library(
  resource_usage_reporter
  STATIC
    resource_usage_reporter.cc
)
target_link_libraries(
  resource_usage_reporter
    render_flowmill_artifacts
    scheduling
    system_ops
    logging
)

add_library(
  curl_engine
  STATIC
    curl_engine.cc
)

target_link_libraries(
  curl_engine
  PUBLIC
    libuv-interface
    element_queue_writer
    curl-static
)

add_library(
  logging
  STATIC
    log.cc
    log_whitelist.cc
)
target_link_libraries(
  logging
    environment_variables
    spdlog
)

add_library(
  agent_id
  STATIC
    agent_id.cc
)
target_link_libraries(
  agent_id
    url
    curl_engine
)

add_library(
  args_parser
  STATIC
    args_parser.cc
)
target_link_libraries(
  args_parser
    environment_variables
    logging
)

add_library(
  element_queue
  STATIC
    element_queue.c
)

add_library(
  fastpass_util
  STATIC
    perf_ring.c
    pool_allocator.c
    lookup3.c
    boot_time.c
)
target_compile_options(
  fastpass_util
  PRIVATE
    -fPIC
)

add_library(fixed_hash INTERFACE)
target_link_libraries(
  fixed_hash
  INTERFACE
    fastpass_util
    absl::flat_hash_map
)
add_unit_test(fixed_hash LIBS fixed_hash)

add_library(
  element_queue_writer
  STATIC
    element_queue_writer.cc
)
target_link_libraries(
  element_queue_writer
    logging
    logging
    element_queue
)

add_library(
  tdigest
  STATIC
    tdigest.cc
)
add_unit_test(tdigest LIBS tdigest)

add_library(
  ip_address
  STATIC
    ip_address.cc
)
target_link_libraries(
  ip_address
    logging
    render_flowmill_artifacts
)
add_unit_test(ip_address LIBS ip_address absl::str_format)

add_library(
  error_handling
  STATIC
    error_handling.cc
)
target_link_libraries(
  error_handling
    absl::strings
    absl::symbolize
    element_queue_writer
    logging
)

add_library(
  uv_helpers
  STATIC
    uv_helpers.cc
)
target_link_libraries(
  uv_helpers
    logging
    libuv-static
)

add_library(
  aws_instance_metadata
  STATIC
    aws_instance_metadata.cc
)
target_link_libraries(
  aws_instance_metadata
    restful
    json
    spdlog
)

add_library(
  gcp_instance_metadata
  STATIC
    gcp_instance_metadata.cc
)
target_link_libraries(
  gcp_instance_metadata
    restful
    json
    ip_address
    spdlog
)

add_library(
  docker_host_config_metadata
  STATIC
    docker_host_config_metadata.cc
)
target_link_libraries(
  docker_host_config_metadata
    json
)

add_library(
  k8s_metadata
  STATIC
    k8s_metadata.cc
)
target_link_libraries(
  k8s_metadata
    json
)

add_library(
  nomad_metadata
  STATIC
    nomad_metadata.cc
)
target_link_libraries(
  nomad_metadata
    environment_variables
    logging
    json
    absl::strings
)

add_unit_test(log_modifiers LIBS logging)
add_unit_test(expected)
add_unit_test(enum)
add_unit_test(meta LIBS render_flowmill_artifacts llvm logging)
add_unit_test(lookup3_hasher LIBS fastpass_util)
add_unit_test(jitter)
add_unit_test(bits)
add_unit_test(counter)
add_unit_test(counter_to_rate)
add_unit_test(gauge)
