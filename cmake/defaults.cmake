# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

# Default build type is debug for fast iterations
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# If building inside the container, help CMake find dependencies in /install
if(EXISTS "/install")
  if(NOT DEFINED CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "/install" CACHE PATH "Install prefix" FORCE)
  endif()

  if(NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "/install" CACHE STRING "Prefix search path" FORCE)
  elseif(NOT CMAKE_PREFIX_PATH MATCHES "(^|;)/install(;|$)")
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/install" CACHE STRING "Prefix search path" FORCE)
  endif()

  if(EXISTS "/install/openssl" AND (NOT DEFINED OPENSSL_ROOT_DIR OR OPENSSL_ROOT_DIR STREQUAL ""))
    set(OPENSSL_ROOT_DIR "/install/openssl" CACHE PATH "OpenSSL root" FORCE)
  endif()

  # Ensure pkg-config sees libraries installed under /install
  set(_pkg_paths "/install/lib64/pkgconfig:/install/lib/pkgconfig:/install/usr/lib64/pkgconfig:/install/usr/lib/pkgconfig:/install/share/pkgconfig:/install/usr/share/pkgconfig")
  if(DEFINED ENV{PKG_CONFIG_PATH} AND NOT "$ENV{PKG_CONFIG_PATH}" STREQUAL "")
    set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${_pkg_paths}")
  else()
    set(ENV{PKG_CONFIG_PATH} "${_pkg_paths}")
  endif()
endif()

# Provide explicit package locations commonly installed in this image.
# These hints short-circuit expensive probing inside find_package.
if(NOT DEFINED LLVM_DIR AND EXISTS "/usr/lib/llvm-19/cmake")
  set(LLVM_DIR "/usr/lib/llvm-19/cmake" CACHE PATH "LLVM config dir" FORCE)
endif()
if(NOT DEFINED gRPC_DIR AND EXISTS "/usr/lib/x86_64-linux-gnu/cmake/grpc")
  set(gRPC_DIR "/usr/lib/x86_64-linux-gnu/cmake/grpc" CACHE PATH "gRPC config dir" FORCE)
endif()
if(NOT DEFINED absl_DIR AND EXISTS "/usr/lib/x86_64-linux-gnu/cmake/absl")
  set(absl_DIR "/usr/lib/x86_64-linux-gnu/cmake/absl" CACHE PATH "abseil config dir" FORCE)
endif()

# Make git tolerant to container bind-mount ownership (mirrors build script behavior)
execute_process(
  COMMAND git config --global --add safe.directory ${CMAKE_SOURCE_DIR}
  ERROR_QUIET
  OUTPUT_QUIET
)

# Derive version defaults from version.sh if not provided
set(_need_version OFF)
foreach(v IN ITEMS EBPF_NET_MAJOR_VERSION EBPF_NET_MINOR_VERSION EBPF_NET_PATCH_VERSION EBPF_NET_COLLECTOR_BUILD_NUMBER)
  if(NOT DEFINED ${v} OR ${v} STREQUAL "")
    set(_need_version ON)
  endif()
endforeach()

if(_need_version)
  find_program(BASH_EXECUTABLE bash)
  if(BASH_EXECUTABLE AND EXISTS "${CMAKE_SOURCE_DIR}/version.sh")
    execute_process(
      COMMAND ${BASH_EXECUTABLE} -c "source '${CMAKE_SOURCE_DIR}/version.sh' >/dev/null 2>&1; printf '%s %s %s %s' \"$EBPF_NET_MAJOR_VERSION\" \"$EBPF_NET_MINOR_VERSION\" \"$EBPF_NET_PATCH_VERSION\" \"$EBPF_NET_COLLECTOR_BUILD_NUMBER\""
      OUTPUT_VARIABLE _ver
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
  endif()

  if(DEFINED _ver AND NOT _ver STREQUAL "")
    string(REGEX REPLACE " +" ";" _ver_list "${_ver}")
    list(LENGTH _ver_list _len)
    if(_len GREATER_EQUAL 4)
      list(GET _ver_list 0 _maj)
      list(GET _ver_list 1 _min)
      list(GET _ver_list 2 _pat)
      list(GET _ver_list 3 _bld)
      if(NOT DEFINED EBPF_NET_MAJOR_VERSION OR EBPF_NET_MAJOR_VERSION STREQUAL "")
        set(EBPF_NET_MAJOR_VERSION "${_maj}" CACHE STRING "Major version" FORCE)
      endif()
      if(NOT DEFINED EBPF_NET_MINOR_VERSION OR EBPF_NET_MINOR_VERSION STREQUAL "")
        set(EBPF_NET_MINOR_VERSION "${_min}" CACHE STRING "Minor version" FORCE)
      endif()
      if(NOT DEFINED EBPF_NET_PATCH_VERSION OR EBPF_NET_PATCH_VERSION STREQUAL "")
        set(EBPF_NET_PATCH_VERSION "${_pat}" CACHE STRING "Patch version" FORCE)
      endif()
      if(NOT DEFINED EBPF_NET_COLLECTOR_BUILD_NUMBER OR EBPF_NET_COLLECTOR_BUILD_NUMBER STREQUAL "")
        set(EBPF_NET_COLLECTOR_BUILD_NUMBER "${_bld}" CACHE STRING "Collector build number" FORCE)
      endif()
    endif()
  endif()
endif()

# Final fallback to keep project(VERSION ...) valid
if(NOT DEFINED EBPF_NET_MAJOR_VERSION OR EBPF_NET_MAJOR_VERSION STREQUAL "")
  set(EBPF_NET_MAJOR_VERSION "0" CACHE STRING "Major version" FORCE)
endif()
if(NOT DEFINED EBPF_NET_MINOR_VERSION OR EBPF_NET_MINOR_VERSION STREQUAL "")
  set(EBPF_NET_MINOR_VERSION "0" CACHE STRING "Minor version" FORCE)
endif()
if(NOT DEFINED EBPF_NET_PATCH_VERSION OR EBPF_NET_PATCH_VERSION STREQUAL "")
  set(EBPF_NET_PATCH_VERSION "0" CACHE STRING "Patch version" FORCE)
endif()
if(NOT DEFINED EBPF_NET_COLLECTOR_BUILD_NUMBER OR EBPF_NET_COLLECTOR_BUILD_NUMBER STREQUAL "")
  set(EBPF_NET_COLLECTOR_BUILD_NUMBER "0" CACHE STRING "Collector build number" FORCE)
endif()
