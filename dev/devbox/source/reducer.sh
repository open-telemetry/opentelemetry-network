#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

num_shards="1"
publish_ports="false"
start_prometheus="false"
publish_otlp_grpc_metrics="true"
publish_prometheus_metrics="false"

docker_args=( \
  --env EBPF_NET_RUN_UNDER_GDB="${EBPF_NET_RUN_UNDER_GDB}"
  --volume "$HOME/src/:/root/src"
  --volume "$HOME/out/:/root/out"
  --privileged
)

app_args=( \
  --port=8000
  --internal-prom=0.0.0.0:7000
  --prom=0.0.0.0:7001
  --partitions-per-shard=1
  --enable-aws-enrichment
  --log-console
  --debug
)

function print_help {
  echo "usage: $0 [--cgdb|--disable-otlp-grpc-metrics|--enable-prometheus-metrics|--env|--gdb|--help|--num-shards <num>|--prom|--publish-ports] args..."
  echo
  echo '  --cgdb: run the pipeline server under `cgdb`'
  echo "  --disable-otlp-grpc-metrics: do not publish metrics via OTLP gRPC"
  echo "  --enable-prometheus-metrics: publish metrics via Prometheus"
  echo "  --env: export environment variable to container (--env VAR=VALUE)"
  echo '  --gdb: run the pipeline server under `gdb`'
  echo "  --help: display this help message and the container's help message"
  echo "  --num-shards <num>: the number of shards to run per reducer core"
  echo "  --prom: start prometheus"
  echo "  --publish-ports: publish individual ports (instead of running with --network=host)"
  echo "  --tag: use the reducer image with the specified tag (--tag <TAG>)"
  echo "  args...: any additional arguments are forwarded to the container"
}

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --disable-otlp-grpc-metrics)
      publish_otlp_grpc_metrics="false"
      ;;
    --enable-prometheus-metrics)
      publish_prometheus_metrics="true"
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

    --help)
      app_args+=("${arg}")
      print_help
      ;;

    --num-shards)
      num_shards="$1"
      shift;
      ;;

    --publish-ports)
      publish_ports="true"
      ;;

    --prom)
      start_prometheus="true"
      ;;

    --tag)
      if [[ "$#" -lt 1 ]]; then
        echo "missing argument for --tag"
	exit 1
      fi
      tag=":$1"; shift
      ;;

    *)
      app_args+=("${arg}")
      ;;
  esac
done

set -x

if [[ ${start_prometheus} == "true" ]]
then
  sudo systemctl start prometheus
fi

if [[ ${publish_ports} == "false" ]]
then
  docker_args+=(
    --network=host
  )
else
  docker_args+=(
    --publish 8000:8000
    --publish 7000:7000
    --publish 7001:7001
    --publish 7002:7001
    --publish 7003:7001
    )
fi

app_args+=(
  --num-ingest-shards=${num_shards}
  --num-matching-shards=${num_shards}
  --num-aggregation-shards=${num_shards}
)

if [[ ${publish_otlp_grpc_metrics} == "true" ]]
then
  app_args+=(
    --enable-otlp-grpc-metrics
  )
fi

if [[ ${publish_prometheus_metrics} == "false" ]]
then
  app_args+=(
    --disable-prometheus-metrics
  )
fi

docker pull localhost:5000/reducer${tag}

export container_id="$( \
  docker create -t --rm "${docker_args[@]}" \
    localhost:5000/reducer${tag} "${app_args[@]}" \
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
