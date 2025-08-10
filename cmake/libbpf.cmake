# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Find libbpf library and create target

# Since the .pc file has wrong prefix, let's find libbpf manually
# Prefer static library over shared library
find_library(LIBBPF_LIBRARY
  NAMES libbpf.a bpf
  PATHS 
    ${CMAKE_INSTALL_PREFIX}/usr/lib64
    ${CMAKE_INSTALL_PREFIX}/lib64
  NO_DEFAULT_PATH
  REQUIRED
)

find_path(LIBBPF_INCLUDE_DIR
  NAMES bpf/libbpf.h
  PATHS 
    ${CMAKE_INSTALL_PREFIX}/usr/include
    ${CMAKE_INSTALL_PREFIX}/include
  NO_DEFAULT_PATH
  REQUIRED
)

if(LIBBPF_LIBRARY AND LIBBPF_INCLUDE_DIR)
  message(STATUS "Found libbpf: ${LIBBPF_LIBRARY}")
  message(STATUS "Found libbpf headers: ${LIBBPF_INCLUDE_DIR}")
  set(LIBBPF_FOUND TRUE)
else()
  message(FATAL_ERROR "libbpf not found in ${CMAKE_INSTALL_PREFIX}")
endif()

# Create imported target for libbpf
if(NOT TARGET libbpf::libbpf)
  add_library(libbpf::libbpf UNKNOWN IMPORTED)
  set_target_properties(libbpf::libbpf PROPERTIES
    IMPORTED_LOCATION "${LIBBPF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LIBBPF_INCLUDE_DIR}"
  )
  
  # Add dependencies (libelf and zlib as mentioned in .pc file)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBELF REQUIRED libelf)
  pkg_check_modules(ZLIB REQUIRED zlib)
  
  set_property(TARGET libbpf::libbpf APPEND PROPERTY
    INTERFACE_LINK_LIBRARIES "${LIBELF_LIBRARIES}" "${ZLIB_LIBRARIES}"
  )
endif()