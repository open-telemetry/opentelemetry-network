# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

######
# ASAN
######

option(USE_ADDRESS_SANITIZER "Use Address Sanitizer in compilation" OFF)

add_library(address_sanitizer-static INTERFACE)
add_library(address_sanitizer-shared INTERFACE)

message(STATUS "Address Sanitizer is ${USE_ADDRESS_SANITIZER}")

if (USE_ADDRESS_SANITIZER)
  target_compile_options(
    address_sanitizer-shared
    INTERFACE
      -fsanitize=address
      -U_FORTIFY_SOURCE
      -fno-stack-protector
      -fno-omit-frame-pointer
      -U__SANITIZE_ADDRESS__
  )
  target_compile_definitions(
    address_sanitizer-shared
    INTERFACE
      NDEBUG_SANITIZER
  )

  target_compile_options(
    address_sanitizer-static
    INTERFACE
      -fsanitize=address
      -U_FORTIFY_SOURCE
      -fno-stack-protector
      -fno-omit-frame-pointer
      -U__SANITIZE_ADDRESS__
  )
  target_compile_definitions(
    address_sanitizer-static
    INTERFACE
      NDEBUG_SANITIZER
  )

  target_link_libraries(
    address_sanitizer-shared
    INTERFACE
      -static-libasan
      -static-libstdc++
      -fsanitize=address
  )

  target_link_libraries(
    address_sanitizer-static
    INTERFACE
      -static-libasan
      -static-libstdc++
      -fsanitize=address
  )
endif()

#######
# UBSAN
#######

option(USE_UNDEFINED_BEHAVIOR_SANITIZER "Use Undefined Behavior Sanitizer in compilation" OFF)

add_library(undefined_behavior_sanitizer-static INTERFACE)
add_library(undefined_behavior_sanitizer-shared INTERFACE)

message(STATUS "Undefined Behavior Sanitizer is ${USE_UNDEFINED_BEHAVIOR_SANITIZER}")

if (USE_UNDEFINED_BEHAVIOR_SANITIZER)
  target_compile_options(
    undefined_behavior_sanitizer-shared
    INTERFACE
      -fsanitize=undefined
      -U_FORTIFY_SOURCE
      -fno-stack-protector
      -fno-omit-frame-pointer
  )

  target_compile_options(
    undefined_behavior_sanitizer-static
    INTERFACE
      -fsanitize=undefined
      -U_FORTIFY_SOURCE
      -fno-stack-protector
      -fno-omit-frame-pointer
  )

  target_link_libraries(
    undefined_behavior_sanitizer-shared
    INTERFACE
      -static-libubsan
      -static-libstdc++
      -fsanitize=undefined
  )

  target_link_libraries(
    undefined_behavior_sanitizer-static
    INTERFACE
      -static-libubsan
      -static-libstdc++
      -fsanitize=undefined
  )
endif()
