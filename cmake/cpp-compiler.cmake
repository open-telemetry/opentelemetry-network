# resolves the command line option for limiting error output
# this is extremely useful for debugging compilation errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CXX_ERROR_LIMIT_FLAG "-ferror-limit")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CXX_ERROR_LIMIT_FLAG "-fmax-errors")
endif()

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

execute_process(
  COMMAND ${CMAKE_CXX_COMPILER} --version
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE CXX_COMPILER_NATIVE_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "C++ compiler native version string: ${CXX_COMPILER_NATIVE_VERSION}")

# most expressive debugging, and some optimization
# also, disabling -Wno-stringop-truncation where necessary given that it warns about
#   behavior we intend to get out of strncpy
set(EBPF_NET_COMMON_COMPILE_FLAGS "-ggdb3 -Wall -Werror -fno-omit-frame-pointer -Wno-stringop-truncation ${CXX_ERROR_LIMIT_FLAG}=1 -pthread")
set(EBPF_NET_COMMON_C_FLAGS "${EBPF_NET_COMMON_COMPILE_FLAGS}")
set(EBPF_NET_COMMON_CXX_FLAGS "${EBPF_NET_COMMON_COMPILE_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EBPF_NET_COMMON_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EBPF_NET_COMMON_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EBPF_NET_COMMON_LINKER_FLAGS} -static-libgcc -static-libstdc++ -pthread")

if(OPTIMIZE)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

function (harden_executable TARGET)
  target_compile_options(
    ${TARGET}
    PUBLIC
      -Wl,-z,relro,-z,now
      -fstack-protector
      -static-pie
      -fpie
      -fPIE
  )
endfunction()
