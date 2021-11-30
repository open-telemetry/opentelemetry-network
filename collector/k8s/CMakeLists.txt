setup_go_module(collector "flowmill.net" VENDORED)

build_protobuf(collector GO collector CPP GRPC)
build_protobuf(kubernetes_info GO collector CPP)

#############
# k8s-relay #
#############

add_library(
  kubernetes_owner_kind
  STATIC
    kubernetes_owner_kind.cc
)
target_link_libraries(
  kubernetes_owner_kind
    render_flowmill_artifacts
)

add_library(
  resync_channel
  STATIC
    resync_channel.cc
)
target_link_libraries(
  resync_channel
    render_flowmill_artifacts
)

add_library(
  kubernetes_rpc_server
  STATIC
    kubernetes_rpc_server.cc
)
target_link_libraries(
  kubernetes_rpc_server
    collector-cpp-protobuf
    kubernetes_info-cpp-protobuf
    yamlcpp
    fastpass_util
    render_flowmill_artifacts
)

add_library(
  resync_queue
  STATIC
    resync_queue.cc
)
target_link_libraries(
  resync_queue
    render_flowmill_artifacts
)

add_library(
  resync_processor
  STATIC
    resync_processor.cc
)
target_link_libraries(
  resync_processor
    reconnecting_channel
    libuv-interface
    spdlog
    fastpass_util
    render_flowmill_artifacts
)

add_executable(
  k8s-relay
    main.cc
)
harden_executable(k8s-relay)

add_dependencies(collectors k8s-relay)

target_link_libraries(
  k8s-relay
  PUBLIC
    signal_handler
    resync_processor
    resync_queue
    resync_channel
    kubernetes_rpc_server
    kubernetes_owner_kind
    connection_caretaker
    resource_usage_reporter
    render_flowmill_artifacts
    render_flowmill_ingest_writer
    config_file
    libuv-static
    args_parser
    system_ops
    spdlog
    static-executable
    -Wl,-Bstatic
    libgrpc++_unsecure.a
    libgrpc_unsecure.a
    libcares.a
    -Wl,-Bdynamic
)

set_target_properties(k8s-relay PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(k8s-relay PROPERTIES LINK_SEARCH_END_STATIC 1)

strip_binary(k8s-relay)

lint_shell_script_bundle(
  k8s-relay-scripts
  SOURCES
    entrypoint.sh
)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  build_custom_docker_image(
    k8s-relay
    OUT_DIR srv
    ARTIFACTS_OF
      k8s-relay
    OUTPUT_OF
      k8s-relay-scripts
      k8s-relay-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
  )
else()
  build_custom_docker_image(
    k8s-relay
    OUT_DIR srv
    OUTPUT_OF
      k8s-relay-scripts
      k8s-relay-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
  )
endif()

add_subdirectory(k8s-watcher)
