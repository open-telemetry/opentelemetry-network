# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

find_package(AWSSDK REQUIRED)
set(AWS_SERVICES ec2 s3 core)
AWSSDK_DETERMINE_LIBS_TO_LINK(AWS_SERVICES AWSSDK_LIBS)

# Unfortunately AWSSDK_DETERMINE_LIBS_TO_LINK will list SSL and Crypto libraries
# simply as "ssl" and "crypto", not using the full path with which the AWS SDK
# was configured inside the build-env.
list(TRANSFORM AWSSDK_LIBS REPLACE "^ssl$" OpenSSL::SSL)
list(TRANSFORM AWSSDK_LIBS REPLACE "^crypto$" OpenSSL::Crypto)

add_library(aws-sdk-cpp INTERFACE)
target_link_libraries(
  aws-sdk-cpp
  INTERFACE
    ${AWSSDK_LIBS}
    ${AWSSDK_LIBS}
    s2n
    dl
)
target_include_directories(
  aws-sdk-cpp
  INTERFACE
    ${AWSSDK_INCLUDE_DIR}
)
