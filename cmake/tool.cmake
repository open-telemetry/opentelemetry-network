# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

add_custom_target(tools)

function(add_tool_executable EXEC_NAME)
  cmake_parse_arguments(P "" "" "SRCS;DEPS" ${ARGN})

  add_executable("${EXEC_NAME}" "${P_SRCS}")
  add_dependencies(tools "${EXEC_NAME}")
  install(
    TARGETS "${EXEC_NAME}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
  )

  if(DEFINED P_DEPS)
    target_link_libraries("${EXEC_NAME}" "${P_DEPS}")
  endif()
endfunction(add_tool_executable)
