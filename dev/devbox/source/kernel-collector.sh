#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


bpf_dump_file='bpf.render.raw'
bpf_src_export_file='bpf.src.c'
ingest_dump_file='ingest.render.raw'
host_data_mount_path="$(pwd)/data-$(basename "$0" .sh)"
container_data_mount_path="/hostfs/data"
collector_entrypoint="/srv/collector-entrypoint.sh"

minidump_dir="minidump"

mkdir -p "${host_data_mount_path}"
touch "${host_data_mount_path}/${bpf_src_export_file}"
mkdir -p "${host_data_mount_path}/${minidump_dir}"

docker_args=( \
  --env EBPF_NET_INTAKE_HOST="127.0.0.1"
  --env EBPF_NET_HOST_DIR="/hostfs"
  --privileged
  --pid host
  --network host
  --volume /sys/fs/cgroup:/hostfs/sys/fs/cgroup
  --volume /usr/src:/hostfs/usr/src
  --volume /lib/modules:/hostfs/lib/modules
  --volume /etc:/hostfs/etc
  --volume /var/cache:/hostfs/cache
  --volume /var/run/docker.sock:/var/run/docker.sock
  --env EBPF_NET_KERNEL_HEADERS_AUTO_FETCH="true"
  --env EBPF_NET_EXPORT_BPF_SRC_FILE="${container_data_mount_path}/${bpf_src_export_file}"
  --env EBPF_NET_MINIDUMP_DIR="${container_data_mount_path}/${minidump_dir}"
  --volume "${host_data_mount_path}:${container_data_mount_path}"
  --volume "$HOME/src/:/root/src"
  --volume "$HOME/out/:/root/out"
)

app_args=( \
  --log-console
)

function print_help {
  echo "usage: $0 [--help|--bpf-dump] args..."
  echo
  echo "  args...: any additional arguments are forwarded to the container"
  echo "  --help: display this help message and the container's help message"
  echo "  --env: export environment variable to container (--env VAR=VALUE)"
  echo '  --gdb: run the kernel collector under `gdb`'
  echo '  --cgdb: run the kernel collector under `cgdb`'
  echo "  --bpf-dump: dump eBPF messages into file '${host_data_mount_path}/${bpf_dump_file}'"
  echo "  --bpf-pipe: dump eBPF messages into named pipe '${host_data_mount_path}/${bpf_dump_file}'"
  echo "  --ingest-dump: dump ingest messages into file '${host_data_mount_path}/${ingest_dump_file}'"
  echo "  --ingest-pipe: dump ingest messages into named pipe '${host_data_mount_path}/${ingest_dump_file}'"
  echo "  --tag: use the kernel-collector image with the specified tag (--tag <TAG>)"
  echo '  --valgrind-memcheck: run the kernel collector under `valgrind` using the memcheck tool'
  echo '  --valgrind-massif: run the kernel collector under `valgrind` using the massif tool'
  echo
  echo "note: use '$HOME/out/tools/bpf_wire_to_json' to decode eBPF messages"
  sleep 5
}

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
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

    --bpf-dump)
      [[ ! -e "${host_data_mount_path}/${bpf_dump_file}" ]] \
        || rm -rf "${host_data_mount_path}/${bpf_dump_file}"
      touch "${host_data_mount_path}/${bpf_dump_file}"
      app_args+=("--bpf-dump-file=${container_data_mount_path}/${bpf_dump_file}")
      ;;

    --bpf-pipe)
      [[ ! -e "${host_data_mount_path}/${bpf_dump_file}" ]] \
        || rm -rf "${host_data_mount_path}/${bpf_dump_file}"
      mkfifo "${host_data_mount_path}/${bpf_dump_file}"
      app_args+=("--bpf-dump-file=${container_data_mount_path}/${bpf_dump_file}")
      ;;

    --ingest-dump)
      [[ ! -e "${host_data_mount_path}/${ingest_dump_file}" ]] \
        || rm -rf "${host_data_mount_path}/${ingest_dump_file}"
      touch "${host_data_mount_path}/${ingest_dump_file}"
      docker_args+=(--env EBPF_NET_RECORD_INTAKE_OUTPUT_PATH="${container_data_mount_path}/${ingest_dump_file}")
      ;;

    --ingest-pipe)
      [[ ! -e "${host_data_mount_path}/${ingest_dump_file}" ]] \
        || rm -rf "${host_data_mount_path}/${ingest_dump_file}"
      mkfifo "${host_data_mount_path}/${ingest_dump_file}"
      docker_args+=(--env EBPF_NET_RECORD_INTAKE_OUTPUT_PATH="${container_data_mount_path}/${ingest_dump_file}")
      ;;

    --tag)
      if [[ "$#" -lt 1 ]]; then
        echo "missing argument for --tag"
	exit 1
      fi
      tag=":$1"; shift
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

docker_args+=(--entrypoint "${collector_entrypoint}")

docker_args+=(
  --env EBPF_NET_INTAKE_PORT="8000"
  --env EBPF_NET_AGENT_NAMESPACE="${EBPF_NET_AGENT_NAMESPACE}"
  --env EBPF_NET_AGENT_CLUSTER="${EBPF_NET_AGENT_CLUSTER}"
  --env EBPF_NET_AGENT_SERVICE="${EBPF_NET_AGENT_SERVICE}"
  --env EBPF_NET_AGENT_HOST="${EBPF_NET_AGENT_HOST}"
  --env EBPF_NET_AGENT_ZONE="${EBPF_NET_AGENT_ZONE}"
  )

set -x

docker pull localhost:5000/kernel-collector${tag}

export container_id="$( \
  docker create -t --rm "${docker_args[@]}" \
    localhost:5000/kernel-collector${tag} "${app_args[@]}" \
)"

function cleanup_docker {
  docker kill "${container_id}" || true
  docker container prune --force || true
  docker volume prune --force || true
  docker image prune --force || true
}
trap cleanup_docker SIGINT

docker cp ".env" "${container_id}:/srv/.env"
cp "collector-entrypoint.sh" "/tmp/collector-entrypoint.sh"
docker cp "/tmp/collector-entrypoint.sh" "${container_id}:/srv/collector-entrypoint.sh"

docker start -i "${container_id}"
cleanup_docker
