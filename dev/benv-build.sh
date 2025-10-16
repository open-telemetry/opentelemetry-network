#!/bin/bash -x
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


# Defaults
clean="false"
cmake_only="false"
asan="false"
ubsan="false"
jobs="$(((`nproc --all` + 1) / 2))"

# must match the mount destination of $host_build_dir in benv script
source_dir="${EBPF_NET_SRC:-${EBPF_NET_SRC_ROOT}}"
build_dir="${EBPF_NET_OUT_DIR:-$HOME/out}"

show_help() {
  cat << EOF
Usage:
  ${0##*/} [options...] [targets...]

Options:
  --clean: clean previous build artifacts before building
  -j N | --jobs N: use N parallel jobs for building (default: auto-detect)
  --debug: build with CMAKE_BUILD_TYPE="Debug" which includes debug code and turns off compiler
           optimization that prevents readable single stepping
  --cmake: run cmake but don't build with a subsequent make
  --asan: enable the address sanitizer (use with --debug to get better backtraces when an asan issue is hit)
  --ubsan: enable the undefined behavior sanitizer (which can have false positives: https://stackoverflow.com/a/57304113)
  --py-static-check: enables static checks for python
  --go-static-linking: enables static linking for Go binaries
  --no-docker: don't run docker commands on docker build targets (needed when using an external docker build tool like CI/CD)
  --fail-fast: fail on the first error instead of building as much as possible
  --dep-tree: produce a dot-file output of the build graph
  --list-targets: print the list of targets that can be built using cmake by passing target(s) to this build script
  -v | --verbose: verbose build output

Targets:
  run 'make help' after successfully running cmake to get a list of targets
EOF
}

die() {
    printf '%s\n' "$1" >&2
    exit 1
}

CMAKE_BUILD_TYPE="RelWithDebInfo"
cmake_args=()
make_args=()
fail_fast=false

while :; do
  case $1 in
    -h|-\?|--help)
      show_help
      exit
      ;;
    --clean)
      clean="true"
      ;;
    -j|--jobs)
      if [ "$2" ]; then
        jobs="$2"
        shift
      else
        die "ERROR: --jobs requires a numeric argument"
      fi
      ;;
    -j|--jobs=?*)
      jobs=${1#*=}
      ;;
    --cmake)
      cmake_only="true"
      ;;
    --debug)
      CMAKE_BUILD_TYPE="Debug"
      cmake_args+=( \
        -DOPTIMIZE=OFF
      )
      ;;
    --dep-tree)
      cmake_args+=(--graphviz=dep-tree.dot)
      ;;
    --list-targets)
      list_targets="true"
      ;;
    --asan)
      cmake_args+=(-DUSE_ADDRESS_SANITIZER=ON)
      ;;
    --ubsan)
      cmake_args+=(-DUSE_UNDEFINED_BEHAVIOR_SANITIZER=ON)
      ;;
    --py-static-check)
      cmake_args+=(-DPY_STATIC_CHECK=ON)
      ;;
    --go-static-linking)
      cmake_args+=(-DGO_STATIC_LINK=ON)
      ;;
    -v | --verbose)
      make_args+=(VERBOSE=1)
      ;;
    --no-docker)
      cmake_args+=(-DRUN_DOCKER_COMMANDS=OFF)
      ;;
    --fail-fast)
      fail_fast=true
      ;;
    --)
      shift
      break
      ;;
    -?*)
      printf 'WARN: Unknown option (ignored): %s\n' "$1" >&2
      ;;
    *)
      break
  esac
  shift
done

while [[ "$#" -gt 0 ]]; do make_args+=("$1"); shift; done

# Add the source directory to git's list of safe directories.
# Prevents "fatal: detected dubious ownership in repository at ..." error.
if [[ $(git config --global --get-all safe.directory | grep -ce "^${source_dir}\$") == "0" ]]; then
  git config --global --add safe.directory $source_dir
fi

cd "${source_dir}"
echo -n "version being built: "
source ./version.sh

CMAKE_FLAGS=( \
  -DCMAKE_INSTALL_PREFIX:PATH="/install"
  -DOPENSSL_ROOT_DIR:PATH="/install/openssl"
  -DCMAKE_PREFIX_PATH:PATH="/install"
  -DEBPF_NET_MAJOR_VERSION=${EBPF_NET_MAJOR_VERSION}
  -DEBPF_NET_MINOR_VERSION=${EBPF_NET_MINOR_VERSION}
  -DEBPF_NET_PATCH_VERSION=${EBPF_NET_PATCH_VERSION}
  -DEBPF_NET_COLLECTOR_BUILD_NUMBER=${EBPF_NET_COLLECTOR_BUILD_NUMBER}
  -DEBPF_NET_PIPELINE_BUILD_NUMBER=${EBPF_NET_PIPELINE_BUILD_NUMBER}
  "${cmake_args[@]}"
)

if (("$jobs" <= 0)); then
  die "ERROR: --jobs requires a positive numeric argument; you gave ($jobs)"
fi

if [[ "$clean" == "true" ]]; then
  if [ -d "$build_dir" ]; then
    rm -rf $build_dir/* || true
  fi
fi

mkdir -p "$build_dir"
cd "$build_dir"

set -e

(set -x; cmake \
  "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" \
  "${CMAKE_FLAGS[@]}" \
  "${source_dir}"
)

if [[ "$cmake_only" == "true" ]]; then
  exit 0
fi

if [[ "$list_targets" == "true" ]]; then
    cmake --build . --target help
    exit 0
fi

[[ "${fail_fast}" == "true" ]] || make_args=(--keep-going "${make_args[@]}")

(set -x; make -j"$jobs" "${make_args[@]}") \
  && echo "BUILD SUCCESSFUL"
