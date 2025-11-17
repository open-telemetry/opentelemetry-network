# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

option(RUN_DOCKER_COMMANDS "when disabled, prepares docker images to be built but stop short of running `docker` commands" ON)

add_custom_target(docker)
add_custom_target(docker-registry)

################
# DOCKER IMAGE #
################

function(build_custom_docker_image IMAGE_NAME)
  cmake_parse_arguments(ARG "" "DOCKERFILE_PATH;DOCKERFILE_NAME;OUT_DIR" "ARGS;IMAGE_TAGS;FILES;BINARIES;DIRECTORIES;DEPENDS;OUTPUT_OF;ARTIFACTS_OF;DOCKER_REGISTRY;DEPENDENCY_OF" ${ARGN})

  # Dockerfile's directory defaults to the one containing CMakeLists.txt
  if (NOT DEFINED ARG_DOCKERFILE_PATH)
    set(ARG_DOCKERFILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  if (NOT DEFINED ARG_DOCKERFILE_NAME)
    set(ARG_DOCKERFILE_NAME "Dockerfile")
  endif()

  if (NOT DEFINED ARG_IMAGE_TAGS)
    if (DEFINED ENV{IMAGE_TAGS})
      set(ARG_IMAGE_TAGS "$ENV{IMAGE_TAGS}")
    else()
      set(ARG_IMAGE_TAGS "latest")
    endif()
  endif()

  if (NOT DEFINED ARG_DOCKER_REGISTRY)
    if (DEFINED ENV{DOCKER_REGISTRY})
      set(ARG_DOCKER_REGISTRY "$ENV{DOCKER_REGISTRY}")
    else()
      set(ARG_DOCKER_REGISTRY "localhost:5000")
    endif()
  endif()

  foreach (FILE ${ARG_FILES})
    if (FILE MATCHES "^[/~]")
      list(APPEND FILES_LIST "${FILE}")
    else()
      list(APPEND FILES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
    endif()
  endforeach()

  foreach (OUTPUT_OF ${ARG_OUTPUT_OF})
    get_target_property(OUTPUT ${OUTPUT_OF} OUTPUT)
    list(APPEND FILES_LIST "${OUTPUT}")
  endforeach()

  # everything in BINARIES is relative to current binary dir
  foreach (BINARY ${ARG_BINARIES})
    list(APPEND BINARIES_LIST "${CMAKE_CURRENT_BINARY_DIR}/${BINARY}")
  endforeach()

  foreach (ARTIFACTS_OF ${ARG_ARTIFACTS_OF})
    list(APPEND FILES_LIST $<TARGET_FILE:${ARTIFACTS_OF}>)
  endforeach()

  set(DOCKER_ARGS "")
  foreach (ARG ${ARG_ARGS})
    list(APPEND DOCKER_ARGS "--build-arg" "${ARG}")
  endforeach()

  set(out_path "${CMAKE_BINARY_DIR}/docker.out/${IMAGE_NAME}")

  if (NOT DEFINED ARG_OUT_DIR)
    set(files_path "${out_path}")
  else()
    set(files_path "${out_path}/${ARG_OUT_DIR}")
  endif()

  ################
  # docker build #
  ################

  add_custom_target(
    "${IMAGE_NAME}-docker"
    DEPENDS
      ${ARG_DEPENDS}
      ${ARG_OUTPUT_OF}
      ${ARG_ARTIFACTS_OF}
  )

  add_dependencies(
    docker
      "${IMAGE_NAME}-docker"
  )

  foreach (DEPENDENT_TARGET ${ARG_DEPENDENCY_OF})
    add_dependencies("${DEPENDENT_TARGET}-docker" "${IMAGE_NAME}-docker")
  endforeach()

  add_custom_command(
    TARGET
      "${IMAGE_NAME}-docker"
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E make_directory "${out_path}"
    COMMAND
      ${CMAKE_COMMAND} -E make_directory "${files_path}"
  )

  add_custom_command(
    TARGET
      "${IMAGE_NAME}-docker"
    POST_BUILD
    WORKING_DIRECTORY
      "${out_path}"
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different ${ARG_DOCKERFILE_PATH}/${ARG_DOCKERFILE_NAME} ${out_path}
  )

  if (DEFINED FILES_LIST)
    add_custom_command(
      TARGET
        "${IMAGE_NAME}-docker"
      POST_BUILD
      WORKING_DIRECTORY
        "${out_path}"
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${FILES_LIST} ${files_path}
    )
  endif()

  if (DEFINED BINARIES_LIST)
    add_custom_command(
      TARGET
        "${IMAGE_NAME}-docker"
      POST_BUILD
      WORKING_DIRECTORY
        "${out_path}"
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${BINARIES_LIST} ${files_path}
    )
  endif()

  foreach (DIRECTORY ${ARG_DIRECTORIES})
    get_filename_component(DIR_NAME ${DIRECTORY} NAME)

    add_custom_command(
      TARGET
        "${IMAGE_NAME}-docker"
      POST_BUILD
      WORKING_DIRECTORY
        "${out_path}"
      COMMAND
        ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}"
          "${files_path}/${DIR_NAME}"
    )
  endforeach()

  if (RUN_DOCKER_COMMANDS)
    add_custom_command(
      TARGET
        "${IMAGE_NAME}-docker"
      POST_BUILD
      WORKING_DIRECTORY
        "${out_path}"
      # Intentionally build with host networking to avoid relying on
      # rootless networking helpers (pasta/slirp4netns) inside nested CI
      # containers. This improves reliability of podman builds in GitHub
      # Actions and similar environments.
    COMMAND
        podman build --network host -t "${IMAGE_NAME}" ${DOCKER_ARGS} -f "${ARG_DOCKERFILE_NAME}" .
    )
  endif()

  ###########################
  # push to docker registry #
  ###########################

  add_custom_target(
    "${IMAGE_NAME}-docker-registry"
    DEPENDS
      docker-registry-login
      "${IMAGE_NAME}-docker"
      ${ARG_DEPENDS}
  )

  add_dependencies(
    docker-registry
      "${IMAGE_NAME}-docker-registry"
  )

  foreach (DEPENDENT_TARGET ${ARG_DEPENDENCY_OF})
    add_dependencies(
      "${DEPENDENT_TARGET}-docker-registry"
        "${IMAGE_NAME}-docker-registry"
    )
  endforeach()

  if (RUN_DOCKER_COMMANDS)
    # TODO: merge docker-registry-push.sh and push_docker_image.sh
    foreach (IMAGE_TAG ${ARG_IMAGE_TAGS})
      add_custom_command(
        TARGET
          "${IMAGE_NAME}-docker-registry"
        POST_BUILD
        COMMAND
          podman tag "${IMAGE_NAME}" "${IMAGE_NAME}:${IMAGE_TAG}"
        COMMAND
          "${CMAKE_SOURCE_DIR}/dev/docker-registry-push.sh"
            "${IMAGE_NAME}" "${IMAGE_TAG}" --no-login "${ARG_DOCKER_REGISTRY}"
      )
    endforeach()
  endif()
endfunction()

###################
# DOCKER REGISRY ##
###################

add_custom_target(
  docker-registry-login
  COMMAND
    "${CMAKE_SOURCE_DIR}/dev/docker-registry-login.sh" --no-vault env
)
