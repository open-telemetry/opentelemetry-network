#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


docker pull localhost:5000/k8s-relay

export container_id="$( \
  docker create -t --rm \
    --env EBPF_NET_INTAKE_PORT="8001" \
    --env EBPF_NET_INTAKE_HOST="127.0.0.1" \
    --network host \
    --entrypoint "/srv/collector-entrypoint.sh" \
    --volume "$HOME/src/:/root/src" \
    --volume "$HOME/out/:/root/out" \
    localhost:5000/k8s-relay \
      --log-console \
      --debug \
      "$@" \
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
