# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

get_property(
  RENDER_COMPILER
  TARGET
    render_compiler
  PROPERTY
    RENDER_COMPILER_PATH
)
render_compile(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PACKAGE
    test
  APPS
    app1
  COMPILER
    ${RENDER_COMPILER}
  OUTPUT_DIR
    "${CMAKE_BINARY_DIR}/generated"
  DEPENDS
    render_compiler
)

add_unit_test(render LIBS render_test_app1)
