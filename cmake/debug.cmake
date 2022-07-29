# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

option(UPLOAD_DEBUG_SYMBOLS "When set, debug symbols will be uploaded to our symbol server" OFF)
option(EXPORT_DEBUG_SYMBOLS "When set, debug symbols will be exported even if not being uploaded to our symbol server" OFF)

function(strip_binary TARGET)
  cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

  if (UPLOAD_DEBUG_SYMBOLS)
    list(APPEND STRIP_ARGS "--export")
    list(APPEND STRIP_ARGS "--upload")
    list(APPEND STRIP_ARGS "flowmill-debug-symbols")
  endif()

  add_custom_target(
    ${TARGET}-stripped
    DEPENDS ${ARG_DEPENDS}
    COMMAND
      ${CMAKE_SOURCE_DIR}/dev/strip-symbols.sh ${STRIP_ARGS} $<TARGET_FILE:${TARGET}>
  )

  set_property(
    TARGET
      ${TARGET}-stripped
    PROPERTY
      "OUTPUT" $<TARGET_FILE:${TARGET}>-stripped
  )
endfunction()
