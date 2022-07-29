# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

function (add_xxd FIL GENERATED_NAME)
  cmake_parse_arguments(MOD "" "OUTPUT" "DEPENDS" ${ARGN})

  get_filename_component(ABS_FIL ${FIL} ABSOLUTE BASENAME ${CMAKE_CURRENT_SOURCE_DIR})
  get_filename_component(FIL_N ${FIL} NAME)
  if (MOD_OUTPUT)
    set(DST_FILENAME "${CMAKE_BINARY_DIR}/generated/${MOD_OUTPUT}")
  else ()
    set(DST_FILENAME "${CMAKE_BINARY_DIR}/generated/${FIL_N}.xxd")
  endif()
  get_filename_component(FIL_D ${ABS_FIL} DIRECTORY)

  add_custom_command(
    OUTPUT "${DST_FILENAME}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
    COMMAND xxd
    ARGS -i ${FIL_N} > "${DST_FILENAME}.generated"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DST_FILENAME}.generated" "${DST_FILENAME}"
    DEPENDS ${FIL} "${MOD_DEPENDS}"    
    WORKING_DIRECTORY ${FIL_D}
    COMMENT "Generating XXD from ${FIL} -> ${DST_FILENAME}"
    VERBATIM 
  )
  set_source_files_properties("${DST_FILENAME}" PROPERTIES GENERATED TRUE)
  set(${GENERATED_NAME} "${DST_FILENAME}" PARENT_SCOPE)
endfunction()     
