find_path(
  CARES_INCLUDE_DIR
    ares.h
  PATHS
    /usr/local/include
    /usr/include
)
if(NOT CARES_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find c-ares, required for DNS. Build container should already have that set up")
endif()
add_library(c-ares-headers INTERFACE)
target_include_directories(c-ares-headers INTERFACE "${CARES_INCLUDE_DIR}")
