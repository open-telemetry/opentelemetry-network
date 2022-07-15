add_executable(
  cloud-collector
    main.cc
    collector.cc
    enumerator.cc
    ingest_connection.cc
)
harden_executable(cloud-collector)

add_dependencies(collectors cloud-collector)

target_compile_options(
  cloud-collector
  PRIVATE
    ${CXX_ERROR_LIMIT_FLAG}=1
)

target_link_libraries(
  cloud-collector
  PUBLIC
    render_flowmill_cloud_collector
    render_flowmill_ingest_writer
    signal_handler
    aws-sdk-cpp
    reconnecting_channel
    connection_caretaker
    resource_usage_reporter
    config_file
    ip_address
    scheduling
    libuv-static
    args_parser
    system_ops
    aws_instance_metadata
    spdlog
    static-executable
)

set_target_properties(
  cloud-collector
  PROPERTIES LINK_FLAGS "-pthread"
)

strip_binary(cloud-collector)

lint_shell_script_bundle(
  cloud-collector-scripts
  SOURCES
    entrypoint.sh
)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  build_custom_docker_image(
    cloud-collector
    OUT_DIR srv
    ARTIFACTS_OF
      cloud-collector
    OUTPUT_OF
      cloud-collector-scripts
      cloud-collector-stripped
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
    cloud-collector
    OUT_DIR srv
    OUTPUT_OF
      cloud-collector-scripts
      cloud-collector-stripped
    BINARIES
      debug-info.conf
    FILES
      ../../NOTICE.txt
      ../../LICENSE.txt
    DEPENDENCY_OF
      collectors
  )
endif()
