# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

function(render_compile INPUT_DIR)
  cmake_parse_arguments(ARG "" "OUTPUT_DIR;PACKAGE;COMPILER" "APPS;DEPENDS" ${ARGN})

  if(DEFINED ARG_OUTPUT_DIR)
    set(OUTPUT_DIR ${ARG_OUTPUT_DIR})
  else()
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")
  endif()

  if(DEFINED ARG_PACKAGE)
    set(PACKAGE ${ARG_PACKAGE})
  else()
    set(PACKAGE "default")
  endif()

  if(DEFINED ARG_COMPILER)
    set(RENDER_COMPILER ${ARG_COMPILER})
  else()
    get_property(
      RENDER_COMPILER
      TARGET
        render_compiler
      PROPERTY
        RENDER_COMPILER_PATH
    )
  endif()

  set(RENDER_${PACKAGE}_OUTPUTS "")

  foreach(APP ${ARG_APPS})
    set(
      RENDER_${PACKAGE}_${APP}_DESCRIPTOR
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/descriptor.cc"
    )
    set(
      RENDER_${PACKAGE}_${APP}_HASH
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/hash.c"
    )
    set(
      RENDER_${PACKAGE}_${APP}_WRITER
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/writer.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/writer.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/encoder.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/encoder.cc"
    )
    set(
      RENDER_${PACKAGE}_${APP}_OUTPUTS
        ${RENDER_${PACKAGE}_${APP}_HASH}
        ${RENDER_${PACKAGE}_${APP}_DESCRIPTOR}
        ${RENDER_${PACKAGE}_${APP}_WRITER}
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/index.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/index.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/containers.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/containers.inl"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/containers.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/keys.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/handles.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/handles.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/auto_handles.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/auto_handles.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/weak_refs.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/weak_refs.inl"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/weak_refs.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/modifiers.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/modifiers.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/spans.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/spans.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/span_base.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/connection.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/connection.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/protocol.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/protocol.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/transform_builder.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/transform_builder.cc"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/parsed_message.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/wire_message.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/meta.h"
        "${OUTPUT_DIR}/${PACKAGE}/${APP}/bpf.h"
    )
    list(
      APPEND
      RENDER_${PACKAGE}_OUTPUTS
        ${RENDER_${PACKAGE}_${APP}_OUTPUTS}
    )
  endforeach()

  list(
    APPEND
    RENDER_${PACKAGE}_OUTPUTS
      "${OUTPUT_DIR}/${PACKAGE}/metrics.h"
  )

  set_source_files_properties(
    ${RENDER_${PACKAGE}_OUTPUTS}
    PROPERTIES
      GENERATED TRUE
  )

  file(
    GLOB
    RENDER_INPUTS
      "${INPUT_DIR}/*.render"
  )

  # Generate sources
  #
  add_custom_command(
    OUTPUT
      ${RENDER_${PACKAGE}_OUTPUTS}
    WORKING_DIRECTORY
      ${INPUT_DIR}
    COMMAND
      ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
    COMMAND
      java -jar ${RENDER_COMPILER} -i . -o "${OUTPUT_DIR}"
    DEPENDS
      ${RENDER_INPUTS}
      ${RENDER_COMPILER}
      ${ARG_DEPENDS}
  )
  add_custom_target(
    render_compile_${PACKAGE}
    DEPENDS
      ${RENDER_${PACKAGE}_OUTPUTS}
  )

  # Generated sources interface library
  #
  add_library(
    render_${PACKAGE}_artifacts
    INTERFACE
  )
  target_include_directories(
    render_${PACKAGE}_artifacts
    INTERFACE
      "${OUTPUT_DIR}"
  )
  add_dependencies(
    render_${PACKAGE}_artifacts
      render_compile_${PACKAGE}
  )

  # Generated sources app libraries
  #
  foreach(APP ${ARG_APPS})
    add_library(
      render_${PACKAGE}_${APP}
      STATIC
      EXCLUDE_FROM_ALL
        ${RENDER_${PACKAGE}_${APP}_OUTPUTS}
    )
    target_include_directories(
      render_${PACKAGE}_${APP}
      PUBLIC
        ${OUTPUT_DIR}
    )
    target_link_libraries(
      render_${PACKAGE}_${APP}
      PUBLIC
        fixed_hash
        render_${PACKAGE}_artifacts
    )

    add_library(
      render_${PACKAGE}_${APP}_hash
      STATIC
      EXCLUDE_FROM_ALL
        ${RENDER_${PACKAGE}_${APP}_HASH}
    )
    target_include_directories(
      render_${PACKAGE}_${APP}_hash
      PUBLIC
        ${OUTPUT_DIR}
    )
    target_link_libraries(
      render_${PACKAGE}_${APP}_hash
      PUBLIC
        render_${PACKAGE}_artifacts
    )

    add_library(
      render_${PACKAGE}_${APP}_descriptor
      STATIC
      EXCLUDE_FROM_ALL
        ${RENDER_${PACKAGE}_${APP}_DESCRIPTOR}
    )
    target_include_directories(
      render_${PACKAGE}_${APP}_descriptor
      PUBLIC
        ${OUTPUT_DIR}
    )
    target_link_libraries(
      render_${PACKAGE}_${APP}_descriptor
      PUBLIC
        render_${PACKAGE}_artifacts
    )

    add_library(
      render_${PACKAGE}_${APP}_writer
      STATIC
      EXCLUDE_FROM_ALL
        ${RENDER_${PACKAGE}_${APP}_WRITER}
    )
    target_include_directories(
      render_${PACKAGE}_${APP}_writer
      PUBLIC
        ${OUTPUT_DIR}
    )
    target_link_libraries(
      render_${PACKAGE}_${APP}_writer
      PUBLIC
        render_${PACKAGE}_artifacts
    )
  endforeach()

endfunction()
