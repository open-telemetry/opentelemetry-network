# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required (VERSION 3.5)

project (opentelemetry-ebpf-build-tools VERSION 0.1.0)

# Architecture:
#
# The build is comprised of multiple directories, each making one Docker image.
# Each such directory can use other docker images as substrates or for artefacts.
# The build system should rebuild a container when:
# 1. There was a more recent commit into the directory, or
# 2. One of the dependencies was rebuilt, or
# 3. The build was cleaned, or
# 4. docker doesn't have the images (e.g., because a dev explicitly erased it)
#
# We maintain two files for each directory inside the build-status directory:
# A. `missing`: touch'd if docker doesn't already have an image for the most recent
#    commit into the directory
# B. `built`: touch'd when we've successfully created a docker image
#
# Each directory's docker build depends on its `missing` and the `built` of its dependencies,
# and outputs (touch's) its own `built`.
#
# On every run, the build system always checks if the most recent commit to the directory is in
# docker, and if not touches `missing`. This handles (1), and (4). Furthermore, if `missing`
# is itself not on the filesystem, then it is touched -- this handles (3). When one of the
# dependencies is rebuilt, this causes a rebuild of the container, solving (2).

set(STATUS_DIR "build-status")

# a dummy target to force checks against docker
add_custom_command(
	OUTPUT "dummy_target_to_force_rebuild"
	COMMAND true
)

include(ProcessorCount)
ProcessorCount(NPROCS)

if(${NPROCS} GREATER 1)
  # don't use up all the cores, leave at least one for other processes and
  # scheduling to avoid trashing
  math(EXPR NPROCS "${NPROCS} - 1")
endif()
message(STATUS "using ${NPROCS} parallel jobs to build")

option(BENV_UNMINIMIZE "whether or not to unminimize the benv image" OFF)

function(build_directory NAME)
	cmake_parse_arguments(P "" "" "DEPENDS" ${ARGN})

	# the missing filename
	set(MISSING_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/${STATUS_DIR}/${NAME}/missing)

	# update the `missing` file
	add_custom_target(check_missing_${NAME}
		# OUTPUT
		# 	${MISSING_FILENAME} # our real output
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${STATUS_DIR}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${STATUS_DIR}/${NAME}
		COMMAND ${CMAKE_SOURCE_DIR}/check_missing.sh ${NAME} ${MISSING_FILENAME}
		# DEPENDS 			"dummy_target_to_force_rebuild" # fake, to force the check to always run
	)


	# for each dependent directory, make command line parameters to pass docker the
	#   directory's resulting docker tag
	set(DOCKER_PARAMS) # the image tags for the dependencies to pass to docker
	set(DEPENDS_FILES) # the `built` files of the dependencies
	list(APPEND DOCKER_PARAMS "--build-arg" "NPROC=${NPROCS}")

	foreach(DEP ${P_DEPENDS})
		list(APPEND DOCKER_PARAMS "--build-arg" "${DEP}_IMAGE_TAG=$$(${CMAKE_SOURCE_DIR}/get_tag.sh" "${DEP})")
		list(APPEND DEPENDS_FILES "${CMAKE_CURRENT_BINARY_DIR}/${STATUS_DIR}/${DEP}/built")
	endforeach()

	if(DEFINED ENV{BENV_BASE_IMAGE_DISTRO})
		message(STATUS "using $ENV{BENV_BASE_IMAGE_DISTRO} as base image distro for ${NAME}")
		list(APPEND DOCKER_PARAMS "--build-arg" "BENV_BASE_IMAGE_DISTRO=$ENV{BENV_BASE_IMAGE_DISTRO}")
	endif()

	if(DEFINED ENV{BENV_BASE_IMAGE_VERSION})
		message(STATUS "using $ENV{BENV_BASE_IMAGE_VERSION} as base image version for ${NAME}")
		list(APPEND DOCKER_PARAMS "--build-arg" "BENV_BASE_IMAGE_VERSION=$ENV{BENV_BASE_IMAGE_VERSION}")
	endif()

	if(BENV_UNMINIMIZE)
		list(APPEND DOCKER_PARAMS "--build-arg" "BENV_UNMINIMIZE=true")
	endif()

	list(APPEND DOCKER_PARAMS "--build-arg" "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		list(APPEND DOCKER_PARAMS "--build-arg" "RESTRICTED_NPROC='1'")
		list(APPEND DOCKER_PARAMS "--build-arg" "BUILD_CFLAGS='-O0 -g'")
		list(APPEND DOCKER_PARAMS "--build-arg" "CONFIGURE_ENABLE_DEBUG='--enable-debug'")
		list(APPEND DOCKER_PARAMS "--build-arg" "CONFIGURE_DEBUG='--debug'")
		list(APPEND DOCKER_PARAMS "--build-arg" "CONFIGURE_RELEASE_DEBUG='--debug'")
	else()
		list(APPEND DOCKER_PARAMS "--build-arg" "RESTRICTED_NPROC=${NPROCS}")
		list(APPEND DOCKER_PARAMS "--build-arg" "GRPC_BUILD_CFLAGS='-Wno-error=class-memaccess -Wno-error=ignored-qualifiers -Wno-error=stringop-truncation'")
		list(APPEND DOCKER_PARAMS "--build-arg" "CONFIGURE_RELEASE_DEBUG='--release'")
	endif()

	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${STATUS_DIR}/${NAME}/built
		COMMAND docker build -t "$$(${CMAKE_SOURCE_DIR}/get_tag.sh" "${NAME})" ${DOCKER_PARAMS} ${CMAKE_SOURCE_DIR}/${NAME} 
		COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/${STATUS_DIR}/${NAME}/built
		DEPENDS ${P_DEPENDS} check_missing_${NAME} ${MISSING_FILENAME} ${DEPENDS_FILES}
	)

	add_custom_target(${NAME} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${STATUS_DIR}/${NAME}/built)
endfunction(build_directory)

build_directory(base)
build_directory(openssl DEPENDS base)
build_directory(curl DEPENDS base openssl)
build_directory(libuv DEPENDS base)
build_directory(aws_sdk DEPENDS base openssl curl)
build_directory(cpp_misc DEPENDS base)
build_directory(grpc_cpp DEPENDS base abseil_cpp openssl)
build_directory(gcp_cpp DEPENDS base openssl curl grpc_cpp)
build_directory(abseil_cpp DEPENDS base)
build_directory(libmaxminddb DEPENDS base)
build_directory(libbpf DEPENDS base)
#gen:dep-dir

build_directory(
  final
  DEPENDS
    base
    openssl
    curl
    libuv
    aws_sdk
    cpp_misc
    grpc_cpp
    abseil_cpp
    libmaxminddb
    gcp_cpp
    libbpf
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_custom_target(benv ALL DEPENDS final
		COMMAND docker tag "$$(${CMAKE_SOURCE_DIR}/get_tag.sh" "final)" debug-build-env)
else()
	add_custom_target(debug-benv ALL DEPENDS final
		COMMAND docker tag "$$(${CMAKE_SOURCE_DIR}/get_tag.sh" "final)" build-env)
endif()
