# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/ext/bcc/cmake)

# required libs, See bcc/cmake/clang_libs.cmake
set(
  BCC_CLANG_LIBS
    clangFrontend
    clangSerialization
    clangDriver
    clangASTMatchers
    clangParse
    clangSema
    clangCodeGen
    clangAnalysis
    clangRewrite
    clangEdit
    clangAST
    clangLex
    clangBasic
)

# find the required clang libraries
foreach( LIB ${BCC_CLANG_LIBS} )
  find_library(lib${LIB} NAMES ${LIB} HINTS ${LLVM_LIBRARY_DIRS})
  if(lib${LIB} STREQUAL "lib${LIB}-NOTFOUND")
    message(FATAL_ERROR "Unable to find clang library ${LIB}. Build container should already have that set up")
  endif()
endforeach()

find_path(BCC_INCLUDE_DIRS bcc/libbpf.h)

# BCC libs, see the line that starts with "target_link_libraries(bcc-static"
#  in bcc/src/cc/MakeLists.txt
find_library(BCC_LIBS NAMES "libbcc-combined.a")

set(CMAKE_REQUIRED_INCLUDES ${BCC_INCLUDE_DIRS})
include(CheckIncludeFile)
check_include_file("bcc/bcc_syms.h" FOUND_BCC_SYMS_H)
if ( NOT FOUND_BCC_SYMS_H )
  message ( FATAL_ERROR "Could not find bcc_syms.h while searching for bcc. Build container should already have that set up" )
endif ( NOT FOUND_BCC_SYMS_H )

FOREACH(LIB ${BCC_LIBS})
  if ( NOT EXISTS ${LIB} )
    message ( FATAL_ERROR "Could not find ${LIB}. Build container should already have that set up" )
  endif ( NOT EXISTS ${LIB} )
ENDFOREACH()

message(STATUS "bcc libraries: ${BCC_LIBS}")
message(STATUS "bcc include dirs: ${BCC_INCLUDE_DIRS}")

add_library(bcc-static INTERFACE)
add_library(bcc-interface INTERFACE)
target_include_directories(
  bcc-interface
  INTERFACE
    ${BCC_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS}
)

# BCC LLVM libs, see bcc/src/cc/CMakeLists.txt
set(
  BCC_LLVM_LIBNAMES
    bitwriter
    bpfcodegen
    debuginfodwarf
    irreader
    linker
    mcjit
    objcarcopts
    option
    passes
    lto
    nativecodegen
    coverage
    coroutines
    bpfasmparser
    bpfdisassembler
)
llvm_map_components_to_libnames(BCC_LLVM_LIBS ${BCC_LLVM_LIBNAMES})
llvm_expand_dependencies(BCC_LLVM_LIBS_EXPANDED ${BCC_LLVM_LIBS})

target_compile_definitions(bcc-interface INTERFACE ${LLVM_DEFINITIONS})
target_link_libraries(bcc-static INTERFACE  bcc-interface
                                            ${BCC_LIBS}
                                            ${BCC_LLVM_LIBS_EXPANDED}
                                            ${BCC_CLANG_LIBS}
                                            libelf.a)

# LLVM is built with -ffunctions-sections -fdata-sections so we can remove unused functions
#target_compile_options(bcc-interface INTERFACE -ffunction-sections -fdata-sections)
#target_link_libraries(bcc-static INTERFACE  -Wl,--gc-sections)
