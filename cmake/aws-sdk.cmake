# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

find_package(AWSSDK REQUIRED)
set(AWS_SERVICES ec2 s3 core)
AWSSDK_DETERMINE_LIBS_TO_LINK(AWS_SERVICES AWSSDK_LIBS)

message(STATUS "Found AWS SDK Services: ${AWSSDK_SERVICES}")
message(STATUS "Found AWS SDK Libraries: ${AWSSDK_LIBS}")


add_library(aws-sdk-cpp INTERFACE)
target_link_libraries(
  aws-sdk-cpp
  INTERFACE
    ${AWSSDK_LIBS}
    ${AWSSDK_LIBS}
    crypto
    s2n
    dl
)
target_include_directories(
  aws-sdk-cpp
  INTERFACE
    ${AWSSDK_INCLUDE_DIR}
)
