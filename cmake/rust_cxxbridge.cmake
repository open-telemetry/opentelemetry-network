## Copyright The OpenTelemetry Authors
## SPDX-License-Identifier: Apache-2.0

include_guard()

# add_rust_cxxbridge(<target_name> <rust_src>)
# - Runs `cxxbridge` on the given Rust source to generate a header and C++ source.
# - Generates:
#   - Header: ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/<target_name>.h
#   - Source: ${CMAKE_CURRENT_BINARY_DIR}/<target_name>.cc
# - Produces a STATIC library <target_name> that compiles the generated C++ source
#   and exposes the header directory as a PUBLIC include dir so consumers can
#   include it via `#include <<target_name>.h>`.
function(add_rust_cxxbridge TARGET_NAME RUST_SRC)
  if(NOT TARGET_NAME)
    message(FATAL_ERROR "add_rust_cxxbridge requires a target name")
  endif()

  if(NOT RUST_SRC)
    message(FATAL_ERROR "add_rust_cxxbridge requires a Rust source path")
  endif()

  # Resolve Rust source to an absolute path for dependable dependency tracking.
  if(NOT IS_ABSOLUTE "${RUST_SRC}")
    set(RUST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${RUST_SRC}")
  endif()

  # Find the cxxbridge CLI installed via `cargo install cxxbridge-cmd`.
  find_program(CXXBRIDGE_EXECUTABLE NAMES cxxbridge REQUIRED)

  # Output paths
  set(CXXBRIDGE_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge")
  set(CXXBRIDGE_HEADER "${CXXBRIDGE_OUT_DIR}/${TARGET_NAME}.h")
  set(CXXBRIDGE_CC     "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.cc")

  # Ensure output directory exists at configure time
  file(MAKE_DIRECTORY "${CXXBRIDGE_OUT_DIR}")

  # Generate header
  add_custom_command(
    OUTPUT "${CXXBRIDGE_HEADER}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CXXBRIDGE_OUT_DIR}"
    COMMAND sh -lc "\"${CXXBRIDGE_EXECUTABLE}\" \"${RUST_SRC}\" --header > \"${CXXBRIDGE_HEADER}\""
    DEPENDS "${RUST_SRC}"
    COMMENT "Generating cxxbridge header ${CXXBRIDGE_HEADER} from ${RUST_SRC}"
    VERBATIM
  )

  # Generate C++ source (depends on header so include dir exists and header is fresh)
  add_custom_command(
    OUTPUT "${CXXBRIDGE_CC}"
    COMMAND sh -lc "\"${CXXBRIDGE_EXECUTABLE}\" \"${RUST_SRC}\" > \"${CXXBRIDGE_CC}\""
    DEPENDS "${RUST_SRC}" "${CXXBRIDGE_HEADER}"
    COMMENT "Generating cxxbridge source ${CXXBRIDGE_CC} from ${RUST_SRC}"
    VERBATIM
  )

  # Build a static library from the generated C++ and expose the header dir
  add_library(${TARGET_NAME} STATIC "${CXXBRIDGE_CC}")
  target_include_directories(${TARGET_NAME} PUBLIC "${CXXBRIDGE_OUT_DIR}")
endfunction()

