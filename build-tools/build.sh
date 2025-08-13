#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Builds the build environment -- a container image which is then used
# to build the project in the main repo.

# Call with `VERBOSE=1` for verbose output
# e.g.: `./build.sh VERBOSE=1`

# use env variables BENV_BASE_IMAGE_DISTRO and BENV_BASE_IMAGE_VERSION
# to customize the base image used to build benv:
#
#   BENV_BASE_IMAGE_DISTRO=debian BENV_BASE_IMAGE_VERSION=testing ./build.sh

# use command line argument -DBENV_UNMINIMIZE=ON to unminimize the benv image

# Note: the `--jobs` flag requires git 2.8.0 or later

# Call with 'debug' to build a debug version of the build-env

if [[ "$1" == "--help" ]]; then
  echo "usage: $0 [{--help | debug}]"
  echo
  echo "  --help: shows this help message"
  echo "  debug: builds benv with debug builds of 3rd party libraries"
  exit 0
fi

set -x

nproc="$(./nproc.sh)"
git submodule update --init --recursive --jobs "${nproc}"

# If this is a debug build set some extra flags to rename the benv image
if [[ "$1" == "debug" ]]; then
  # Debug build
  echo Enabling debug build
  EXTRA_CMAKE_OPTIONS=-DCMAKE_BUILD_TYPE=Debug
  export DOCKER_TAG_PREFIX=debug-
  shift
  mkdir -p Debug
  cd Debug
  CMAKE_SOURCE_DIR=..
else
  # Release build
  echo Enabling release build
  EXTRA_CMAKE_OPTIONS=-DCMAKE_BUILD_TYPE=Release
  mkdir -p Release
  cd Release
  CMAKE_SOURCE_DIR=..
fi

cmake $EXTRA_CMAKE_OPTIONS "$@" $CMAKE_SOURCE_DIR

if [[ "$1" == "--cmake-only" ]]; then
  echo "=============================================="
  echo "cmake completed - skipping make and docker tag"
  echo "=============================================="
  exit 0
fi

make

if [[ -n "${BENV_BASE_IMAGE_DISTRO}" ]] && [[ -n "${BENV_BASE_IMAGE_VERSION}" ]]; then
  echo docker tag ${DOCKER_TAG_PREFIX}build-env:latest "${DOCKER_TAG_PREFIX}build-env:${BENV_BASE_IMAGE_DISTRO}-${BENV_BASE_IMAGE_VERSION}"
  docker tag ${DOCKER_TAG_PREFIX}build-env:latest "${DOCKER_TAG_PREFIX}build-env:${BENV_BASE_IMAGE_DISTRO}-${BENV_BASE_IMAGE_VERSION}"
fi
