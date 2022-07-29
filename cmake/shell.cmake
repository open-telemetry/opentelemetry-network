# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Runs linters on the given set of shell scripts.
#
# The scripts are bundled as a single cmake target.
#
# Positional Arguments:
#   1. TARGET:
#     The name of the target to create for the bundle.
#
# Named Arguments:
#   SOURCES:
#     The list of scripts to add to the bundle, relative to the current source directory.
#
#   DEPENDS:
#     The list of targets this bundle explicitly depends on.
#
# Output:
#   The list of files linted (`SOURCES`) is exposed through the property `OUTPUT` and can be
#   accessed with the expression `$<TARGET_PROPERTY:TARGET,OUTPUT>`, where `TARGET` is the
#   target name given to the bundle. E.g.: `SET(script_list $<TARGET_PROPERTY:my_bundle,OUTPUT>)`.
#
# Usage:
#   lint_shell_script_bundle(
#     my_target
#     SOURCES
#       my_script_1.sh
#       my_script_2.sh
#     DEPENDS
#       some_dependency_1
#       some_dependency_2
#   )
function(lint_shell_script_bundle TARGET)
  cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDS" ${ARGN})

  add_custom_target(
    ${TARGET}
    ALL
    DEPENDS ${ARG_DEPENDS}
  )

  foreach(SOURCE ${ARG_SOURCES})
    add_custom_command(
      TARGET
        ${TARGET}
      WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}"
      COMMAND
        shellcheck -x "${SOURCE}"
    )
    list(APPEND OUTPUT_LIST "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}")
  endforeach()

  set_property(TARGET ${TARGET} PROPERTY "OUTPUT" ${OUTPUT_LIST})
endfunction()
