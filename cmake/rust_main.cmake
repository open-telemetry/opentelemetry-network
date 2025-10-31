include_guard()

function(add_rust_main)
  cmake_parse_arguments(ARG "" "TARGET;STRIPPED_TARGET;PACKAGE;BIN_NAME;PROJ_DIR;RUST_BIN_TARGET_DIR;DUMMY_TARGET" "LINK_LIBS" ${ARGN})

  if(NOT DEFINED ARG_TARGET)
    message(FATAL_ERROR "add_rust_main: TARGET is required")
  endif()
  if(NOT DEFINED ARG_STRIPPED_TARGET)
    message(FATAL_ERROR "add_rust_main: STRIPPED_TARGET is required")
  endif()
  if(NOT DEFINED ARG_PACKAGE)
    message(FATAL_ERROR "add_rust_main: PACKAGE is required (Cargo package)")
  endif()
  if(NOT DEFINED ARG_BIN_NAME)
    message(FATAL_ERROR "add_rust_main: BIN_NAME is required (expected executable name)")
  endif()
  if(NOT ARG_LINK_LIBS)
    message(FATAL_ERROR "add_rust_main: LINK_LIBS is required (C++ link libraries)")
  endif()

  if(NOT DEFINED ARG_PROJ_DIR)
    set(ARG_PROJ_DIR "${PROJECT_SOURCE_DIR}")
  endif()
  if(NOT DEFINED ARG_RUST_BIN_TARGET_DIR)
    set(ARG_RUST_BIN_TARGET_DIR "${CMAKE_BINARY_DIR}/target")
  endif()
  if(NOT DEFINED ARG_DUMMY_TARGET)
    set(ARG_DUMMY_TARGET "${ARG_TARGET}-dummy")
  endif()

  # 1) Create a tiny dummy main to force generation of a link.txt for the link line
  set(_dummy_main "${CMAKE_CURRENT_BINARY_DIR}/${ARG_DUMMY_TARGET}_main.cc")
  file(WRITE "${_dummy_main}" "int main(int, char**) { return 0; }\n")

  add_executable(${ARG_DUMMY_TARGET} "${_dummy_main}")
  target_link_libraries(
    ${ARG_DUMMY_TARGET}
    PUBLIC
      ${ARG_LINK_LIBS}
      static-executable
  )

  # Path to dummy's link.txt produced by the generator
  set(_link_file "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${ARG_DUMMY_TARGET}.dir/link.txt")

  # 2) Build the Rust binary via cargo using link hints from the dummy
  set(_rust_bin_path "${ARG_RUST_BIN_TARGET_DIR}/release/${ARG_BIN_NAME}")

  add_custom_command(
    OUTPUT "${_rust_bin_path}"
    COMMAND ${CMAKE_COMMAND}
      -DLINK_FILE=${_link_file}
      -DBIN_DIR=${CMAKE_BINARY_DIR}
      -DPROJ_DIR=${ARG_PROJ_DIR}
      -DRUST_BIN_TARGET_DIR=${ARG_RUST_BIN_TARGET_DIR}
      -DRUST_PACKAGE=${ARG_PACKAGE}
      -P ${CMAKE_SOURCE_DIR}/cmake/cargo_build_rust.cmake
    WORKING_DIRECTORY ${ARG_PROJ_DIR}
    DEPENDS ${ARG_LINK_LIBS}
    VERBATIM
  )

  add_custom_target(
    ${ARG_TARGET}
    DEPENDS "${_rust_bin_path}"
  )

  # 3) Stripped variant
  add_custom_command(
    OUTPUT ${_rust_bin_path}-stripped
    COMMAND ${CMAKE_SOURCE_DIR}/dev/strip-symbols.sh ${_rust_bin_path}
    DEPENDS ${_rust_bin_path}
    VERBATIM
  )
  add_custom_target(
    ${ARG_STRIPPED_TARGET}
    DEPENDS "${_rust_bin_path}-stripped"
  )
  set_property(
    TARGET ${ARG_STRIPPED_TARGET}
    PROPERTY
      "OUTPUT" "${_rust_bin_path}-stripped"
  )

endfunction()
