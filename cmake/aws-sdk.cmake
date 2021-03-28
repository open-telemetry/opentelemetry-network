find_package(AWSSDK REQUIRED)
set(AWS_SERVICES ec2 s3 core)
AWSSDK_DETERMINE_LIBS_TO_LINK(AWS_SERVICES AWSSDK_LIBS)
add_library(aws-sdk-cpp INTERFACE)
target_link_libraries(
  aws-sdk-cpp
  INTERFACE
    -L${AWSSDK_LIB_DIR}
    ${AWSSDK_LIBS}
    ${AWSSDK_LIBS}
    curl-static
    z
)
target_include_directories(
  aws-sdk-cpp
  INTERFACE
    ${AWSSDK_INCLUDE_DIR}
)
target_link_libraries(
  aws-sdk-cpp
  INTERFACE
    OpenSSL::Crypto
    -ldl
)
