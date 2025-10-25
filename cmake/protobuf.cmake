# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()



find_package(Protobuf REQUIRED)
# Avoid the very slow pkg-config-based RE2 lookup inside gRPC's Findre2.cmake
# by predefining the imported target when possible. This short-circuits the
# module and prevents multiple costly pkg-config invocations.
if(NOT TARGET re2::re2)
  find_library(_RE2_LIB NAMES re2)
  find_path(_RE2_INCLUDE_DIR NAMES re2/re2.h)
  if(_RE2_LIB AND _RE2_INCLUDE_DIR)
    add_library(re2::re2 INTERFACE IMPORTED)
    set_property(TARGET re2::re2 PROPERTY INTERFACE_LINK_LIBRARIES "${_RE2_LIB}")
    set_property(TARGET re2::re2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${_RE2_INCLUDE_DIR}")
    message(STATUS "Using preconfigured RE2 (no pkg-config): ${_RE2_LIB}")
  endif()
endif()
find_package(gRPC CONFIG REQUIRED)

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

  set(TARGET_PREPARE "${NAME}-protobuf-prepare")
  set(TARGET "${NAME}-protobuf")

  add_custom_target(
    "${TARGET_PREPARE}"
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
        "${TARGET_PREPARE}"
      POST_BUILD
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
    get_target_property(MOD_BUILD_DIR "${ARG_GO}-go-module" MOD_BUILD_DIR)
    
    list(
      APPEND
      PROTOBUF_ARGS
        --go_out="${GO_PATH_SRC}"
        --go-grpc_out="${GO_PATH_SRC}"
    )

    add_dependencies(
      "${TARGET_PREPARE}"
        "${ARG_GO}-go-module"
    )

    set(
      GEN_FILES_GO
        "${MOD_BUILD_DIR}/${NAME}.pb.go"
        "${MOD_BUILD_DIR}/${NAME}_grpc.pb.go"
    )

    if (ARG_GRPC)
      list(
        APPEND
        PROTOBUF_ARGS
          -I"${GO_PROTOBUF_ANNOTATIONS_DIR}"
          -I"${GO_PROTOBUF_GOOGLEAPIS_DIR}"
          --grpc-gateway_out="logtostderr=true:${GO_PATH_SRC}"
      )

      list(
        APPEND
        GEN_FILES_GO
          "${MOD_BUILD_DIR}/${NAME}.pb.gw.go"
      )
    endif()
  endif()

  list(
    APPEND
    PROTOBUF_ARGS
      "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.proto"
  )

  set(GEN_FILES_ALL ${GEN_FILES_CPP})
  if (DEFINED ARG_GO)
    list(APPEND GEN_FILES_ALL ${GEN_FILES_GO})
  endif()

  add_custom_command(
    OUTPUT
      ${GEN_FILES_ALL}
    COMMAND
      protoc
        ${PROTOBUF_ARGS}
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.proto"
      ${TARGET_PREPARE}
  )

  add_custom_target(
    "${TARGET}"
    DEPENDS
      ${GEN_FILES_ALL}
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
        protobuf::libprotobuf
        gRPC::grpc++
    )

    target_include_directories(
      "${CPP_TARGET}"
      PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}"
    )

  endif()

  if (DEFINED ARG_GO)
    set(GO_TARGET "${NAME}-go-protobuf")

    set_source_files_properties(
    ${GEN_FILES_GO}
    PROPERTIES
      GENERATED TRUE
    )

    add_custom_target(
      "${GO_TARGET}"
      DEPENDS
        ${GEN_FILES_GO}
    )
  endif()

  foreach(DEPENDENCY ${ARG_DEPENDENCY_OF})
    add_dependencies(${DEPENDENCY} ${TARGET_PREPARE})
  endforeach()
endfunction()
