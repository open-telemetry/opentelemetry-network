# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


# Kernel collector legacy executable
#
add_executable(
  kernel-collector-legacy
    main.cc
)
set(KERNEL_COLLECTOR_LINK_LIBRARIES
    agentlib
    render_ebpf_net_ingest_writer
    render_rust_ebpf_net
    fastpass_util
    file_ops
    config_file
    libuv-static
    args_parser
    system_ops
    spdlog
)
target_link_libraries(
  kernel-collector-legacy
  PUBLIC
    ${KERNEL_COLLECTOR_LINK_LIBRARIES}
    static-executable
)
target_include_directories(
    kernel-collector-legacy

  PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

# DNS library
#
add_library(
  agentdnslib
  STATIC
    dns/ares_expand_name.c
    dns/ares_parse_a_aaaa_reply.c
    dns/ares_parse_query.c
)
add_dependencies(agentdnslib render_ebpf_net_artifacts)
target_compile_options(agentdnslib PRIVATE -fPIC)

# Agent library
#
add_library(
  agentlib
  STATIC
    perf_reader.cc
    perf_poller.cc
    buffered_poller.cc
    dns_requests.cc
    proc_reader.cc
    process_prober.cc
    process_handler.cc
    socket_prober.cc
    fd_reader.cc
    proc_net_reader.cc
    proc_cmdline.cc
    probe_handler.cc
    kernel_collector.cc
    kernel_collector_restarter.cc
    bpf_handler.cc
    cgroup_prober.cc
    cgroup_handler.cc
    nat_prober.cc
    nat_handler.cc
    troubleshooting.cc
    tcp_data_handler.cc
    kernel_symbols.cc
    protocols/protocol_handler_base.cc
    protocols/protocol_handler_unknown.cc
    protocols/protocol_handler_http.cc
    entrypoint.cc
)
target_link_libraries(
  agentlib
  PUBLIC
    render_ebpf_net_agent_internal_hash
    render_ebpf_net_ingest_writer
    render_ebpf_net_kernel_collector
    agentdnslib
    yamlcpp
    curl_engine
    agent_id
    resource_usage_reporter
    scheduling
    libuv-interface
    element_queue_writer
    file_channel
    upstream_connection
    aws_instance_metadata
    gcp_instance_metadata
    docker_host_config_metadata
    k8s_metadata
    nomad_metadata
    ip_address
    intake_config
    proc_ops
    system_ops
    time
    absl::flat_hash_map
    absl::flat_hash_set
    stdc++fs
    libbpf::libbpf
    versions
    signal_handler
)
target_compile_options(agentlib PRIVATE -fPIC)
# some agentlib sources include ${BPF_DEBUG_INFO}
add_dependencies(agentlib generate_bpf_skeleton)

################################################################################
# Rust binary integration (generalized)
#
include(rust_main)
add_rust_main(
  TARGET           kernel-collector
  STRIPPED_TARGET  kernel-collector-stripped
  PACKAGE          kernel-collector-bin
  BIN_NAME         kernel-collector
  LINK_LIBS        ${KERNEL_COLLECTOR_LINK_LIBRARIES}
)

add_dependencies(collectors kernel-collector kernel-collector-stripped)

################################################################################
# libbpf skeleton
#

set(BPF_DEPENDENCIES
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/render_bpf.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/render_bpf.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_debug.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_http_protocol.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_inet_csk_accept.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_memory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_tcp_events.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_tcp_processor.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_tcp_send_recv.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_tcp_socket.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/tcp-processor/bpf_types.h"
  "${CMAKE_BINARY_DIR}/generated/ebpf_net/agent_internal/wire_message.h"
  "${CMAKE_BINARY_DIR}/generated/ebpf_net/agent_internal/bpf.h"
  "${PROJECT_SOURCE_DIR}/jitbuf/jb.h"
  render_compile_ebpf_net
)

set(BPF_SKEL_HEADER "${CMAKE_BINARY_DIR}/generated/render_bpf.skel.h")
set(BPF_OBJECT_FILE "${CMAKE_BINARY_DIR}/generated/render_bpf.bpf.o")
set(BPFTOOL "${CMAKE_INSTALL_PREFIX}/usr/local/sbin/bpftool")
set(CLANG clang)

# Get Clang's system includes for BPF compilation
execute_process(
  COMMAND ${CLANG} -v -E - 
  INPUT_FILE /dev/null
  ERROR_VARIABLE CLANG_VERBOSE_OUTPUT
  OUTPUT_QUIET
)
string(REGEX MATCH "#include <\\.\\.\\.> search starts here:(.*)End of search list\\." _ "${CLANG_VERBOSE_OUTPUT}")
string(REGEX REPLACE "\n" ";" CLANG_INCLUDES "${CMAKE_MATCH_1}")
set(CLANG_BPF_SYS_INCLUDES "")
foreach(INCLUDE_PATH ${CLANG_INCLUDES})
  string(STRIP "${INCLUDE_PATH}" INCLUDE_PATH)
  if(NOT "${INCLUDE_PATH}" STREQUAL "")
    list(APPEND CLANG_BPF_SYS_INCLUDES "-idirafter" "${INCLUDE_PATH}")
  endif()
endforeach()

# Determine architecture for vmlinux.h
set(ARCH_RAW "${CMAKE_HOST_SYSTEM_PROCESSOR}")
string(REGEX REPLACE "x86_64" "x86" ARCH "${ARCH_RAW}")
string(REGEX REPLACE "arm.*" "arm" ARCH "${ARCH}")
string(REGEX REPLACE "aarch64" "arm64" ARCH "${ARCH}")
string(REGEX REPLACE "ppc64le" "powerpc" ARCH "${ARCH}")
string(REGEX REPLACE "mips.*" "mips" ARCH "${ARCH}")
string(REGEX REPLACE "riscv64" "riscv" ARCH "${ARCH}")
string(REGEX REPLACE "loongarch64" "loongarch" ARCH "${ARCH}")

set(VMLINUX_H "${PROJECT_SOURCE_DIR}/ext/vmlinux.h/include/${ARCH}/vmlinux.h")
get_filename_component(VMLINUX_DIR "${VMLINUX_H}" DIRECTORY)


# Compile BPF source to object file for libbpf skeleton
add_custom_command(
  OUTPUT
    "${BPF_OBJECT_FILE}"
  COMMAND
    "${CLANG}"
    -g -O2 -target bpf
    -D__TARGET_ARCH_${ARCH}
    -D_PROCESSING_BPF=1
    -I "${CMAKE_BINARY_DIR}/generated"
    -I "${CMAKE_BINARY_DIR}"
    -I "${CMAKE_INSTALL_PREFIX}/usr/lib64"
    -I "${CMAKE_INSTALL_PREFIX}/usr/include"
    -I "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src"
    -I "${PROJECT_SOURCE_DIR}"
    -I "${VMLINUX_DIR}"
    ${CLANG_BPF_SYS_INCLUDES}
    -c "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/render_bpf.c"
    -o "${BPF_OBJECT_FILE}.tmp"
  COMMAND
    "${BPFTOOL}" gen object "${BPF_OBJECT_FILE}" "${BPF_OBJECT_FILE}.tmp"
  DEPENDS "${BPF_DEPENDENCIES}" "${VMLINUX_H}"
  COMMENT "Compiling BPF source to object file for skeleton generation"
  VERBATIM
)

# Generate BPF skeleton header
add_custom_command(
  OUTPUT
    "${BPF_SKEL_HEADER}"
  COMMAND
    "${BPFTOOL}" gen skeleton "${BPF_OBJECT_FILE}" > "${BPF_SKEL_HEADER}"
  DEPENDS
    "${BPF_OBJECT_FILE}"
  COMMENT "Generating BPF skeleton header"
  VERBATIM
)

set_source_files_properties("${BPF_SKEL_HEADER}" PROPERTIES GENERATED TRUE)

# Target for libbpf skeleton generation
add_custom_target(
  generate_bpf_skeleton
  DEPENDS
    "${BPF_SKEL_HEADER}"
)

################################################################################

# Shell scripts
#
lint_shell_script_bundle(
  kernel-collector-scripts
  SOURCES
    entrypoint.sh
    entrypoint-kct.sh
)

# Docker image
#
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  build_custom_docker_image(
    kernel-collector
    OUT_DIR srv
    OUTPUT_OF
      kernel-collector-scripts
      kernel-collector-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
    ARGS
      BUILD_TYPE="Debug"
  )

  build_custom_docker_image(
    kernel-collector-test
    DOCKERFILE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/kernel_collector_test_docker"
    OUT_DIR srv
    ARTIFACTS_OF
      bpf_wire_to_json
      intake_wire_to_json
      kernel_collector_test
    OUTPUT_OF
      kernel-collector-scripts
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    ARGS
      BUILD_TYPE="Debug"
  )
else()
  build_custom_docker_image(
    kernel-collector
    OUT_DIR srv
    OUTPUT_OF
      kernel-collector-scripts
      kernel-collector-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
  )

  build_custom_docker_image(
    kernel-collector-test
    DOCKERFILE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/kernel_collector_test_docker"
    OUT_DIR srv
    ARTIFACTS_OF
      bpf_wire_to_json
      intake_wire_to_json
      kernel_collector_test
    OUTPUT_OF
      kernel-collector-scripts
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
  )
endif()

# Unit Tests
#
add_unit_test(cgroup_handler LIBS agentlib test_channel render_ebpf_net_ingest_writer render_rust_ebpf_net)
add_unit_test(kernel_symbols LIBS agentlib)
add_ebpf_unit_test(kernel_collector LIBS signal_handler agentlib fastpass_util file_ops config_file libuv-static system_ops static-executable test_channel render_ebpf_net_ingest_writer render_rust_ebpf_net)
