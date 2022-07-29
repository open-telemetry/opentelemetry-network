# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_library(llvm INTERFACE)
add_library(llvm-shared INTERFACE)
add_library(llvm-interface INTERFACE)
target_include_directories(
  llvm-interface
  INTERFACE
    ${LLVM_INCLUDE_DIRS}
)
llvm_map_components_to_libnames(
  LLVM_LIBS
    core
    mcjit
    native
    executionengine
    scalaropts
)
target_compile_definitions(llvm-interface INTERFACE ${LLVM_DEFINITIONS})
target_link_libraries(llvm INTERFACE llvm-interface ${LLVM_LIBS})
target_link_libraries(llvm-shared INTERFACE llvm-interface -L${LLVM_LIBRARY_DIRS} -lLLVM)
