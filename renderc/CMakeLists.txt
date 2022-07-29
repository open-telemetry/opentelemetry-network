# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


set(
  RENDER_COMPILER
    "${CMAKE_CURRENT_BINARY_DIR}/net.flowmill.render.standalone/build/libs/net.flowmill.render.standalone-1.0.0-SNAPSHOT-all.jar"
)

# Prepare directories
set(
  COMPILER_DIRECTORIES
    "net.flowmill.render"
    "net.flowmill.render.standalone"
)
set(COMPILER_DIRECTORIES_ABS)
foreach(FIL ${COMPILER_DIRECTORIES})
  get_filename_component(FIL_DST_ABS "${FIL}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  add_custom_command(
    OUTPUT
      ${FIL}
    COMMAND
      ${CMAKE_COMMAND} -E make_directory ${FIL_DST_ABS}
    COMMENT
      "Preparing build (creating directory ${FIL})"
  )
  list(APPEND COMPILER_DIRECTORIES_ABS "${FIL_DST_ABS}")
endforeach()
add_custom_target(
  render_compiler_directories
  DEPENDS
    ${COMPILER_DIRECTORIES_ABS}
)

# Prepare links
set(
  COMPILER_FILES
    "build.gradle"
    "gradle"
    "gradlew"
    "settings.gradle"
    "net.flowmill.render/src"
    "net.flowmill.render/META-INF"
    "net.flowmill.render/build.gradle"
    "net.flowmill.render/build.properties"
    "net.flowmill.render.standalone/src"
    "net.flowmill.render.standalone/build.gradle"
    "net.flowmill.render.standalone/plugin.properties"
)
set(COMPILER_FILES_ABS)
foreach(FIL ${COMPILER_FILES})
  get_filename_component(FIL_SRC_ABS "${FIL}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  get_filename_component(FIL_DST_ABS "${FIL}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  add_custom_command(
    OUTPUT
      ${FIL}
    COMMAND
      ${CMAKE_COMMAND} -E create_symlink ${FIL_SRC_ABS} ${FIL_DST_ABS}
    DEPENDS
      render_compiler_directories
    COMMENT
      "Preparing build (linking ${FIL})"
  )
  list(APPEND COMPILER_FILES_ABS "${FIL_DST_ABS}")
endforeach()

add_custom_command(
  OUTPUT
    ${RENDER_COMPILER}
  COMMAND
    ./gradlew --console=plain
    :net.flowmill.render.standalone:shadowJar
  COMMAND
    touch --no-create "${RENDER_COMPILER}"
  DEPENDS
    ./build.gradle
    ./gradle/source-layout.gradle
    ./settings.gradle
    ./net.flowmill.render/build.gradle
    ./net.flowmill.render/build.properties
    ./net.flowmill.render/src/net/flowmill/render/scoping/RenderScopeProvider.xtend
    ./net.flowmill.render/src/net/flowmill/render/RenderRuntimeModule.xtend
    ./net.flowmill.render/src/net/flowmill/render/RenderStandaloneSetup.xtend
    ./net.flowmill.render/src/net/flowmill/render/GenerateRender.mwe2
    ./net.flowmill.render/src/net/flowmill/render/validation/RenderValidator.xtend
    ./net.flowmill.render/src/net/flowmill/render/extensions/AppExtensions.xtend
    ./net.flowmill.render/src/net/flowmill/render/extensions/SpanExtensions.xtend
    ./net.flowmill.render/src/net/flowmill/render/extensions/FieldExtensions.xtend
    ./net.flowmill.render/src/net/flowmill/render/extensions/FieldTypeExtensions.xtend
    ./net.flowmill.render/src/net/flowmill/render/extensions/MessageExtensions.xtend
    ./net.flowmill.render/src/net/flowmill/render/extensions/UtilityExtensions.xtend
    ./net.flowmill.render/src/net/flowmill/render/extensions/XPackedMessageExtensions.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/RenderGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/AppPacker.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/MessageGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/AppGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/RenderFormatter.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/PerfectHash.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/ConnectionGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/ProtocolGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/TransformBuilderGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/BpfGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/WriterGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/generator/SpanGenerator.xtend
    ./net.flowmill.render/src/net/flowmill/render/Render.xtext
    ./net.flowmill.render.standalone/build.gradle
    ./net.flowmill.render.standalone/plugin.properties
    ./net.flowmill.render.standalone/src/Main.xtend
    ${COMPILER_FILES_ABS}
  COMMENT
    "Making Render compiler"
)
add_custom_target(
  render_compiler
  ALL
  DEPENDS
    ${RENDER_COMPILER}
)
set_target_properties(
  render_compiler
  PROPERTIES
    LOCATION ${RENDER_COMPILER}
)
