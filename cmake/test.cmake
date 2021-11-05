include(ExternalProject)

set(gtest_URL https://github.com/google/googletest.git)
set(gtest_TAG "release-1.10.0")

# Directories
set(gtest_LIB_DIR ${CMAKE_BINARY_DIR}/googletest/src/googletest/lib)
set(gmock_LIB_DIR ${CMAKE_BINARY_DIR}/googletest/src/googletest/lib)
set(gtest_INC_DIR ${CMAKE_BINARY_DIR}/googletest/src/googletest/googletest/include)
set(gmock_INC_DIR ${CMAKE_BINARY_DIR}/googletest/src/googletest/googlemock/include)

# Outputs
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(gtest_LIB ${gtest_LIB_DIR}/libgtestd.a)
  set(gtest_MAIN_LIB ${gtest_LIB_DIR}/libgtest_maind.a)
  set(gmock_LIB ${gmock_LIB_DIR}/libgmockd.a)
  set(gmock_MAIN_LIB ${gmock_LIB_DIR}/libgmock_maind.a)
else()
  set(gtest_LIB ${gtest_LIB_DIR}/libgtest.a)
  set(gtest_MAIN_LIB ${gtest_LIB_DIR}/libgtest_main.a)
  set(gmock_LIB ${gmock_LIB_DIR}/libgmock.a)
  set(gmock_MAIN_LIB ${gmock_LIB_DIR}/libgmock_main.a)
endif()

ExternalProject_Add(
  googletest
    PREFIX "${CMAKE_BINARY_DIR}/googletest"
    GIT_REPOSITORY ${gtest_URL}
    GIT_TAG ${gtest_TAG}
    DOWNLOAD_DIR "${DOWNLOAD_LOCATION}"
    BUILD_IN_SOURCE 1
    BUILD_BYPRODUCTS ${gtest_LIB} ${gtest_MAIN_LIB} ${gmock_LIB} ${gmock_MAIN_LIB}
    INSTALL_COMMAND ""
    CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_GMOCK:BOOL=ON
        -DINSTALL_GTEST:BOOL=OFF
)

enable_testing()
include_directories(${gtest_INC_DIR})

# This is for tests that aren't run as part of `make test`. These are useful
# for manual or component tests that you don't want to necessarily run as
# part of the unit test suite.
function(add_standalone_gtest testName)
  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SRCS DEPS)
  cmake_parse_arguments(add_standalone_gtest
      "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${testName}
                 ${add_standalone_gtest_SRCS})
  target_include_directories(${testName} PRIVATE
      ${gtest_INC_DIR} ${gmock_INC_DIR})
  target_link_libraries(${testName}
      ${gmock_MAIN_LIB} ${gtest_LIB} ${gmock_LIB} "-pthread"
      ${add_standalone_gtest_DEPS})
  add_dependencies(${testName} googletest)
endfunction (add_standalone_gtest)

# This function is use for gtest/gmock-dependent libraries for use in other
# unit tests (i.e. does not link a `main` symbol).
function(add_gtest_lib testName)
  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SRCS DEPS)
  cmake_parse_arguments(add_gtest_lib
      "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_library(${testName}
              ${add_gtest_lib_SRCS})
  target_include_directories(${testName} PRIVATE
      ${gtest_INC_DIR} ${gmock_INC_DIR})
  target_link_libraries(${testName}
      ${gtest_LIB} ${gmock_LIB} "-pthread" ${add_gtest_lib_DEPS})
  add_dependencies(${testName} googletest)
endfunction (add_gtest_lib)

# Adds a unit test named `${NAME}_test`.
#
# The file `${NAME}_test.cc` is implicitly added as a source.
# Additional source files can be declared with the `SRCS` parameter.
#
# Test executable is implicitly linked with `gtest` and `gmock`.
# Additional libraries can be linked with the `LIBS` parameter.
#
# Additional dependencies can be declares with the `DEPS` parameter.
function(add_cpp_test NAME)
  set(TEST_NAME "${NAME}_test")
  cmake_parse_arguments(ARG "" "" "SRCS;LIBS;DEPS" ${ARGN})

  add_executable(
    ${TEST_NAME}
      "${TEST_NAME}.cc"
      ${ARG_SRCS}
  )
  add_test(${TEST_NAME} ${TEST_NAME})

  target_include_directories(
    ${TEST_NAME}
    PRIVATE
      ${gtest_INC_DIR}
      ${gmock_INC_DIR}
  )
  target_link_libraries(
    ${TEST_NAME}
      ${ARG_LIBS}
      ${gmock_MAIN_LIB}
      ${gtest_LIB}
      ${gmock_LIB}
      shared-executable
  )

  add_dependencies(
    ${TEST_NAME}
      ${ARG_DEPS}
      googletest
  )
endfunction(add_cpp_test)

# Adds a unit test named `${NAME}_test`, which is part of the `unit_tests` target.
#
# The file `${NAME}_test.cc` is implicitly added as a source.
# Additional source files can be declared with the `SRCS` parameter.
#
# Test executable is implicitly linked with `gtest` and `gmock`.
# Additional libraries can be linked with the `LIBS` parameter.
#
# Additional dependencies can be declares with the `DEPS` parameter.
add_custom_target(unit_tests)
function(add_unit_test NAME)
  cmake_parse_arguments(ARG "" "" "SRCS;LIBS;DEPS" ${ARGN})
  add_cpp_test(${NAME} SRCS ${ARG_SRCS} LIBS ${ARG_LIBS} DEPS ${ARG_DEPS})
  add_dependencies(unit_tests "${NAME}_test")
endfunction(add_unit_test)
