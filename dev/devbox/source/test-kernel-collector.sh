#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

image="localhost:5000/kernel-collector-test"

bpf_dump_file='bpf.render.raw'
bpf_src_export_file='bpf.src.c'
ingest_dump_file='ingest.render.raw'
host_data_mount_path="$(pwd)/data-$(basename "$0" .sh)"
container_data_mount_path="/hostfs/data"

mkdir -p "${host_data_mount_path}"
touch "${host_data_mount_path}/${bpf_src_export_file}"

docker_args=( \
  --env EBPF_NET_HOST_DIR="/hostfs"
  --privileged
  --volume /sys/fs/cgroup:/hostfs/sys/fs/cgroup
  --volume /usr/src:/hostfs/usr/src
  --volume /lib/modules:/hostfs/lib/modules
  --volume /etc:/hostfs/etc
  --volume /var/cache:/hostfs/cache
  --volume /var/run/docker.sock:/var/run/docker.sock
  --env EBPF_NET_KERNEL_HEADERS_AUTO_FETCH="true"
  --env EBPF_NET_EXPORT_BPF_SRC_FILE="${container_data_mount_path}/${bpf_src_export_file}"
  --volume "${host_data_mount_path}:${container_data_mount_path}"
)

app_args=( \
  --log-console
)

function print_help {
  echo "usage: $0 [--help|...] args..."
  echo
  echo "  args...: any additional arguments are forwarded to the container"
  echo "  --delay-exit: sleep forever after test completes to prevent the docker container from exiting"
  echo "  --delay-exit-on-failure: if test fails, sleep forever to prevent the docker container from exiting"
  echo "  --help: display this help message and the container's help message"
  echo "  --env: export environment variable to container (--env VAR=VALUE)"
  echo '  --gdb: run the kernel collector under `gdb`'
  echo '  --cgdb: run the kernel collector under `cgdb`'
  echo "  --public: use the public kernel-collector-test image from quay.io (default is to use localhost:5000/kernel-collector-test image from local registry)"
  echo "  --tag: use the kernel-collector image with the specified tag (--tag <TAG>)"
  echo "  --trace: enable trace logging (default is debug level)"
  echo '  --valgrind-memcheck: run the kernel collector under `valgrind` using the memcheck tool'
  echo '  --valgrind-massif: run the kernel collector under `valgrind` using the massif tool'
  echo
  sleep 5
}

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --delay-exit)
      docker_args+=(--env DELAY_EXIT="true")
      ;;

    --delay-exit-on-failure)
      docker_args+=(--env DELAY_EXIT_ON_FAILURE="true")
      ;;

    --env)
      if [[ "$#" -lt 1 ]]; then
        echo "expected: environment variable to export"
	exit 1
      fi
      docker_args+=(--env "$1"); shift
      ;;

    --gdb)
      docker_args+=(--env EBPF_NET_RUN_UNDER_GDB="gdb")
      ;;

    --cgdb)
      docker_args+=(--env EBPF_NET_RUN_UNDER_GDB="cgdb")
      ;;

    --public)
      image="quay.io/splunko11ytest/network-explorer-debug/kernel-collector-test"
      if [[ "${tag}" == "" ]]
      then
        tag=":latest"
      fi
      ;;

    --tag)
      if [[ "$#" -lt 1 ]]; then
        echo "missing argument for --tag"
	exit 1
      fi
      tag=":$1"; shift
      ;;

    --trace)
      docker_args+=(--env SPDLOG_LEVEL="trace")
      ;;

    --valgrind-memcheck)
      docker_args+=(--env EBPF_NET_RUN_UNDER_VALGRIND="--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes")
      ;;

    --valgrind-massif)
      docker_args+=(--env EBPF_NET_RUN_UNDER_VALGRIND="--tool=massif --stacks=yes --massif-out-file=/root/out/massif.out.%p")
      ;;

    --help)
      app_args+=("${arg}")
      print_help
      ;;

    *)
      app_args+=("${arg}")
      ;;
  esac
done

set -x

docker pull "${image}${tag}"

export container_id="$( \
  docker create -t --rm "${docker_args[@]}" \
    "${image}${tag}" "${app_args[@]}" \
)"

function cleanup_docker {
  docker kill "${container_id}" || true
  docker container prune --force || true
  docker volume prune --force || true
  docker image prune --force || true
}
trap cleanup_docker SIGINT

docker start -i "${container_id}"
cleanup_docker
