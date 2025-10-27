cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED LINK_FILE)
  message(FATAL_ERROR "LINK_FILE not provided to cargo_build_rust.cmake")
endif()
if(NOT DEFINED BIN_DIR)
  message(FATAL_ERROR "BIN_DIR not provided to cargo_build_rust.cmake")
endif()
if(NOT DEFINED PROJ_DIR)
  message(FATAL_ERROR "PROJ_DIR not provided to cargo_build_rust.cmake")
endif()
if(NOT DEFINED RUST_BIN_TARGET_DIR)
  message(FATAL_ERROR "RUST_BIN_TARGET_DIR not provided to cargo_build_rust.cmake")
endif()
if(NOT DEFINED RUST_PACKAGE)
  message(FATAL_ERROR "RUST_PACKAGE not provided to cargo_build_rust.cmake")
endif()

# Read the CMake-generated link command for the dummy/existing C++ target
file(READ "${LINK_FILE}" LINK_CONTENT)
get_filename_component(LINK_DIR "${LINK_FILE}" DIRECTORY)
# Derive the target binary directory (two levels up from CMakeFiles/<target>.dir)
get_filename_component(_cmakefiles_dir "${LINK_DIR}" DIRECTORY)
get_filename_component(TARGET_BIN_DIR "${_cmakefiles_dir}" DIRECTORY)

# Seed library search paths with known build output dirs and system dirs
set(SEARCH_DIRS
  "${BIN_DIR}/collector/kernel"
  "${BIN_DIR}/collector"
  "${TARGET_BIN_DIR}"
  "${BIN_DIR}/render"
  "${BIN_DIR}/channel"
  "${BIN_DIR}/config"
  "${BIN_DIR}/platform"
  "${BIN_DIR}/scheduling"
  "${BIN_DIR}/util"
  "${BIN_DIR}/otlp"
  "${BIN_DIR}/geoip"
  "${BIN_DIR}/reducer"
  "${BIN_DIR}/reducer/util"
  "/install/lib"
  "/install/usr/lib64"
  "/usr/lib/x86_64-linux-gnu"
)

# Extract -L entries from link line
string(REGEX MATCHALL "-L([^ \t\n]+)" LFLAGS "${LINK_CONTENT}")
foreach(LF IN LISTS LFLAGS)
  string(REGEX REPLACE "^-L" "" LF_PATH "${LF}")
  list(APPEND SEARCH_DIRS "${LF_PATH}")
endforeach()

# Extract directories of static/shared libraries present on the link line
# Include both absolute and relative paths; resolve relatives against LINK_DIR.
string(REGEX MATCHALL "([^ \t\n]*lib[^ \t\n]+\\.(a|so)(\\.[0-9.]+)?)" ALL_LIB_PATHS "${LINK_CONTENT}")
foreach(LP IN LISTS ALL_LIB_PATHS)
  set(LIB_DIR "${LP}")
  if(NOT IS_ABSOLUTE "${LIB_DIR}")
    get_filename_component(LIB_DIR "${LINK_DIR}/${LIB_DIR}" DIRECTORY)
  else()
    get_filename_component(LIB_DIR "${LIB_DIR}" DIRECTORY)
  endif()
  list(APPEND SEARCH_DIRS "${LIB_DIR}")
endforeach()

list(REMOVE_DUPLICATES SEARCH_DIRS)

# Build library list specs
set(LIB_SPECS)

# 1) Static libs by path (.a), absolute or relative
string(REGEX MATCHALL "([^ \t\n]*lib[^ \t\n]+\\.a)" ANY_A "${LINK_CONTENT}")
foreach(LIB IN LISTS ANY_A)
  get_filename_component(FNAME "${LIB}" NAME)
  string(REGEX REPLACE "^lib" "" NAME_NO_PREFIX "${FNAME}")
  string(REGEX REPLACE "\\.a$" "" NAME_NO_EXT "${NAME_NO_PREFIX}")
  if(NOT NAME_NO_EXT STREQUAL "encoder_ebpf_net_all")
    list(APPEND LIB_SPECS "static=${NAME_NO_EXT}")
  endif()
endforeach()

# 2) Shared libs by absolute path (.so)
string(REGEX MATCHALL "(/[^ \t\n]*/lib[^ \t\n]+\\.so(\\.[0-9.]+)?)" ABS_SO "${LINK_CONTENT}")
foreach(LIB IN LISTS ABS_SO)
  get_filename_component(FNAME "${LIB}" NAME)
  string(REGEX REPLACE "^lib" "" NAME_NO_PREFIX "${FNAME}")
  string(REGEX REPLACE "\\.so(\\..*)?$" "" NAME_NO_EXT "${NAME_NO_PREFIX}")
  list(APPEND LIB_SPECS "dylib=${NAME_NO_EXT}")
endforeach()

# 3) -l flags (match standalone tokens only, not substrings like -static-libgcc or paths)
string(REGEX MATCHALL "(^|[ \t\n])-l[A-Za-z0-9_+\-]+" LFLAG_MATCHES "${LINK_CONTENT}")
foreach(TOK IN LISTS LFLAG_MATCHES)
  string(STRIP "${TOK}" TOK_CLEAN)
  string(REGEX REPLACE "^(-l)" "" LNAME "${TOK_CLEAN}")
  list(APPEND LIB_SPECS "dylib=${LNAME}")
endforeach()

# Ensure C++ runtime bits
list(APPEND LIB_SPECS "dylib=stdc++" "dylib=gcc_s")

# Deduplicate while preserving first occurrence
list(REMOVE_DUPLICATES LIB_SPECS)

# Linker args for static group ordering
set(LINK_ARGS "-Wl,--start-group;-Wl,--end-group")

# Compose env var strings
string(JOIN ":" OTN_LINK_SEARCH ${SEARCH_DIRS})
string(JOIN ";" OTN_LINK_LIBS ${LIB_SPECS})
set(OTN_LINK_ARGS "${LINK_ARGS}")

message(STATUS "OTN_LINK_SEARCH=${OTN_LINK_SEARCH}")
message(STATUS "OTN_LINK_LIBS=${OTN_LINK_LIBS}")

# Escape semicolons for passing via CMake -E env
string(REPLACE ";" "\\;" OTN_LINK_LIBS_ESC "${OTN_LINK_LIBS}")
string(REPLACE ";" "\\;" OTN_LINK_ARGS_ESC "${OTN_LINK_ARGS}")

execute_process(
  COMMAND ${CMAKE_COMMAND} -E env
    CARGO_TARGET_DIR=${RUST_BIN_TARGET_DIR}
    OTN_LINK_SEARCH=${OTN_LINK_SEARCH}
    OTN_LINK_LIBS=${OTN_LINK_LIBS_ESC}
    OTN_LINK_ARGS=${OTN_LINK_ARGS_ESC}
    cargo build --release --package ${RUST_PACKAGE} --manifest-path ${PROJ_DIR}/Cargo.toml
  WORKING_DIRECTORY ${PROJ_DIR}
  RESULT_VARIABLE CARGO_RES
  OUTPUT_VARIABLE CARGO_OUT
  ERROR_VARIABLE CARGO_ERR
)

if(NOT CARGO_RES EQUAL 0)
  message(STATUS "Cargo stdout:\n${CARGO_OUT}")
  message(STATUS "Cargo stderr:\n${CARGO_ERR}")
  message(FATAL_ERROR "Cargo build failed with exit code ${CARGO_RES}")
endif()
