# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

function(strip_binary TARGET)
  cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

  add_custom_target(
    ${TARGET}-stripped
    DEPENDS ${ARG_DEPENDS}
    COMMAND
      ${CMAKE_SOURCE_DIR}/dev/strip-symbols.sh $<TARGET_FILE:${TARGET}>
  )

  set_property(
    TARGET
      ${TARGET}-stripped
    PROPERTY
      "OUTPUT" $<TARGET_FILE:${TARGET}>-stripped
  )
endfunction()
