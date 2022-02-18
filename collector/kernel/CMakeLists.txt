
# Kernel collector executable
#
add_executable(
  kernel-collector
    main.cc
)
target_link_libraries(
  kernel-collector
  PUBLIC
    signal_handler
    agentlib
    agentxxdlib
    fastpass_util
    file_ops
    bcc-interface
    bcc-static
    config_file
    libuv-static
    args_parser
    system_ops
    spdlog
    static-executable
)
target_include_directories(
  kernel-collector
  PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)
install(
  TARGETS
    kernel-collector
  RUNTIME DESTINATION
    bin
)
harden_executable(kernel-collector)
strip_binary(kernel-collector)
add_dependencies(collectors kernel-collector)

# DNS library
#
add_library(
  agentdnslib
  STATIC
    dns/ares_expand_name.c
    dns/ares_parse_a_aaaa_reply.c
    dns/ares_parse_query.c
)
target_link_libraries(
  agentdnslib
    c-ares-headers
)
add_dependencies(agentdnslib render_flowmill_artifacts)
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
    protocols/protocol_handler_base.cc
    protocols/protocol_handler_unknown.cc
    protocols/protocol_handler_http.cc
)
# BPF needs to be newest headers despite what is in the build-env /usr/include
# BCC provides the ones that we should use
target_include_directories(
  agentlib
  SYSTEM
  BEFORE
  PUBLIC
    /root/install/include/bcc/compat/
)
target_link_libraries(
  agentlib
  PUBLIC
    bcc-interface
    render_flowmill_agent_internal_hash
    render_flowmill_ingest_writer
    render_flowmill_kernel_collector
    agentdnslib
    yamlcpp
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
    tls_channel
    proc_ops
    system_ops
    time
    absl::flat_hash_map
    absl::flat_hash_set
    stdc++fs
)
target_compile_options(agentlib PRIVATE -fPIC)
# some agentlib sources include ${BPF_DEBUG_INFO}
add_dependencies(agentlib preprocess_bpf)


################################################################################
# Preprocess BPF Code and Convert to XXD
#

set(BPF_PROCESSED_SOURCE "${CMAKE_BINARY_DIR}/generated/agent_bpf.c")
set(BPF_DEBUG_INFO "${CMAKE_BINARY_DIR}/generated/agent_bpf_debug.inc")

set_source_files_properties("${BPF_PROCESSED_SOURCE}" PROPERTIES GENERATED TRUE)
set_source_files_properties("${BPF_DEBUG_INFO}" PROPERTIES GENERATED TRUE)

set(BPF_DEPENDENCIES
  "${CMAKE_CURRENT_SOURCE_DIR}/bpf_preprocess.py"
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
  "${CMAKE_BINARY_DIR}/generated/flowmill/agent_internal.wire_message.h"
  "${CMAKE_BINARY_DIR}/generated/flowmill/agent_internal/bpf.h"
  "${PROJECT_SOURCE_DIR}/jitbuf/jb.h"
  render_compile_flowmill
)

# bpf_preprocess.py <infile> <outfile> <debugfile>
# Note: in this command we define _PROCESSING_BPF=1 so headers shared between
# bpf and userland can have different code when required
add_custom_command(
  OUTPUT
    "${BPF_PROCESSED_SOURCE}"
    "${BPF_DEBUG_INFO}"
  COMMAND
    python3
    "${CMAKE_CURRENT_SOURCE_DIR}/bpf_preprocess.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/bpf_src/render_bpf.c"
    "${BPF_PROCESSED_SOURCE}"
    "${BPF_DEBUG_INFO}"
    -D _PROCESSING_BPF=1
    -I "${PROJECT_SOURCE_DIR}"
    -I "${CMAKE_CURRENT_SOURCE_DIR}"
    -I "${CMAKE_BINARY_DIR}"
    -I "${PROJECT_BINARY_DIR}"
    -I "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${CMAKE_BINARY_DIR}/generated"
    -I "${CMAKE_BINARY_DIR}/generated/bpf"
    -I "${CMAKE_BINARY_DIR}/generated/flowmill"
    -I "${CMAKE_BINARY_DIR}/generated/flowmill/agent_internal"
    -I "${CMAKE_BINARY_DIR}/generated/bpf/bpf_src"
    -I "${CMAKE_BINARY_DIR}/generated/bpf/bpf_src/tcp-processor"
  DEPENDS "${BPF_DEPENDENCIES}"
  COMMENT "Processing BPF source file"
  VERBATIM
)
add_custom_target(
  preprocess_bpf
  DEPENDS
    "${BPF_PROCESSED_SOURCE}"
    "${BPF_DEBUG_INFO}"
)

# Compiled-in BPF source
#
add_xxd("${BPF_PROCESSED_SOURCE}" BPF_PROCESSED_XXD
  OUTPUT "agent_bpf_xxd.c"
  DEPENDS preprocess_bpf
)
add_library(
  agentxxdlib
  STATIC
    ${BPF_PROCESSED_XXD}
)
target_include_directories(
  agentxxdlib
  PRIVATE
    ${CMAKE_BINARY_DIR}
)

################################################################################

# Shell scripts
#
lint_shell_script_bundle(
  kernel-collector-scripts
  SOURCES
    entrypoint.sh
    kernel_headers.sh
)

# Docker image
#
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  build_custom_docker_image(
    kernel-collector
    OUT_DIR srv
    ARTIFACTS_OF
      kernel-collector
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
endif()

# Unit Tests
#
add_unit_test(cgroup_handler LIBS agentlib test_channel)
add_ebpf_unit_test(kernel_collector LIBS signal_handler agentlib agentxxdlib fastpass_util file_ops bcc-interface bcc-static config_file libuv-static system_ops static-executable test_channel)
