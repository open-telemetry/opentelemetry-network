# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

option(GO_STATIC_LINK "statically link go binaries" OFF)

set(GO_PATH "${CMAKE_BINARY_DIR}/go-path")
set(GO_PATH_SRC "${GO_PATH}/src")

function(setup_go_module NAME DOMAIN)
  cmake_parse_arguments(ARG "VENDORED" "" "" ${ARGN})

  set(PACKAGE "${DOMAIN}/${NAME}")
  set(TARGET "${NAME}-go-module")

  set(MOD_BUILD_DIR "${GO_PATH}/src/${PACKAGE}")
  set(GO_MOD_FILE "${MOD_BUILD_DIR}/go.mod")

  add_custom_target(
    "${TARGET}"
    DEPENDS
      ${ARG_DEPENDS}
  )

  add_custom_command(
    TARGET
      "${TARGET}"
    COMMAND
      ${CMAKE_COMMAND} -E make_directory
        "${MOD_BUILD_DIR}"
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/go.mod"
        "${GO_MOD_FILE}"
  )

  if (ARG_VENDORED)
    add_custom_command(
      TARGET
        "${TARGET}"
      WORKING_DIRECTORY
        "${MOD_BUILD_DIR}"
      COMMAND
        ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
          "${MOD_BUILD_DIR}/vendor"
    )
  else()
    add_custom_command(
      TARGET
        "${TARGET}"
      WORKING_DIRECTORY
        "${MOD_BUILD_DIR}"
      COMMAND
        env GOPATH="${GO_PATH}"
          go mod download -x
    )
  endif()

  set_property(TARGET "${TARGET}" PROPERTY "MOD_BUILD_DIR" "${MOD_BUILD_DIR}")
  set_property(TARGET "${TARGET}" PROPERTY "GO_MOD_FILE" "${GO_MOD_FILE}")

  foreach(DEPENDENCY ${ARG_DEPENDENCY_OF})
    add_dependencies(${DEPENDENCY} ${TARGET})
  endforeach()
endfunction()

# TODO: ADD STATIC CHECK
# the given go package under previously set up (with `setup_go_module`) go module
# sets the `GO_TARGET` variable in the parent scope with this target's name
function(build_go_package NAME MODULE)
  cmake_parse_arguments(ARG "BINARY;GENERATED;ALL" "" "DEPENDS;DEPENDENCY_OF" ${ARGN})

  string(REPLACE "/" "-" IDENTIFIER "${NAME}")
  set(TARGET "${IDENTIFIER}-go")
  set(GO_TARGET "${TARGET}" PARENT_SCOPE)

  get_target_property(MOD_BUILD_DIR "${MODULE}" MOD_BUILD_DIR)
  get_target_property(GO_MOD_FILE "${MODULE}" GO_MOD_FILE)
  set(BUILD_DIR "${MOD_BUILD_DIR}/${NAME}")
  set(OUT_BINARY "${CMAKE_CURRENT_BINARY_DIR}/${NAME}")

  set(TARGET_OPTIONS)
  if(ARG_ALL)
    list(APPEND TARGET_OPTIONS ALL)
  endif()

  add_custom_target(
    "${TARGET}"
    ${TARGET_OPTIONS}
    DEPENDS
      ${MODULE}
      ${ARG_DEPENDS}
    COMMAND
      ${CMAKE_COMMAND} -E make_directory
        "${BUILD_DIR}"
  )

  if (NOT ARG_GENERATED)
    add_custom_command(
      TARGET
        "${TARGET}"
      COMMAND
        ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}"
          "${BUILD_DIR}"
      #COMMAND
      #  ${CMAKE_COMMAND} -E create_symlink
      #    "${CMAKE_CURRENT_SOURCE_DIR}"
      #    "${BUILD_DIR}"
    )
  endif()

  if (ARG_BINARY)
    set(GO_BUILD_ARGS)
    if (GO_STATIC_LINK)
      list(APPEND GO_BUILD_ARGS CGO_ENABLED=0 GOOS=linux GOARCH=amd64)
    endif()

    add_custom_command(
      TARGET
        "${TARGET}"
      WORKING_DIRECTORY
        "${BUILD_DIR}"
      BYPRODUCTS
        "${OUT_BINARY}"
      COMMAND
        env GOPATH="${GO_PATH}" ${GO_BUILD_ARGS}
          go build -o "${OUT_BINARY}" -modfile "${GO_MOD_FILE}" .
    )

    set_property(
      TARGET
        "${TARGET}"
      PROPERTY
        "OUTPUT" "${OUT_BINARY}"
    )
  endif()

  foreach(DEPENDENCY ${ARG_DEPENDENCY_OF})
    add_dependencies(${DEPENDENCY} ${TARGET})
  endforeach()
endfunction()

function(build_standalone_go_binary NAME MODULE)
  cmake_parse_arguments(ARG "BINARY;GENERATED;ALL" "" "DEPENDS;DEPENDENCY_OF" ${ARGN})
endfunction()
