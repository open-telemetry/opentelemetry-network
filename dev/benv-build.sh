#!/bin/bash -x

# Defaults
dirty="false"
cmake_only="false"
asan="false"
ubsan="false"
jobs="$(((`nproc --all` + 1) / 2))"

# must match the mount destination of $host_build_dir in benv script
source_dir="${FLOWMILL_SRC}"
build_dir="${FLOWMILL_OUT_DIR}"

show_help() {
  cat << EOF
Usage:
  ${0##*/} [options...] [targets...]

Options:
  --dirty: don't clean previous build artifacts before building
  -j N | --jobs N: use N parallel jobs for building (default: auto-detect)
  --debug: build with CMAKE_BUILD_TYPE="Debug" which includes debug code, debug and trace logging, and turns off compiler
           optimization that prevents readable single stepping
  --cmake: run cmake but don't build with a subsequent make
  --asan: enable the address sanitizer (use with --debug to get better backtraces when an asan issue is hit)
  --ubsan: enable the undefined behavior sanitizer (which can have false positives: https://stackoverflow.com/a/57304113)
  --py-static-check: enables static checks for python
  --go-static-linking: enables static linking for Go binaries
  --go-static-check: enables static checks for golang
  --go-static-check-extra: 'enables honnef.co/go/tools/cmd/staticcheck' checks for golang
  --upload-symbols: uploads debug symbols
  --export-symbols: export debug symbols to local symbols directory
  --docker: start docker daemon within build environment
  --no-docker: don't run docker commands on docker build targets (needed when using an external docker build tool like CI/CD)
  --fail-fast: fail on the first error instead of building as much as possible
  --dep-tree: produce a dot-file output of the build graph
  --list-targets: print the list of targets that can be built using cmake by passing target(s) to this build script
  --cicd: use preset settings for building in CI/CD
  --otlp: build with otlp support
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
    -d|--dirty)
      dirty="true"
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
        -DDEBUG_LOG=ON
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
    --go-static-check)
      cmake_args+=(-DGO_STATIC_CHECK=ON)
      ;;
    --go-static-check-extra)
      cmake_args+=(-DGO_STATIC_CHECK=ON -DGO_STATIC_CHECK_EXTRA=ON)
      ;;
    --export-symbols)
      cmake_args+=(-DEXPORT_DEBUG_SYMBOLS=ON)
      ;;
    --upload-symbols)
      cmake_args+=(-DUPLOAD_DEBUG_SYMBOLS=ON)
      ;;
    --otlp)
      cmake_args+=(-DBUILD_WITH_OTLP=ON)
      ;;
    -v | --verbose)
      make_args+=(VERBOSE=1)
      ;;
    --docker)
      service docker start
      ;;
    --no-docker)
      cmake_args+=(-DRUN_DOCKER_COMMANDS=OFF)
      ;;
    --fail-fast)
      fail_fast=true
      ;;
    --cicd)
      # --no-docker
      cmake_args+=(-DRUN_DOCKER_COMMANDS=OFF)
      # --verbose
      make_args+=(VERBOSE=1)
      # --fail-fast
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

cd "${source_dir}"
echo -n "version being built: "
source ./version.sh

CMAKE_FLAGS=( \
  -DLLVM_DIR:PATH="$HOME/install/lib/cmake/llvm"
  -DCLANG_DIR="$HOME/install/lib/cmake/clang"
  -DCMAKE_INSTALL_PREFIX:PATH="$HOME/install"
  -DOPENSSL_ROOT_DIR:PATH="$HOME/install/openssl"
  -DCMAKE_PREFIX_PATH:PATH="$HOME/install"
  -DFLOWMILL_MAJOR_VERSION=${FLOWMILL_MAJOR_VERSION}
  -DFLOWMILL_MINOR_VERSION=${FLOWMILL_MINOR_VERSION}
  -DFLOWMILL_COLLECTOR_BUILD_NUMBER=${FLOWMILL_COLLECTOR_BUILD_NUMBER}
  -DFLOWMILL_PIPELINE_BUILD_NUMBER=${FLOWMILL_PIPELINE_BUILD_NUMBER}
  "${cmake_args[@]}"
)

if (("$jobs" <= 0)); then
  die "ERROR: --jobs requires a positive numeric argument; you gave ($jobs)"
fi

if [ "$dirty" != "true" ]; then
  if [ -d "$build_dir" ]; then
    rm -rf "$build_dir/*" || true
  fi
  mkdir -p "$build_dir"
fi

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
