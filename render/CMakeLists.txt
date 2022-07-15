
if(FLOWMILL_SUBPROJECT)
  return()
endif()

get_target_property(
  RENDER_COMPILER
   render_compiler LOCATION
)

render_compile(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PACKAGE
    flowmill
  APPS
    agent_internal
    kernel_collector
    cloud_collector
    ingest
  COMPILER
    ${RENDER_COMPILER}
  OUTPUT_DIR
    "${CMAKE_BINARY_DIR}/generated"
  DEPENDS
    render_compiler
)
