# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()



find_package(PkgConfig REQUIRED)
pkg_check_modules(protobuf REQUIRED IMPORTED_TARGET GLOBAL protobuf)
pkg_check_modules(grpc_unsecure REQUIRED IMPORTED_TARGET GLOBAL grpc_unsecure)
pkg_check_modules(grpc++ REQUIRED IMPORTED_TARGET GLOBAL grpc++)
pkg_check_modules(grpc++_unsecure REQUIRED IMPORTED_TARGET GLOBAL grpc++_unsecure)

find_package(ZLIB REQUIRED)
target_link_libraries(PkgConfig::protobuf INTERFACE ZLIB::ZLIB)

# Add protobuf as a dependency to grpc++ to ensure proper linking
target_link_libraries(PkgConfig::grpc_unsecure INTERFACE PkgConfig::protobuf)
target_link_libraries(PkgConfig::grpc++ INTERFACE PkgConfig::protobuf)
target_link_libraries(PkgConfig::grpc++_unsecure INTERFACE PkgConfig::protobuf)

find_program(GRPC_CPP_PLUGIN_LOCATION grpc_cpp_plugin REQUIRED)
if(GRPC_CPP_PLUGIN_LOCATION)
    message(STATUS "Found grpc_cpp_plugin: ${GRPC_CPP_PLUGIN_LOCATION}")
else()
    message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()

set(GO_PROTOBUF_ANNOTATIONS_DIR /usr/local/go/src/github.com/grpc-ecosystem/grpc-gateway)
set(GO_PROTOBUF_GOOGLEAPIS_DIR /usr/local/go/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis)

# GO should be given the go-module name associated with the built packages
function (build_protobuf NAME)
  cmake_parse_arguments(ARG "CPP;GRPC;GRPC_GATEWAY" "GO" "DEPENDS;DEPENDENCY_OF" ${ARGN})

  set(TARGET "${NAME}-protobuf")

  add_custom_target(
    "${TARGET}"
    DEPENDS
      ${ARG_DEPENDS}
  )

  set(
    PROTOBUF_ARGS
      -I"${CMAKE_CURRENT_SOURCE_DIR}"
  )

  if (ARG_CPP)
    list(
      APPEND
      PROTOBUF_ARGS
        -I/usr/local/include
        --cpp_out="${CMAKE_CURRENT_BINARY_DIR}/generated"
    )

    add_custom_command(
      TARGET
        "${TARGET}"
      COMMAND
        ${CMAKE_COMMAND} -E make_directory
          "${CMAKE_CURRENT_BINARY_DIR}/generated"
    )

    set(
      GEN_FILES_CPP
        "${CMAKE_CURRENT_BINARY_DIR}/generated/${NAME}.pb.h"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/${NAME}.pb.cc"
    )

    if (ARG_GRPC)
      list(
        APPEND
        PROTOBUF_ARGS
          --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_LOCATION}"
          --grpc_out="${CMAKE_CURRENT_BINARY_DIR}/generated"
      )

      list(
        APPEND
        GEN_FILES_CPP
          "${CMAKE_CURRENT_BINARY_DIR}/generated/${NAME}.grpc.pb.h"
          "${CMAKE_CURRENT_BINARY_DIR}/generated/${NAME}.grpc.pb.cc"
      )
    endif()
  endif()

  if (DEFINED ARG_GO)
    list(
      APPEND
      PROTOBUF_ARGS
        --go_out="${GO_PATH_SRC}"
        --go-grpc_out="${GO_PATH_SRC}"
    )

    add_dependencies(
      "${TARGET}"
        "${ARG_GO}-go-module"
    )

    if (ARG_GRPC)
      list(
        APPEND
        PROTOBUF_ARGS
          -I"${GO_PROTOBUF_ANNOTATIONS_DIR}"
          -I"${GO_PROTOBUF_GOOGLEAPIS_DIR}"
          --grpc-gateway_out="logtostderr=true:${GO_PATH_SRC}"
      )
    endif()
  endif()

  list(
    APPEND
    PROTOBUF_ARGS
      "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.proto"
  )

  add_custom_command(
    OUTPUT
      ${GEN_FILES_CPP}
    COMMAND
      protoc
        ${PROTOBUF_ARGS}
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.proto"
  )

  if (ARG_CPP)
    set(CPP_TARGET "${NAME}-cpp-protobuf")

    set_source_files_properties(
      ${GEN_FILES_CPP}
      PROPERTIES
        GENERATED TRUE
    )

    add_library(
      "${CPP_TARGET}"
      STATIC
        ${GEN_FILES_CPP}
    )

    target_link_libraries(
      "${CPP_TARGET}"
        PkgConfig::protobuf
        PkgConfig::grpc++
    )

    target_include_directories(
      "${CPP_TARGET}"
      PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_dependencies(
      "${CPP_TARGET}"
        "${TARGET}"
    )
  endif()

  if (DEFINED ARG_GO)
    set(GO_TARGET "${NAME}-go-protobuf")

    add_custom_target(
      "${GO_TARGET}"
      DEPENDS
        "${TARGET}"
    )
  endif()

  foreach(DEPENDENCY ${ARG_DEPENDENCY_OF})
    add_dependencies(${DEPENDENCY} ${TARGET})
  endforeach()
endfunction()
