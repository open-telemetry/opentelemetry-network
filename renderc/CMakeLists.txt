# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


set(
  RENDER_COMPILER
    "${CMAKE_CURRENT_BINARY_DIR}/io.opentelemetry.render.standalone/build/libs/io.opentelemetry.render.standalone-1.0.0-SNAPSHOT-all.jar"
)

# Prepare directories
set(
  COMPILER_DIRECTORIES
    "io.opentelemetry.render"
    "io.opentelemetry.render.standalone"
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
    "io.opentelemetry.render/src"
    "io.opentelemetry.render/META-INF"
    "io.opentelemetry.render/build.gradle"
    "io.opentelemetry.render.standalone/src"
    "io.opentelemetry.render.standalone/build.gradle"
    "io.opentelemetry.render.standalone/plugin.properties"
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
    :io.opentelemetry.render.standalone:shadowJar
  COMMAND
    touch --no-create "${RENDER_COMPILER}"
  DEPENDS
    ./build.gradle
    ./gradle/source-layout.gradle
    ./settings.gradle
    ./io.opentelemetry.render/build.gradle
    ./io.opentelemetry.render/src/io/opentelemetry/render/scoping/RenderScopeProvider.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/RenderRuntimeModule.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/RenderStandaloneSetup.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/GenerateRender.mwe2
    ./io.opentelemetry.render/src/io/opentelemetry/render/validation/RenderValidator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/formatting/RenderFormatter.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/extensions/AppExtensions.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/extensions/SpanExtensions.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/extensions/FieldExtensions.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/extensions/FieldTypeExtensions.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/extensions/MessageExtensions.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/extensions/UtilityExtensions.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/extensions/XPackedMessageExtensions.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/RenderGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/MetricsGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/AppPacker.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/MessageGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/AppGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/HashGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/PerfectHash.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/ConnectionGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/ProtocolGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/TransformBuilderGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/BpfGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/RustEncoderGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/RustWireMessageGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/RustCargoGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/WriterGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/SpanGenerator.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/generator/SpanAutoDependencies.xtend
    ./io.opentelemetry.render/src/io/opentelemetry/render/Render.xtext
    ./io.opentelemetry.render.standalone/build.gradle
    ./io.opentelemetry.render.standalone/plugin.properties
    ./io.opentelemetry.render.standalone/src/Main.xtend
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
set_property(
  TARGET
    render_compiler
  PROPERTY
    RENDER_COMPILER_PATH
      "${RENDER_COMPILER}"
)

add_subdirectory(test)
