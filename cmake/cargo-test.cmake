include_guard()

# Adds a single target to run all Rust tests in the workspace.
# Tests run from the source root, with artifacts under ${CMAKE_BINARY_DIR}/target.
if(NOT TARGET cargo-test)
  add_custom_target(
    cargo-test
    COMMAND
      ${CMAKE_COMMAND} -E chdir ${PROJECT_SOURCE_DIR}
      ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_BINARY_DIR}/target cargo test
    VERBATIM
  )
endif()

